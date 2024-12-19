--- pci_early_quirks.c.orig	2024-12-15 20:23:31.796640000 +0000
+++ pci_early_quirks.c	2024-12-15 21:52:35.393998000 +0000
@@ -64,6 +64,15 @@
     "Size of the intel graphics stolen memory.");
 
 /*
+ * The sysctl value of the Intel graphics generation isn't fully accurate
+ * because it doesn't have to. It's only used by bhyve to check for Intel
+ * graphics gen 11 and above.
+ */
+u_int intel_graphics_gen = 0;
+SYSCTL_UINT(_hw, OID_AUTO, intel_graphics_gen, CTLFLAG_RD, &intel_graphics_gen,
+    0, "Generation of the intel graphics device.");
+
+/*
  * Intel early quirks functions
  */
 static vm_paddr_t
@@ -193,36 +202,43 @@
 struct intel_stolen_ops {
 	vm_paddr_t (*base)(int domain, int bus, int slot, int func);
 	vm_paddr_t (*size)(int domain, int bus, int slot, int func);
+	u_int gen;
 };
 
 static const struct intel_stolen_ops intel_stolen_ops_gen3 = {
 	.base = intel_stolen_base_gen3,
 	.size = intel_stolen_size_gen3,
+	.gen = 3,
 };
 
 static const struct intel_stolen_ops intel_stolen_ops_gen6 = {
 	.base = intel_stolen_base_gen3,
 	.size = intel_stolen_size_gen6,
+	.gen = 6,
 };
 
 static const struct intel_stolen_ops intel_stolen_ops_gen8 = {
 	.base = intel_stolen_base_gen3,
 	.size = intel_stolen_size_gen8,
+	.gen = 8,
 };
 
 static const struct intel_stolen_ops intel_stolen_ops_gen9 = {
 	.base = intel_stolen_base_gen3,
 	.size = intel_stolen_size_gen9,
+	.gen = 9,
 };
 
 static const struct intel_stolen_ops intel_stolen_ops_chv = {
 	.base = intel_stolen_base_gen3,
 	.size = intel_stolen_size_chv,
+	.gen = 3,
 };
 
 static const struct intel_stolen_ops intel_stolen_ops_gen11 = {
 	.base = intel_stolen_base_gen11,
 	.size = intel_stolen_size_gen9,
+	.gen = 11,
 };
 
 static const struct pci_device_id intel_ids[] = {
@@ -306,6 +322,7 @@
 		ops = intel_ids[i].data;
 		intel_graphics_stolen_base = ops->base(domain, bus, slot, func);
 		intel_graphics_stolen_size = ops->size(domain, bus, slot, func);
+		intel_graphics_gen = ops->gen;
 		break;
 	}
 
