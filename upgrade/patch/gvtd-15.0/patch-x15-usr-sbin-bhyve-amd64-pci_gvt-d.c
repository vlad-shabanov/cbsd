--- pci_gvt-d.c.orig	2024-12-15 20:23:32.510843000 +0000
+++ pci_gvt-d.c	2024-12-15 22:16:59.099197000 +0000
@@ -32,6 +32,7 @@
 #define PCI_VENDOR_INTEL 0x8086
 
 #define PCIR_BDSM 0x5C	   /* Base of Data Stolen Memory register */
+#define PCIR_BDSM_GEN11 0xC0
 #define PCIR_ASLS_CTL 0xFC /* Opregion start address register */
 
 #define PCIM_BDSM_GSM_ALIGNMENT \
@@ -39,6 +40,7 @@
 
 #define GVT_D_MAP_GSM 0
 #define GVT_D_MAP_OPREGION 1
+#define GVT_D_MAP_VBT 2
 
 static int
 gvt_d_probe(struct pci_devinst *const pi)
@@ -81,6 +83,58 @@
 	    e820_alloc(4 * GB, length, alignment, type, E820_ALLOCATE_HIGHEST));
 }
 
+static int
+gvt_d_dsmbase_read(struct pci_devinst *pi, uint64_t off, uint64_t size, uint64_t *rv)
+{
+	assert((off & (size - 1)) == 0);
+
+	switch (size) {
+		case 1:
+			*rv = pci_get_cfgdata8(pi, PCIR_BDSM_GEN11 + off);
+			break;
+		case 2:
+			*rv = pci_get_cfgdata16(pi, PCIR_BDSM_GEN11 + off);
+			break;
+		case 4:
+			*rv = pci_get_cfgdata32(pi, PCIR_BDSM_GEN11 + off);
+			break;
+		case 8:
+			*rv = pci_get_cfgdata32(pi, PCIR_BDSM_GEN11 + off);
+			*rv |= (((uint64_t)pci_get_cfgdata32(pi, off)) << 32);
+			break;
+		default:
+			return (-1);
+	}
+
+	return (0);
+}
+
+static int
+gvt_d_dsmbase_write(struct pci_devinst *pi, uint64_t off, uint64_t size, uint64_t val)
+{
+	assert((off & (size - 1)) == 0);
+
+	switch (size) {
+		case 1:
+			pci_set_cfgdata8(pi, PCIR_BDSM_GEN11 + off, val);
+			break;
+		case 2:
+			pci_set_cfgdata16(pi, PCIR_BDSM_GEN11 + off, val);
+			break;
+		case 4:
+			pci_set_cfgdata32(pi, PCIR_BDSM_GEN11 + off, val);
+			break;
+		case 8:
+			pci_set_cfgdata32(pi, PCIR_BDSM_GEN11 + off, val);
+			pci_set_cfgdata32(pi, PCIR_BDSM_GEN11 + off, val >> 32);
+			break;
+		default:
+			return (-1);
+	}
+
+	return (0);
+}
+
 /*
  * Note that the graphics stolen memory is somehow confusing. On the one hand
  * the Intel Open Source HD Graphics Programmers' Reference Manual states that
@@ -108,7 +162,7 @@
 	struct passthru_softc *sc;
 	struct passthru_mmio_mapping *gsm;
 	size_t sysctl_len;
-	uint32_t bdsm;
+	uint64_t bdsm, gen = 0;
 	int error;
 
 	sc = pi->pi_arg;
@@ -169,21 +223,109 @@
 		    "Warning: Unable to reuse host address of Graphics Stolen Memory. GPU passthrough might not work properly.");
 	}
 
-	bdsm = pci_host_read_config(passthru_get_sel(sc), PCIR_BDSM, 4);
-	pci_set_cfgdata32(pi, PCIR_BDSM,
-	    gsm->gpa | (bdsm & (PCIM_BDSM_GSM_ALIGNMENT - 1)));
+	sysctl_len = sizeof(gen);
+	error = sysctlbyname("hw.intel_graphics_gen", &gen, &sysctl_len, NULL,
+	    0);
+	if (error) {
+		warn("%s: Unable to get generation of intel graphics device",
+		    __func__);
+		return (-1);
+	}
 
-	return (set_pcir_handler(sc, PCIR_BDSM, 4, passthru_cfgread_emulate,
-	    passthru_cfgwrite_emulate));
+	if (gen < 11) {
+		bdsm = pci_host_read_config(passthru_get_sel(sc), PCIR_BDSM, 4);
+		pci_set_cfgdata32(pi, PCIR_BDSM,
+		    gsm->gpa | (bdsm & (PCIM_BDSM_GSM_ALIGNMENT - 1)));
+		error = set_pcir_handler(sc, PCIR_BDSM, 4,
+		    passthru_cfgread_emulate, passthru_cfgwrite_emulate);
+	} else {
+		/* Protect the BDSM register in PCI config space. */
+		bdsm = pci_host_read_config(passthru_get_sel(sc), PCIR_BDSM_GEN11, 8);
+		pci_set_cfgdata32(pi, PCIR_BDSM_GEN11,
+		    gsm->gpa | (bdsm & (PCIM_BDSM_GSM_ALIGNMENT - 1)));
+		pci_set_cfgdata32(pi, PCIR_BDSM_GEN11 + 4, gsm->gpa >> 32);
+		error = set_pcir_handler(sc, PCIR_BDSM_GEN11, 8,
+		    passthru_cfgread_emulate, passthru_cfgwrite_emulate);
+
+		/* Protect the BDSM register in MMIO space. */
+		passthru_set_bar_handler(sc, 0, 0x1080C0, sizeof(uint64_t),
+		    gvt_d_dsmbase_read, gvt_d_dsmbase_write);
+	}
+
+	return (error);
 }
 
 static int
