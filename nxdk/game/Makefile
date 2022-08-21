XBE_TITLE = nxdk\ game\ -\ SDL2_image
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.c
NXDK_DIR ?= $(CURDIR)/..
NXDK_SDL = y

include $(NXDK_DIR)/Makefile

TARGET += $(OUTPUT_DIR)/resources
$(GEN_XISO): $(OUTPUT_DIR)/resources
$(OUTPUT_DIR)/resources: $(CURDIR)/resources $(OUTPUT_DIR)
	$(VE)cp -R '$<' '$@'
