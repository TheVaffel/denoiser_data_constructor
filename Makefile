VULKAN_SUBDIR := Vulkan-glTF-PBR
CHAMELEON_SUBDIR := ChameleonRT
PATH_WRITER_SUBDIR := path_writer
NANORT_SUBDIR := ../nanort

VULKAN_BUILD_DIR := $(VULKAN_SUBDIR)/build
CHAMELEON_BUILD_DIR := $(CHAMELEON_SUBDIR)/build
PATH_WRITER_BUILD_DIR := $(PATH_WRITER_SUBDIR)
NANORT_BUILD_DIR := $(NANORT_SUBDIR)/build


PATH_JSON ?= sponza_path.json
SCENE ?= /home/haakon/data/sponza/sponza1.gltf
SCENE_OBJ := $(shell echo "$${SCENE%.*}.obj")
# SCENE_OBJ := /home/haakon/data/sponza/sponza1.obj
SCENE_DIR := $(shell echo "$${SCENE%/*}/")
# SCENE_DIR := /home/haakon/data/sponza/

INTERVAL ?= -1 -1

OUTPUT_DIR ?= generic_output


all: vulkan chameleon path_writer

vulkan:
	$(MAKE) -C $(VULKAN_BUILD_DIR)

chameleon:
	$(MAKE) -C $(CHAMELEON_BUILD_DIR)

# nanort:
# 	$(MAKE) -C $(NANORT_BUILD_DIR)

path_writer:
	$(MAKE) -C $(PATH_WRITER_BUILD_DIR)

data: feature_buffers ray_trace path_header

feature_buffers: $(OUTPUT_DIR)
	cd $(VULKAN_BUILD_DIR) && ./bin/Vulkan-glTF-PBR -p ../../$(PATH_JSON) -s $(SCENE) -o ../../$(OUTPUT_DIR)/shading_normal,../../$(OUTPUT_DIR)/albedo,../../$(OUTPUT_DIR)/world_position -f normal,albedo,position --start 0 --interval $(INTERVAL)

ray_trace: $(OUTPUT_DIR)
	cd $(CHAMELEON_BUILD_DIR) && ./chameleonrt -optix $(SCENE) -path ../../$(PATH_JSON) -output ../../$(OUTPUT_DIR)/color -start 0 -interval $(INTERVAL)

path_header: $(OUTPUT_DIR)
	cd $(PATH_WRITER_BUILD_DIR) && ./path_write ../$(PATH_JSON) -o ../$(OUTPUT_DIR)/camera_matrices.h

# nano_trace: $(OUTPUT_DIR)
# 	cd $(NANORT_BUILD_DIR) && ./bin/path_tracer -p ../../dataconstruction/$(PATH_JSON) -o ../../dataconstruction/$(OUTPUT_DIR)/color $(SCENE_OBJ) 1.0 $(SCENE_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)
