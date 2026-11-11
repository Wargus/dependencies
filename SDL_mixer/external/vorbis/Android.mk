LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vorbisdec

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_CFLAGS := -DHAVE_ALLOCA -DVAR_ARRAYS -DHAVE_SYS_TYPES_H

LOCAL_SRC_FILES += \
    lib/mdct.c \
    lib/smallft.c \
    lib/block.c \
    lib/envelope.c \
    lib/window.c \
    lib/lsp.c \
    lib/lpc.c \
    lib/analysis.c \
    lib/synthesis.c \
    lib/psy.c \
    lib/info.c \
    lib/floor1.c \
    lib/floor0.c\
    lib/res0.c \
    lib/mapping0.c \
    lib/registry.c \
    lib/codebook.c \
    lib/sharedbook.c \
    lib/lookup.c \
    lib/bitrate.c \
    lib/vorbisfile.c

LOCAL_SHARED_LIBRARIES := ogg

include $(BUILD_STATIC_LIBRARY)
