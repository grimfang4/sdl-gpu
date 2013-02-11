LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_gpu

RENDERER_DIR := OpenGLES_1
SOIL_DIR := externals/SOIL
VASE_REND_DIR := externals/vase_rend

LOCAL_CFLAGS := -I$(LOCAL_PATH)/../SDL/include -I$(LOCAL_PATH)/$(RENDERER_DIR) -I$(LOCAL_PATH)/$(SOIL_DIR) -I$(LOCAL_PATH)/$(VASE_REND_DIR)


BASE_FILES := $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/*.c))
RENDERER_FILES := $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/$(RENDERER_DIR)/*.c))

$(info Local: $(LOCAL_PATH))
$(info Soil: $(SOIL_DIR))
$(info Something: $(RENDERER_FILES))

LOCAL_SRC_FILES := $(BASE_FILES) \
				   $(RENDERER_DIR)/SDL_gpu_OpenGLES_1.c \
				   $(RENDERER_DIR)/SDL_gpuShapes_OpenGLES_1.c \
				   $(SOIL_DIR)/image_DXT.c \
				   $(SOIL_DIR)/image_helper.c \
				   $(SOIL_DIR)/SOIL.c \
				   $(SOIL_DIR)/stb_image.c \
				   $(SOIL_DIR)/stb_image_write.c


LOCAL_CFLAGS += -DSDL_GPU_USE_OPENGLES_1 -DSTBI_FAILURE_USERMSG
LOCAL_LDLIBS += -llog -lGLESv1_CM

LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)
