CORE_AVRDUDE = avrdude $(CORE_PROGRAMMER) -P $(CORE_PROGRAMMER_PORT) -p $(CORE_PROGRAMMED_DEVICE)
CORE_COMPILE = avr-gcc -g -Wall -Os -DF_CPU=$(CORE_CLOCK) -mmcu=$(CORE_DEVICE)
NETWORK_AVRDUDE = avrdude $(NETWORK_PROGRAMMER) -P $(NETWORK_PROGRAMMER_PORT) -p $(NETWORK_PROGRAMMED_DEVICE)
NETWORK_COMPILE = avr-gcc -g -Wall -Os -DF_CPU=$(NETWORK_CLOCK) -mmcu=$(NETWORK_DEVICE)

all: build/core.hex build/network.hex

clean:
	rm -rf build/* $(CORE_OBJECTS) $(NETWORK_OBJECTS)

#disasm: build/core.elf build/network.elf
#	avr-objdump -d -S build/core.elf > build/core.lss
#	avr-objdump -d -S build/network.elf > build/network.lss
    
disasm: build/core.lss build/network.lss

%.lss: %.elf
	avr-objdump -d -S $< > $@

core/%.o: core/%.c
	$(CORE_COMPILE) -c $< -o $@

network/%.o: network/%.c
	$(NETWORK_COMPILE) -c $< -o $@
    
build/core.elf: $(CORE_OBJECTS)
	$(CORE_COMPILE) -o build/core.elf $(CORE_OBJECTS)
	rm -rf $(CORE_OBJECTS) $(NETWORK_OBJECTS)

build/network.elf: $(NETWORK_OBJECTS)
	$(NETWORK_COMPILE) -o build/network.elf $(NETWORK_OBJECTS)
	rm -rf $(CORE_OBJECTS) $(NETWORK_OBJECTS)

build/core.hex: build/core.elf
	rm -f build/core.hex
	avr-objcopy -j .text -j .data -O ihex build/core.elf build/core.hex
	avr-size --format=avr --mcu=$(CORE_DEVICE) build/core.elf

build/network.hex: build/network.elf
	rm -f build/network.hex
	avr-objcopy -j .text -j .data -O ihex build/network.elf build/network.hex
	avr-size --format=avr --mcu=$(NETWORK_DEVICE) build/network.elf

core-flash: build/core.hex
	$(CORE_AVRDUDE) -U flash:w:build/core.hex:i
	$(CORE_AVRDUDE) $(CORE_FUSES)

network-flash: build/network.hex
	$(NETWORK_AVRDUDE) -U flash:w:build/network.hex:i
	$(NETWORK_AVRDUDE) $(NETWORK_FUSES)
