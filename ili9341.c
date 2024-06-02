#include "ili9341.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include <stdlib.h>
#include <string.h>
#include "Font.h"
/*
// Basit 5x7 font (ASCII 32-127)
static const uint8_t font[] = {
    // ASCII 32-127 font data here...
    // This part of the array should contain the bitmaps for ASCII characters.
    // Example for 'A' (65) : 0x7E, 0x11, 0x11, 0x11, 0x7E,
};

*/

static uint8_t rotation = 0;
uint16_t width = ILI9341_WIDTH;
uint16_t height = ILI9341_HEIGHT;

// Prototipler
void ili9341_write_command(uint8_t cmd);
void ili9341_write_data(uint8_t data);
void ili9341_set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ili9341_fill_screen(uint16_t color);
void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void ili9341_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void ili9341_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color);
void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void ili9341_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);
void ili9341_set_rotation(uint8_t rotation);  // Ekran döndürme fonksiyonu
void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ILI9341_writePixel(uint16_t color);
void ILI9341_copyFrameBufferToDisplay(uint16_t* frameBuffer, uint16_t width, uint16_t height);

void ili9341_init() {
    // GPIO pin ayarları
    gpio_init(ILI9341_CS_PIN);
    gpio_set_dir(ILI9341_CS_PIN, GPIO_OUT);
    gpio_init(ILI9341_RST_PIN);
    gpio_set_dir(ILI9341_RST_PIN, GPIO_OUT);
    gpio_init(ILI9341_DC_PIN);
    gpio_set_dir(ILI9341_DC_PIN, GPIO_OUT);

    // SPI ayarları
    spi_init(spi1, 40000 * 1000);
    gpio_set_function(ILI9341_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(ILI9341_MOSI_PIN, GPIO_FUNC_SPI);

    // Reset işlemi
    gpio_put(ILI9341_RST_PIN, 0);
    sleep_ms(100);
    gpio_put(ILI9341_RST_PIN, 1);
    sleep_ms(100);

    // İnisyalizasyon komutları
    ili9341_write_command(0xEF);
    ili9341_write_data(0x03);
    ili9341_write_data(0x80);
    ili9341_write_data(0x02);

    ili9341_write_command(0xCF);
    ili9341_write_data(0x00);
    ili9341_write_data(0xC1);
    ili9341_write_data(0x30);

    ili9341_write_command(0xED);
    ili9341_write_data(0x64);
    ili9341_write_data(0x03);
    ili9341_write_data(0x12);
    ili9341_write_data(0x81);

    ili9341_write_command(0xE8);
    ili9341_write_data(0x85);
    ili9341_write_data(0x00);
    ili9341_write_data(0x78);

    ili9341_write_command(0xCB);
    ili9341_write_data(0x39);
    ili9341_write_data(0x2C);
    ili9341_write_data(0x00);
    ili9341_write_data(0x34);
    ili9341_write_data(0x02);

    ili9341_write_command(0xF7);
    ili9341_write_data(0x20);

    ili9341_write_command(0xEA);
    ili9341_write_data(0x00);
    ili9341_write_data(0x00);

    ili9341_write_command(0xC0);    // Power control
    ili9341_write_data(0x23);       // VRH[5:0]

    ili9341_write_command(0xC1);    // Power control
    ili9341_write_data(0x10);       // SAP[2:0];BT[3:0]

    ili9341_write_command(0xC5);    // VCM control
    ili9341_write_data(0x3e);       // Contrast
    ili9341_write_data(0x28);

    ili9341_write_command(0xC7);    // VCM control
    ili9341_write_data(0x86);       // --

    ili9341_write_command(0x36);    // Memory Access Control
    ili9341_write_data(0x48);       // Rotation

    ili9341_write_command(0x3A);
    ili9341_write_data(0x55);

    ili9341_write_command(0xB1);
    ili9341_write_data(0x00);
    ili9341_write_data(0x18);

    ili9341_write_command(0xB6);    // Display Function Control
    ili9341_write_data(0x08);
    ili9341_write_data(0x82);
    ili9341_write_data(0x27);

    ili9341_write_command(0xF2);    // 3Gamma Function Disable
    ili9341_write_data(0x00);

    ili9341_write_command(0x26);    // Gamma curve selected
    ili9341_write_data(0x01);

    ili9341_write_command(0xE0);    // Set Gamma
    ili9341_write_data(0x0F);
    ili9341_write_data(0x31);
    ili9341_write_data(0x2B);
    ili9341_write_data(0x0C);
    ili9341_write_data(0x0E);
    ili9341_write_data(0x08);
    ili9341_write_data(0x4E);
    ili9341_write_data(0xF1);
    ili9341_write_data(0x37);
    ili9341_write_data(0x07);
    ili9341_write_data(0x10);
    ili9341_write_data(0x03);
    ili9341_write_data(0x0E);
    ili9341_write_data(0x09);
    ili9341_write_data(0x00);

    ili9341_write_command(0xE1);    // Set Gamma
    ili9341_write_data(0x00);
    ili9341_write_data(0x0E);
    ili9341_write_data(0x14);
    ili9341_write_data(0x03);
    ili9341_write_data(0x11);
    ili9341_write_data(0x07);
    ili9341_write_data(0x31);
    ili9341_write_data(0xC1);
    ili9341_write_data(0x48);
    ili9341_write_data(0x08);
    ili9341_write_data(0x0F);
    ili9341_write_data(0x0C);
    ili9341_write_data(0x31);
    ili9341_write_data(0x36);
    ili9341_write_data(0x0F);

    ili9341_write_command(0x11);    // Exit Sleep
    sleep_ms(120);
    ili9341_write_command(0x29);    // Display on
}

void ili9341_write_command(uint8_t cmd) {
    gpio_put(ILI9341_DC_PIN, 0);
    gpio_put(ILI9341_CS_PIN, 0);
    spi_write_blocking(spi1, &cmd, 1);
    gpio_put(ILI9341_CS_PIN, 1);
}

void ili9341_write_data(uint8_t data) {
    gpio_put(ILI9341_DC_PIN, 1);
    gpio_put(ILI9341_CS_PIN, 0);
    spi_write_blocking(spi1, &data, 1);
    gpio_put(ILI9341_CS_PIN, 1);
}

void ili9341_set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ili9341_write_command(0x2A); // Column addr set
    ili9341_write_data(x0 >> 8);
    ili9341_write_data(x0 & 0xFF);
    ili9341_write_data(x1 >> 8);
    ili9341_write_data(x1 & 0xFF);

    ili9341_write_command(0x2B); // Row addr set
    ili9341_write_data(y0 >> 8);
    ili9341_write_data(y0 & 0xFF);
    ili9341_write_data(y1 >> 8);
    ili9341_write_data(y1 & 0xFF);

    ili9341_write_command(0x2C); // Write to RAM
}

