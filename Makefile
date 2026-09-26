#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)

include $(DEVKITPRO)/wups/share/wups_rules

WUT_ROOT := $(DEVKITPRO)/wut
WUMS_ROOT := $(DEVKITPRO)/wums

TARGET   := NUSspliBackgroundDownloadManager
BUILD    := build
SOURCES  := src
INCLUDES := include

CFLAGS := -Wall -O3 -ffunction-sections $(MACHDEP)
CFLAGS += $(INCLUDE) -D__WIIU__ -D__WUT__ -D__WUPS__

CXXFLAGS := $(CFLAGS) -std=c++20

ASFLAGS := -g $(ARCH)
LDFLAGS = -g $(ARCH) $(RPXSPECS) -Wl,-Map,$(notdir $*.map) -T$(WUMS_ROOT)/share/libmappedmemory.ld $(WUPSSPECS)

ifeq ($(DEBUG),1)
CFLAGS += -DDEBUG -g
CXXFLAGS += -DDEBUG -g
endif

ifeq ($(DEBUG),VERBOSE)
CFLAGS += -DDEBUG -DVERBOSE_DEBUG -g
CXXFLAGS += -DDEBUG -DVERBOSE_DEBUG -g
endif

LIBS := -lwups -lwut
LIBDIRS := $(PORTLIBS) $(WUPS_ROOT) $(WUT_ROOT) $(WUMS_ROOT)

ifneq ($(BUILD),$(notdir $(CURDIR)))

export OUTPUT := $(CURDIR)/$(TARGET)
export TOPDIR := $(CURDIR)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES_SRC := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES := $(OFILES_SRC)

export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(CURDIR)/$(BUILD)

export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

all: $(BUILD)

$(BUILD):
	@$(shell [ ! -d $(BUILD) ] && mkdir -p $(BUILD))
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).wps $(TARGET).elf

else

.PHONY: all

DEPENDS := $(OFILES:.o=.d)

all: $(OUTPUT).wps

$(OUTPUT).wps: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
