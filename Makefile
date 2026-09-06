RACK_DIR ?= ../Rack-SDK

FLAGS +=
CFLAGS +=
CXXFLAGS += -Isrc
CXXFLAGS += -isystem
LDFLAGS +=

SOURCES += $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp)

ifeq ($(ARCH_WIN),1)
EXTRA_LDFLAGS += -lsupc++
endif

DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard presets)
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += src/extern/signalsmith-linear/LICENSE.txt
DISTRIBUTABLES += src/extern/signalsmith-stretch/LICENSE.txt

include $(RACK_DIR)/plugin.mk
