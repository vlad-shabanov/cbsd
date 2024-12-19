--- pci_passthru.c.orig	2024-12-16 14:36:36.044189000 +0300
+++ pci_passthru.c	2024-12-17 01:14:52.899198000 +0300
@@ -72,12 +72,20 @@
 #define MSIX_TABLE_COUNT(ctrl) (((ctrl) & PCIM_MSIXCTRL_TABLE_SIZE) + 1)
 #define MSIX_CAPLEN 12
 
-#define PASSTHRU_MMIO_MAX 2
+#define PASSTHRU_MMIO_MAX 3
 
 static int pcifd = -1;
 
 SET_DECLARE(passthru_dev_set, struct passthru_dev);
 
+struct passthru_bar_handler {
+	TAILQ_ENTRY(passthru_bar_handler) chain;
+	vm_offset_t off;
+	vm_size_t len;
+	passthru_read_handler read;
+	passthru_write_handler write;
+};
+
 struct passthru_softc {
 	struct pci_devinst *psc_pi;
 	/* ROM is handled like a BAR */
@@ -95,6 +103,8 @@
 	struct passthru_mmio_mapping psc_mmio_map[PASSTHRU_MMIO_MAX];
 	cfgread_handler psc_pcir_rhandler[PCI_REGMAX + 1];
 	cfgwrite_handler psc_pcir_whandler[PCI_REGMAX + 1];
+	TAILQ_HEAD(,
+	passthru_bar_handler) psc_bar_handler[PCI_BARMAX_WITH_ROM + 1];
 };
 
 static int
@@ -694,7 +704,46 @@
 
 	return (0);
 }
+
+int
+passthru_set_bar_handler(struct passthru_softc *sc, int baridx, vm_offset_t off,
+    vm_size_t len, passthru_read_handler rhandler,
+    passthru_write_handler whandler)
+{
+	struct passthru_bar_handler *handler_new;
+	struct passthru_bar_handler *handler;
+
+	assert(sc->psc_bar[baridx].type == PCIBAR_IO ||
+	    sc->psc_bar[baridx].type == PCIBAR_MEM32 ||
+	    sc->psc_bar[baridx].type == PCIBAR_MEM64);
+	assert(sc->psc_bar[baridx].size > off + len);
+	assert(off < off + len);
 
+	handler_new = malloc(sizeof(struct passthru_bar_handler));
+	if (handler_new == NULL) {
+		return (ENOMEM);
+	}
+
+	handler_new->off = off;
+	handler_new->len = len;
+	handler_new->read = rhandler;
+	handler_new->write = whandler;
+
+	TAILQ_FOREACH (handler, &sc->psc_bar_handler[baridx], chain) {
+		if (handler->off < handler_new->off) {
+			assert(handler->off + handler->len < handler_new->off);
+			continue;
+		}
+		assert(handler->off > handler_new->off + handler_new->len);
+		TAILQ_INSERT_BEFORE(handler, handler_new, chain);
+		return (0);
+	}
+
+	TAILQ_INSERT_TAIL(&sc->psc_bar_handler[baridx], handler_new, chain);
+
+	return (0);
+}
+
 static int
 passthru_legacy_config(nvlist_t *nvl, const char *opts)
 {
@@ -900,6 +949,9 @@
 
 	pi->pi_arg = sc;
 	sc->psc_pi = pi;
+
+	for (uint8_t i = 0; i < PCI_BARMAX_WITH_ROM + 1; ++i)
+		TAILQ_INIT(&sc->psc_bar_handler[i]);
 
 	/* initialize config space */
 	if ((error = cfginit(pi, bus, slot, func)) != 0)
@@ -1118,6 +1170,7 @@
     uint64_t value)
 {
 	struct passthru_softc *sc;
+	struct passthru_bar_handler *handler;
 	struct pci_bar_ioreq pio;
 
 	sc = pi->pi_arg;
@@ -1125,9 +1178,16 @@
 	if (baridx == pci_msix_table_bar(pi)) {
 		msix_table_write(sc, offset, size, value);
 	} else {
-		assert(pi->pi_bar[baridx].type == PCIBAR_IO);
 		assert(size == 1 || size == 2 || size == 4);
-		assert(offset <= UINT32_MAX && offset + size <= UINT32_MAX);
+
+		TAILQ_FOREACH(handler, &sc->psc_bar_handler[baridx], chain) {
+			if (handler->off <= offset &&
+				offset + size <= handler->off + handler->len) {
+					handler->write(pi, offset - handler->off, size,
+				    value);
+				return;
+				}
+		}
 
 		bzero(&pio, sizeof(pio));
 		pio.pbi_sel = sc->psc_sel;
@@ -1145,6 +1205,7 @@
 passthru_read(struct pci_devinst *pi, int baridx, uint64_t offset, int size)
 {
 	struct passthru_softc *sc;
+	struct passthru_bar_handler *handler;
 	struct pci_bar_ioreq pio;
 	uint64_t val;
 
@@ -1153,10 +1214,16 @@
 	if (baridx == pci_msix_table_bar(pi)) {
 		val = msix_table_read(sc, offset, size);
 	} else {
-		assert(pi->pi_bar[baridx].type == PCIBAR_IO);
 		assert(size == 1 || size == 2 || size == 4);
-		assert(offset <= UINT32_MAX && offset + size <= UINT32_MAX);
 
+		TAILQ_FOREACH(handler, &sc->psc_bar_handler[baridx], chain) {
+			if (handler->off <= offset &&
+			    offset + size <= handler->off + handler->len) {
+				handler->read(pi, offset - handler->off, size, &val);
+				return (val);
+			    }
+		}
+
 		bzero(&pio, sizeof(pio));
 		pio.pbi_sel = sc->psc_sel;
 		pio.pbi_op = PCIBARIO_READ;
@@ -1207,47 +1274,84 @@
 		address += table_offset + table_size;
 		if (!enabled) {
 			if (vm_unmap_pptdev_mmio(pi->pi_vmctx,
-						 sc->psc_sel.pc_bus,
-						 sc->psc_sel.pc_dev,
-						 sc->psc_sel.pc_func, address,
-						 remaining) != 0)
+						sc->psc_sel.pc_bus,
+						sc->psc_sel.pc_dev,
+						sc->psc_sel.pc_func, address,
+						remaining) != 0)
 				warnx("pci_passthru: unmap_pptdev_mmio failed");
 		} else {
 			if (vm_map_pptdev_mmio(pi->pi_vmctx, sc->psc_sel.pc_bus,
-					       sc->psc_sel.pc_dev,
-					       sc->psc_sel.pc_func, address,
-					       remaining,
-					       sc->psc_bar[baridx].addr +
-					       table_offset + table_size) != 0)
+						sc->psc_sel.pc_dev,
+						sc->psc_sel.pc_func, address,
+						remaining,
+						sc->psc_bar[baridx].addr +
+						table_offset + table_size) != 0)
 				warnx("pci_passthru: map_pptdev_mmio failed");
 		}
 	}
 }
 
