#ifndef GDM12864H
#define GDM12864H

#include <stdint.h>
#include <avr/io.h>

#define GDM12864H_DATA_DDR DDRA
#define GDM12864H_DATA_PORT PORTA

#define GDM12864H_AUX_DDR DDRC
#define GDM12864H_AUX_PORT PORTC
#define GDM12864H_DATA_INSTRUCTION_PIN PC6
#define GDM12864H_READ_WRITE_PIN PC5
#define GDM12864H_ENABLE_PIN PC7
#define GDM12864H_CS1_PIN PC3
#define GDM12864H_CS2_PIN PC4

void gdm12864h_init(void);
void gdm12864h_set_pixel(uint8_t x, uint8_t y, uint8_t color);
void gdm12864h_refresh(void);

#endif