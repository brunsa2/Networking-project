COMMON_DIRECTORY = src/common/
CORE_DIRECTORY = src/core/
NETWORK_DIRECTORY = src/network/
BUILD_DIRECTORY = bin/

COMMON_SOURCES := $(wildcard $(COMMON_DIRECTORY)*.c)
COMMON_ASM_SOURCES := $(wildcard $(COMMON_DIRECTORY)*.S)
CORE_SOURCES := $(wildcard $(CORE_DIRECTORY)*.c)
CORE_ASM_SOURCES := $(wildcard $(CORE_DIRECTORY)*.S)
NETWORK_SOURCES := $(wildcard $(NETWORK_DIRECTORY)*.c)
NETWORK_ASM_SOURCES := $(wildcard $(NETWORK_DIRECTORY)*.S)

COMMON_OBJECTS := $(patsubst %.c,%.o,$(COMMON_SOURCES)) $(patsubst %.S,%.o,$(COMMON_ASM_SOURCES))
CORE_OBJECTS := $(patsubst %.c,%.o,$(CORE_SOURCES)) $(patsubst %.S,%.o,$(CORE_ASM_SOURCES))
NETWORK_OBJECTS := $(patsubst %.c,%.o,$(NETWORK_SOURCES)) $(patsubst %.S,%.o,$(NETWORK_ASM_SOURCES))

CORE_AVRDUDE = avrdude $(CORE_PROGRAMMER) -P $(CORE_PROGRAMMER_PORT) -p $(CORE_PROGRAMMED_DEVICE)
CORE_COMPILE = avr-gcc -g -Wall -Os -DF_CPU=$(CORE_CLOCK) -mmcu=$(CORE_DEVICE)
NETWORK_AVRDUDE = avrdude $(NETWORK_PROGRAMMER) -P $(NETWORK_PROGRAMMER_PORT) -p $(NETWORK_PROGRAMMED_DEVICE)
NETWORK_COMPILE = avr-gcc -g -Wall -Os -DF_CPU=$(NETWORK_CLOCK) -mmcu=$(NETWORK_DEVICE)

all: $(BUILD_DIRECTORY)core.hex $(BUILD_DIRECTORY)network.hex

clean:
	rm -rf $(BUILD_DIRECTORY)* $(COMMON_OBJECTS) $(CORE_OBJECTS) $(NETWORK_OBJECTS)
    
disasm: $(BUILD_DIRECTORY)core.lss $(BUILD_DIRECTORY)network.lss

%.lss: %.elf
	avr-objdump -d -S $< > $@

$(CORE_DIRECTORY)%.o: $(CORE_DIRECTORY)%.c
	$(CORE_COMPILE) -c $< -o $@

$(NETWORK_DIRECTORY)%.o: $(NETWORK_DIRECTORY)%.c
	$(NETWORK_COMPILE) -c $< -o $@

$(NETWORK_DIRECTORY)%.o: $(NETWORK_DIRECTORY)%.S
	$(NETWORK_COMPILE) -x assembler-with-cpp -c $< -o $@
    
$(BUILD_DIRECTORY)core.elf: $(CORE_OBJECTS)
	$(CORE_COMPILE) -o $(BUILD_DIRECTORY)core.elf $(CORE_OBJECTS) $(COMMON_SOURCES) $(COMMON_ASM_SOURCES)

$(BUILD_DIRECTORY)network.elf: $(NETWORK_OBJECTS)
	$(NETWORK_COMPILE) -o $(BUILD_DIRECTORY)network.elf $(NETWORK_OBJECTS) $(COMMON_SOURCES) $(COMMON_ASM_SOURCES)

$(BUILD_DIRECTORY)core.hex: $(BUILD_DIRECTORY)core.elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIRECTORY)core.elf $(BUILD_DIRECTORY)core.hex
	avr-size --format=avr --mcu=$(CORE_DEVICE) $(BUILD_DIRECTORY)core.elf

$(BUILD_DIRECTORY)network.hex: $(BUILD_DIRECTORY)network.elf
	avr-objcopy -j .text -j .data -O ihex $(BUILD_DIRECTORY)network.elf $(BUILD_DIRECTORY)network.hex
	avr-size --format=avr --mcu=$(NETWORK_DEVICE) $(BUILD_DIRECTORY)network.elf

core-flash: $(BUILD_DIRECTORY)core.hex
	$(CORE_AVRDUDE) -U flash:w:$(BUILD_DIRECTORY)core.hex:i
	$(CORE_AVRDUDE) $(CORE_FUSES)

network-flash: $(BUILD_DIRECTORY)network.hex
	$(NETWORK_AVRDUDE) -U flash:w:$(BUILD_DIRECTORY)network.hex:i
	$(NETWORK_AVRDUDE) $(NETWORK_FUSES)
