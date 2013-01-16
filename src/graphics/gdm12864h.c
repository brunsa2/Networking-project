#include "gdm12864h.h"

#define GDM12864H_DATA_INPUT (GDM12864H_DATA_DDR = 0x00)

void gdm12864h_init(void) {
    volatile uint16_t delay_counter;
    
    // Set up aux pins
    GDM12864H_AUX_DDR = (1 << GDM12864H_DATA_INSTRUCTION_PIN) |
            (1 << GDM12864H_READ_WRITE_PIN) | (1 << GDM12864H_ENABLE_PIN) |
            (1 << GDM12864H_CS1_PIN) | (1 << GDM12864H_CS2_PIN);
    
    // Data is input for now
    GDM12864H_DATA_INPUT;
    // CS1/CS2 high, the rest low
    GDM12864H_AUX_PORT |= (1 << GDM12864H_READ_WRITE_PIN) | (1 << GDM12864H_ENABLE_PIN) | (1 << GDM12864H_DATA_INSTRUCTION_PIN);
    GDM12864H_AUX_PORT &= ~((1 << GDM12864H_CS1_PIN) | (1 << GDM12864H_CS2_PIN));
    
    // Delay while this sets up
    for (delay_counter = 50000; delay_counter != 0; delay_counter--);
    
    // Run startup sequence
    
    
}

void gdm12864h_set_pixel(uint8_t x, uint8_t y, uint8_t color) {

}

void gdm12864h_refresh(void) {
    
}