void ili9341_fill_screen(uint16_t color) {
    ili9341_set_address_window(0, 0, width - 1, height - 1);
    uint8_t data[] = {color >> 8, color & 0xFF};
    gpio_put(ILI9341_DC_PIN, 1);
    gpio_put(ILI9341_CS_PIN, 0);
    for (uint32_t i = 0; i < width * height; i++) {
        spi_write_blocking(spi1, data, 2);
    }
    gpio_put(ILI9341_CS_PIN, 1);
}

void ili9341_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if ((x >= width) || (y >= height)) return;
    ili9341_set_address_window(x, y, x + 1, y + 1);
    uint8_t data[] = {color >> 8, color & 0xFF};
    gpio_put(ILI9341_DC_PIN, 1);
    gpio_put(ILI9341_CS_PIN, 0);
    spi_write_blocking(spi1, data, 2);
    gpio_put(ILI9341_CS_PIN, 1);
}

void ili9341_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;

    while (1) {
        ili9341_draw_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void ili9341_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ili9341_draw_line(x, y, x + w - 1, y, color);
    ili9341_draw_line(x, y, x, y + h - 1, color);
    ili9341_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
    ili9341_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
}

void ili9341_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if ((x >= width) || (y >= height)) return;
    if ((x + w - 1) >= width) w = width - x;
    if ((y + h - 1) >= height) h = height - y;

    ili9341_set_address_window(x, y, x + w - 1, y + h - 1);
    uint8_t data[] = {color >> 8, color & 0xFF};
    gpio_put(ILI9341_DC_PIN, 1);
    gpio_put(ILI9341_CS_PIN, 0);
    for (uint32_t i = 0; i < w * h; i++) {
        spi_write_blocking(spi1, data, 2);
    }
    gpio_put(ILI9341_CS_PIN, 1);
}

void ili9341_draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    ili9341_draw_pixel(x0, y0 + r, color);
    ili9341_draw_pixel(x0, y0 - r, color);
    ili9341_draw_pixel(x0 + r, y0, color);
    ili9341_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        ili9341_draw_pixel(x0 + x, y0 + y, color);
        ili9341_draw_pixel(x0 - x, y0 + y, color);
        ili9341_draw_pixel(x0 + x, y0 - y, color);
        ili9341_draw_pixel(x0 - x, y0 - y, color);
        ili9341_draw_pixel(x0 + y, y0 + x, color);
        ili9341_draw_pixel(x0 - y, y0 + x, color);
        ili9341_draw_pixel(x0 + y, y0 - x, color);
        ili9341_draw_pixel(x0 - y, y0 - x, color);
    }
}

void ili9341_draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
    if ((x >= width) || (y >= height)) return;
    if ((x + 6 * size - 1) >= width || (y + 8 * size - 1) >= height) return;

    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = font[(c - 32) * 5 + i];
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                if (size == 1)
                    ili9341_draw_pixel(x + i, y + j, color);
                else
                    ili9341_fill_rect(x + i * size, y + j * size, size, size, color);
            } else if (bg != color) {
                if (size == 1)
                    ili9341_draw_pixel(x + i, y + j, bg);
                else
                    ili9341_fill_rect(x + i * size, y + j * size, size, size, bg);
            }
        }
    }
    if (bg != color) {
        if (size == 1)
            ili9341_draw_line(x + 5, y, x + 5, y + 7, bg);
        else
            ili9341_fill_rect(x + 5 * size, y, size, 8 * size, bg);
    }
}

