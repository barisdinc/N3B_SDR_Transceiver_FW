#ifndef ILI9341_H
#define ILI9341_H

#include "pico/stdlib.h"

#define ILI9341_WIDTH  320     //240
#define ILI9341_HEIGHT 240    // 320

// Pin tanımlamaları
#define ILI9341_CS_PIN    12
#define ILI9341_RST_PIN    5
#define ILI9341_DC_PIN     4
#define ILI9341_SCK_PIN    10
#define ILI9341_MOSI_PIN   11

// Renk tanımları
#define ILI9341_BLACK       0x0000
#define ILI9341_NAVY        0x000F
#define ILI9341_DARKGREEN   0x03E0
#define ILI9341_DARKCYAN    0x03EF
#define ILI9341_MAROON      0x7800
#define ILI9341_PURPLE      0x780F
#define ILI9341_OLIVE       0x7BE0
#define ILI9341_LIGHTGREY   0xC618
#define ILI9341_DARKGREY    0x7BEF
#define ILI9341_BLUE        0x001F
#define ILI9341_GREEN       0x07E0
#define ILI9341_CYAN        0x07FF
#define ILI9341_RED         0xF800
#define ILI9341_MAGENTA     0xF81F
#define ILI9341_YELLOW      0xFFE0
#define ILI9341_WHITE       0xFFFF
#define ILI9341_ORANGE      0xFD20
#define ILI9341_GREENYELLOW 0xAFE5
#define ILI9341_PINK        0xF81F

// ILI9341 commands
#define ILI9341_SWRESET 0x01
#define ILI9341_SLPOUT  0x11
#define ILI9341_DISPON  0x29
#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C

void ili9341_init();
void ili9341_write_command(uint8_t cmd);
void ili9341_write_data(uint8_t data);
void ili9341_set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ili9341_fill_screen(uint16_t color);
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void ili9341_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ili9341_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);  // Bu fonksiyon eklendi
void ili9341_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void ili9341_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
void ili9341_set_rotation(uint8_t rotation);  // Ekran döndürme fonksiyonu
void ili9341_set_scroll_area(uint16_t top_fixed_area, uint16_t bottom_fixed_area);
void ili9341_set_vertical_scroll(uint16_t tfa, uint16_t vsa, uint16_t bfa);
void ili9341_scroll(uint16_t scroll);
void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_writePixel(uint16_t color);
void ILI9341_copyFrameBufferToDisplay(uint16_t* frameBuffer, uint16_t width, uint16_t height);


// Animasyon fonksiyonları
void ili9341_animate_text(const char *str, uint16_t color, uint16_t bg, uint8_t size, uint16_t speed);
void ili9341_animate_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t speed);

void ili9341_bouncing_rect(uint16_t w, uint16_t h, uint16_t color, uint16_t speed);
void ili9341_expanding_circles(uint16_t x, uint16_t y, uint16_t max_r, uint16_t color, uint16_t speed);
// Global değişkenler
extern uint16_t width;
extern uint16_t height;

#endif