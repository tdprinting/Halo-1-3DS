# Halo 1 for New 3DS. Requires devkitARM + libctru + citro3d (devkitPro).
ifeq ($(strip $(DEVKITARM)),)
$(error "Set DEVKITARM, e.g. export DEVKITARM=/opt/devkitpro/devkitARM")
endif
include $(DEVKITARM)/3ds_rules

TARGET   := halo3ds
BUILD    := build
SOURCES  := source/platform source/render3ds
INCLUDES := include source/platform source/render3ds $(BUILD)/shaders
APP_TITLE := Halo 1
APP_DESCRIPTION := Halo: Combat Evolved (decomp port, WIP)
APP_AUTHOR := halo-1-3ds

ARCH     := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS   := -g -Wall -O2 -mword-relocations -ffunction-sections $(ARCH) \
            $(addprefix -I,$(INCLUDES)) -I$(CTRULIB)/include -I$(PORTLIBS)/include -D__3DS__
LDFLAGS  := -specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(BUILD)/$(TARGET).map
LIBS     := -lcitro3d -lctru -lm
LIBDIRS  := $(CTRULIB) $(PORTLIBS)
LDFLAGS  += $(addprefix -L,$(addsuffix /lib,$(LIBDIRS)))

CFILES   := $(foreach d,$(SOURCES),$(wildcard $(d)/*.c))
OFILES   := $(patsubst %.c,$(BUILD)/%.o,$(CFILES)) $(BUILD)/shaders/ctr_basic.shbin.o

all: $(TARGET).3dsx

# Shaders: picasso -> .shbin -> bin2s -> object + header (<name>_shbin.h)
$(BUILD)/shaders/%.shbin: shaders/%.v.pica
	@mkdir -p $(dir $@)
	picasso -o $@ $<

$(BUILD)/shaders/%.shbin.o $(BUILD)/shaders/%_shbin.h: $(BUILD)/shaders/%.shbin
	cd $(dir $<) && bin2s -a 4 -H $*_shbin.h $*.shbin | $(AS) -o $*.shbin.o

$(BUILD)/source/render3ds/ctr_gpu.o: $(BUILD)/shaders/ctr_basic_shbin.h

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OFILES)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).3dsx $(TARGET).smdh

.PHONY: all clean