void ili9341_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        ili9341_draw_char(x, y, *str, color, bg, size);
        x += 6 * size;
        str++;
    }
}

void ili9341_set_rotation(uint8_t m) {
    rotation = m % 4;
    ili9341_write_command(0x36);
    switch (rotation) {
        case 0:
            ili9341_write_data(0x48);
            width = 240;
            height = 320;
            break;
        case 1:
            ili9341_write_data(0x28);
            width = 320;
            height = 240;
            break;
        case 2:
            ili9341_write_data(0x88);
            width = 240;
            height = 320;
            break;
        case 3:
            ili9341_write_data(0xE8);
            width = 320;
            height = 240;
            break;
    }
}

void ili9341_animate_text(const char *str, uint16_t color, uint16_t bg, uint8_t size, uint16_t speed) {
    uint16_t length = strlen(str) * 6 * size;
    for (int16_t x = width; x >= -length; x--) {
        ili9341_fill_screen(bg);
        ili9341_draw_string(x, (height - 8 * size) / 2, str, color, bg, size);
        sleep_ms(speed);
    }
}

void ili9341_animate_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t speed) {
    for (int16_t i = 0; i < height; i++) {
        ili9341_fill_screen(ILI9341_BLACK);
        ili9341_draw_rect(x, y + i, w, h, color);
        sleep_ms(speed);
    }
}



void ili9341_bouncing_rect(uint16_t w, uint16_t h, uint16_t color, uint16_t speed) {
    int16_t x = (width - w) / 2;
    int16_t y = (height - h) / 2;
    int16_t dx = 2, dy = 2;

    while (true) {
        ili9341_fill_screen(ILI9341_BLACK);
        ili9341_fill_rect(x, y, w, h, color);
        sleep_ms(speed);

        x += dx;
        y += dy;

        if (x <= 0 || x + w >= width) dx = -dx;
        if (y <= 0 || y + h >= height) dy = -dy;
    }
}

void ili9341_expanding_circles(uint16_t x, uint16_t y, uint16_t max_r, uint16_t color, uint16_t speed) {
    for (uint16_t r = 0; r <= max_r; r++) {
        ili9341_fill_screen(ILI9341_BLACK);
        ili9341_draw_circle(x, y, r, color);
        sleep_ms(speed);
    }
}


// Vertical scrolling functions

void ili9341_set_vertical_scroll(uint16_t tfa, uint16_t vsa, uint16_t bfa) {
    ili9341_write_command(0x33);
    ili9341_write_data(tfa >> 8);
    ili9341_write_data(tfa & 0xFF);
    ili9341_write_data(vsa >> 8);
    ili9341_write_data(vsa & 0xFF);
    ili9341_write_data(bfa >> 8);
    ili9341_write_data(bfa & 0xFF);
}

void ili9341_set_scroll_area(uint16_t top_fixed_area, uint16_t bottom_fixed_area) {
    uint16_t scroll_area = height - top_fixed_area - bottom_fixed_area;
    ili9341_write_command(0x33);
    ili9341_write_data(top_fixed_area >> 8);
    ili9341_write_data(top_fixed_area & 0xFF);
    ili9341_write_data(scroll_area >> 8);
    ili9341_write_data(scroll_area & 0xFF);
    ili9341_write_data(bottom_fixed_area >> 8);
    ili9341_write_data(bottom_fixed_area & 0xFF);
}

void ili9341_scroll(uint16_t scroll) {
    ili9341_write_command(0x37);
    ili9341_write_data(scroll >> 8);
    ili9341_write_data(scroll & 0xFF);
}


void ILI9341_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    ili9341_write_command(ILI9341_CASET);
    ili9341_write_data(x0 >> 8);
    ili9341_write_data(x0 & 0xFF);
    ili9341_write_data(x1 >> 8);
    ili9341_write_data(x1 & 0xFF);

    ili9341_write_command(ILI9341_PASET);
    ili9341_write_data(y0 >> 8);
    ili9341_write_data(y0 & 0xFF);
    ili9341_write_data(y1 >> 8);
    ili9341_write_data(y1 & 0xFF);

    ili9341_write_command(ILI9341_RAMWR);
}

void ILI9341_writePixel(uint16_t color) {
    ili9341_write_data(color);
}

void ILI9341_copyFrameBufferToDisplay(uint16_t* frameBuffer, uint16_t width, uint16_t height) {

    gpio_put(ILI9341_DC_PIN, 1);
    gpio_put(ILI9341_CS_PIN, 0);
    
    for (uint32_t i = 0; i < width*height; i++) {
        // if (*frameBuffer > 0)  printf("%d \n", *frameBuffer);
        spi_write_blocking(spi1,frameBuffer++, 2);//was 1

    }
    gpio_put(ILI9341_CS_PIN, 1);
    // sleep_ms(2000);


    // TFT_CS_PORT |= (1 << TFT_CS_PIN);
}