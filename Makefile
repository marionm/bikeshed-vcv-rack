RACK_DIR ?= ../Rack-SDK

FLAGS +=
CFLAGS +=
CXXFLAGS += -Isrc
CXXFLAGS += -isystem extern
LDFLAGS +=

SOURCES += $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)

include $(RACK_DIR)/plugin.mk

ifdef ARCH_LIN
    # Only necessary for the github integration to work
    # This path works for the VCV Rack Plugin Toolchain Docker image from compose.yml
    LDFLAGS += -L/home/build/rack-plugin-toolchain/Rack-SDK-lin-x64/dep/lib64 \
               -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -ldl
endif

ifdef ARCH_WIN
    CXXFLAGS += -D_WIN32_WINNT=0x0A00 # Target Windows 10/11 for httplib
    LDFLAGS += -lws2_32 -lcrypt32
endif