+gvt_d_setup_vbt(struct pci_devinst *const pi, int memfd, uint64_t vbt_hpa,
+    uint64_t vbt_len, vm_paddr_t *vbt_gpa)
+{
+	struct passthru_softc *sc;
+	struct passthru_mmio_mapping *vbt;
+
+	sc = pi->pi_arg;
+
+	vbt = passthru_get_mmio(sc, GVT_D_MAP_VBT);
+	if (vbt == NULL) {
+		warnx("%s: Unable to access VBT", __func__);
+		return (-1);
+	}
+
+	vbt->hpa = vbt_hpa;
+	vbt->len = vbt_len;
+
+	vbt->hva = mmap(NULL, vbt->len, PROT_READ, MAP_SHARED, memfd, vbt->hpa);
+	if (vbt->hva == MAP_FAILED) {
+		warn("%s: Unable to map VBT", __func__);
+		return (-1);
+	}
+
+	vbt->gpa = gvt_d_alloc_mmio_memory(vbt->hpa, vbt->len,
+	    E820_ALIGNMENT_NONE, E820_TYPE_NVS);
+	if (vbt->gpa == 0) {
+		warnx(
+		    "%s: Unable to add VBT to E820 table (hpa 0x%lx len 0x%lx)",
+		    __func__, vbt->hpa, vbt->len);
+		munmap(vbt->hva, vbt->len);
+		e820_dump_table();
+		return (-1);
+	}
+	vbt->gva = vm_map_gpa(pi->pi_vmctx, vbt->gpa, vbt->len);
+	if (vbt->gva == NULL) {
+		warnx("%s: Unable to map guest VBT", __func__);
+		munmap(vbt->hva, vbt->len);
+		return (-1);
+	}
+	if (vbt->gpa != vbt->hpa) {
+		/*
+		* A 1:1 host to guest mapping is not required but this could
+		* change in the future.
+		*/
+		warnx(
+		    "Warning: Unable to reuse host address of VBT. GPU passthrough might not work properly.");
+	}
+
+	memcpy(vbt->gva, vbt->hva, vbt->len);
+
+	/*
+	* Return the guest physical address. It's used to patch the OpRegion
+	* properly.
+	*/
+	*vbt_gpa = vbt->gpa;
+
+	return (0);
+}
+
+static int
 gvt_d_setup_opregion(struct pci_devinst *const pi)
 {
 	struct passthru_softc *sc;
 	struct passthru_mmio_mapping *opregion;
+	struct igd_opregion *opregion_ptr;
 	struct igd_opregion_header *header;
+	vm_paddr_t vbt_gpa = 0;
+	vm_paddr_t vbt_hpa;
 	uint64_t asls;
+	int error = 0;
 	int memfd;
 
 	sc = pi->pi_arg;
@@ -236,6 +378,39 @@
 		close(memfd);
 		return (-1);
 	}
+
+	opregion_ptr = (struct igd_opregion *)opregion->hva;
+	if (opregion_ptr->mbox3.rvda != 0) {
+		/*
+		* OpRegion v2.0 contains a physical address to the VBT. This
+		* address is useless in a guest environment. It's possible to
+		* patch that but we don't support that yet. So, the only thing
+		* we can do is give up.
+		*/
+		if (opregion_ptr->header.over == 0x02000000) {
+			warnx(
+			    "%s: VBT lays outside OpRegion. That's not yet supported for a version 2.0 OpRegion",
+			    __func__);
+			close(memfd);
+			return (-1);
+		}
+		vbt_hpa = opregion->hpa + opregion_ptr->mbox3.rvda;
+		if (vbt_hpa < opregion->hpa) {
+			warnx(
+			    "%s: overflow when calculating VBT address (OpRegion @ 0x%lx, RVDA = 0x%lx)",
+			    __func__, opregion->hpa, opregion_ptr->mbox3.rvda);
+			close(memfd);
+			return (-1);
+		}
+
+		if ((error = gvt_d_setup_vbt(pi, memfd, vbt_hpa,
+		    opregion_ptr->mbox3.rvds, &vbt_gpa)) != 0) {
+			munmap(opregion->hva, opregion->len);
+			close(memfd);
+			return (error);
+			}
+	}
+
 	close(memfd);
 
 	opregion->gpa = gvt_d_alloc_mmio_memory(opregion->hpa, opregion->len,
@@ -262,6 +437,20 @@
 	}
 
 	memcpy(opregion->gva, opregion->hva, opregion->len);
+
+	/*
+	* Patch the VBT address to match our guest physical address.
+	*/
+	if (vbt_gpa != 0) {
+		if (vbt_gpa < opregion->gpa) {
+			warnx(
+			    "%s: invalid guest VBT address 0x%16lx (OpRegion @ 0x%16lx)",
+			    __func__, vbt_gpa, opregion->gpa);
+			return (-1);
+		}
+
+		((struct igd_opregion *)opregion->gva)->mbox3.rvda = vbt_gpa - opregion->gpa;
+	}
 
 	pci_set_cfgdata32(pi, PCIR_ASLS_CTL, opregion->gpa);
 