-static void
-passthru_mmio_addr(struct pci_devinst *pi, int baridx, int enabled,
-    uint64_t address)
+static int
+passthru_mmio_map(struct pci_devinst *pi, int baridx, int enabled,
+    uint64_t address, uint64_t off, uint64_t len)
 {
-	struct passthru_softc *sc;
+	struct passthru_softc *sc = pi->pi_arg;
 
-	sc = pi->pi_arg;
 	if (!enabled) {
 		if (vm_unmap_pptdev_mmio(pi->pi_vmctx, sc->psc_sel.pc_bus,
-					 sc->psc_sel.pc_dev,
-					 sc->psc_sel.pc_func, address,
-					 sc->psc_bar[baridx].size) != 0)
-			warnx("pci_passthru: unmap_pptdev_mmio failed");
+			sc->psc_sel.pc_dev, sc->psc_sel.pc_func, address + off,
+			len) != 0) {
+				warnx("pci_passthru: unmap_pptdev_mmio failed");
+				return (-1);
+			}
 	} else {
 		if (vm_map_pptdev_mmio(pi->pi_vmctx, sc->psc_sel.pc_bus,
-				       sc->psc_sel.pc_dev,
-				       sc->psc_sel.pc_func, address,
-				       sc->psc_bar[baridx].size,
-				       sc->psc_bar[baridx].addr) != 0)
+		    sc->psc_sel.pc_dev, sc->psc_sel.pc_func, address + off,
+		    len, sc->psc_bar[baridx].addr + off) != 0) {
 			warnx("pci_passthru: map_pptdev_mmio failed");
+			return (-1);
+		}
 	}
+
+	return (0);
 }
 
 static void
+passthru_mmio_addr(struct pci_devinst *pi, int baridx, int enabled,
+    uint64_t address)
+{
+	struct passthru_softc *sc;
+	struct passthru_bar_handler *handler;
+	uint64_t off;
+
+	sc = pi->pi_arg;
+
+	off = 0;
+
+	/* The queue is sorted by offset in ascending order. */
+	TAILQ_FOREACH(handler, &sc->psc_bar_handler[baridx], chain) {
+		uint64_t handler_off = rounddown2(handler->off, PAGE_SIZE);
+		uint64_t handler_end = roundup2(handler->off + handler->len,
+		    PAGE_SIZE);
+
+		/*
+		* When two handler point to the same page, handler_off can be
+		* lower than off. That's fine because we have nothing to do in
+		* that case.
+		*/
+		if (handler_off > off) {
+		    passthru_mmio_map(pi, baridx, enabled, address, off,
+			handler_off - off);
+		}
+
+		off = handler_end;
+	}
+
+	passthru_mmio_map(pi, baridx, enabled, address, off,
+	    sc->psc_bar[baridx].size - off);
+}
+
+static void
 passthru_addr_rom(struct pci_devinst *const pi, const int idx,
     const int enabled)
 {
