--- pci_passthru.h.orig	2024-12-16 14:36:36.044279000 +0300
+++ pci_passthru.h	2024-12-16 14:45:20.830669000 +0300
@@ -35,6 +35,10 @@
     struct pci_devinst *pi, int coff, int bytes, uint32_t *rv);
 typedef int (*cfgwrite_handler)(struct passthru_softc *sc,
     struct pci_devinst *pi, int coff, int bytes, uint32_t val);
+typedef int (*passthru_read_handler)(struct pci_devinst *pi, uint64_t off,
+    uint64_t size, uint64_t *rv);
+typedef int (*passthru_write_handler)(struct pci_devinst *pi, uint64_t off,
+    uint64_t size, uint64_t val);
 
 uint32_t pci_host_read_config(const struct pcisel *sel, long reg, int width);
 void pci_host_write_config(const struct pcisel *sel, long reg, int width,
@@ -49,3 +53,6 @@
 struct pcisel *passthru_get_sel(struct passthru_softc *sc);
 int set_pcir_handler(struct passthru_softc *sc, int reg, int len,
     cfgread_handler rhandler, cfgwrite_handler whandler);
+int passthru_set_bar_handler(struct passthru_softc *sc, int baridx,
+    vm_offset_t off, vm_size_t len, passthru_read_handler rhandler,
+    passthru_write_handler whandler);
