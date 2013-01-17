LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_gpu

RENDERER_DIR := OpenGLES_1
SOIL_DIR := $(RENDERER_DIR)/SOIL

LOCAL_CFLAGS := -I$(LOCAL_PATH)/../SDL/include -I$(LOCAL_PATH)/$(RENDERER_DIR) -I$(LOCAL_PATH)/$(SOIL_DIR)

LOCAL_SRC_FILES := SDL_gpu.c \
				   SDL_gpu_Renderer.c \
				   SDL_gpuShapes.c \
				   $(RENDERER_DIR)/SDL_gpu_OpenGLES_1.c \
				   $(RENDERER_DIR)/SDL_gpuShapes_OpenGLES_1.c \
				   $(SOIL_DIR)/image_DXT.c \
				   $(SOIL_DIR)/image_helper.c \
				   $(SOIL_DIR)/SOIL.c \
				   $(SOIL_DIR)/stb_image.c \
				   $(SOIL_DIR)/stb_image_write.c


LOCAL_CFLAGS += -DSDL_GPU_USE_OPENGLES_1
LOCAL_LDLIBS += -llog -lGLESv1_CM

LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)
