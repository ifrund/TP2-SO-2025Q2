// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include "include/fonts.h"
#include "include/lib_math.h"
#include "include/lib_mem.h"
#include "include/videoDriver.h"

#define ERROR_FONT 0xDADADA
#define ERROR_BACK 0xa70000

struct vbe_mode_info_structure
{
    uint16_t attributes;  // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;     // deprecated
    uint8_t window_b;     // deprecated
    uint16_t granularity; // deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr; // deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;        // number of bytes per horizontal line
    uint16_t width;        // width in pixels
    uint16_t height;       // height in pixels
    uint8_t w_char;        // unused...
    uint8_t y_char;        // ...
    uint8_t planes;
    uint8_t bpp;   // bits per pixel in this mode
    uint8_t banks; // deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size; // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer; // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size; // size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} __attribute__((packed));

typedef struct vbe_mode_info_structure *VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr)0x0000000000005C00;
uint16_t pitch;
uint16_t width;
uint8_t bpp;

static char buffer[64];

static uint16_t cursorX = 0x0000;
static uint16_t cursorY = 0x0000;

//================================================================================================================================
// Getters for videoMode
//================================================================================================================================
//================================================================================================================================
uint16_t getScreenHeight()
{
    return VBE_mode_info->height;
}

uint16_t getScreenWidth()
{
    return VBE_mode_info->width;
}

//================================================================================================================================
// Draw for videoMode
//================================================================================================================================
//================================================================================================================================
uint8_t DRAW_SIZE = 3; // size for bitmaps

void changeDrawSize(uint8_t size)
{
    DRAW_SIZE = size;
}

uint8_t getDrawSize()
{
    return DRAW_SIZE;
}

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y)
{
    uint8_t *framebuffer = (uint8_t *)(uintptr_t)VBE_mode_info->framebuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp) / 8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset] = (hexColor) & 0xFF;
    framebuffer[offset + 1] = (hexColor >> 8) & 0xFF;
    framebuffer[offset + 2] = (hexColor >> 16) & 0xFF;
}

void draw_rectangle(uint64_t ancho, uint64_t alto, uint32_t color, uint64_t init_x, uint64_t init_y)
{
    for (uint64_t i = 0; i < alto; i++)
    {
        for (uint64_t j = 0; j < ancho; j++)
        {
            putPixel(color, j + init_x, i + init_y);
        }
    }
}

void printBitmap(uint16_t *bitmap, uint32_t color, uint16_t alto, uint64_t x, uint64_t y)
{
    for (uint64_t i = 0; i < (alto * DRAW_SIZE); i++)
    {
        for (uint64_t j = 0; j < (16 * DRAW_SIZE); j++)
        { // 16 is because of the bitmap datasize (uint16_t)
            if (((bitmap[(i / DRAW_SIZE)] << (j / DRAW_SIZE)) & (1 << (15))))
            { // 1<<charWidth permite leer de a un bit de izq a der del row de la font (fixed in 15 in this implementation)
                putPixel(color, x + j, y + i);
            }
        }
    }
}

//================================================================================================================================
// Text for videoMode
//================================================================================================================================
//================================================================================================================================

uint8_t SCALE = 1; // size variable

void changeFontSize(uint8_t size)
{
    SCALE = size;
}

uint8_t getFontSize()
{
    return SCALE;
}

#define SCALED_CHARACTER_WIDTH (charWidth * SCALE)
#define SCALED_CHARACTER_HEIGHT (charHeight * SCALE)
#define WIDTH_IN_CHARS (VBE_mode_info->width)
#define HEIGHT_IN_CHARS (VBE_mode_info->height)

void putChar(char character, uint32_t colorFont, uint32_t colorBg, uint64_t init_x, uint64_t init_y)
{
    for (uint64_t i = 0; i < (charHeight * SCALE); i++)
    {
        for (uint64_t j = 0; j < (charWidth * SCALE); j++)
        {
            if (((font[(int)character][(i / SCALE)] << (j / SCALE)) & (1 << (charWidth - 1))))
            { // 1<<charWidth permite leer de a un bit de izq a der del row de la font
                putPixel(colorFont, init_x + j, init_y + i);
            }
            else
            {
                putPixel(colorBg, init_x + j, init_y + i);
            }
        }
    }
}

void printChar(char character)
{
    printCharColor(character, DEFAULT_FONT, DEFAULT_BACK);
}

void printCharColor(char character, uint32_t fontColor, uint32_t bgColor)
{
    if (cursorX == WIDTH_IN_CHARS)
    {
        newLine();
    }

    switch (character)
    {
    case '\n':
        newLine();
        break;
    case '\b':
        delChar();
        break;
    case '\t':
        print("    ");
        break;
    default:
        putChar(character, fontColor, bgColor, cursorX, cursorY);
        cursorX += SCALED_CHARACTER_WIDTH;
        if (cursorX >= WIDTH_IN_CHARS)
            newLine();
    }
}

void print(const char *string)
{
    for (int i = 0; string[i] != 0;)
        i = process_input(string, i, DEFAULT_FONT, DEFAULT_BACK);
}

void printCant(const char *string, uint64_t cant)
{
    for (int i = 0; string[i] != 0 && i < cant;)
        i = process_input(string, i, DEFAULT_FONT, DEFAULT_BACK);
}

void printColor(const char *string, uint32_t fontColor, uint32_t bgColor)
{
    for (int i = 0; string[i] != 0;)
        i = process_input(string, i, fontColor, bgColor);
}

void printColorCant(const char *string, uint64_t cant, uint32_t fontColor, uint32_t bgColor)
{
    for (int i = 0; string[i] != 0 && i < cant;)
        i = process_input(string, i, fontColor, bgColor);
}

void printError(const char *string)
{
    for (int i = 0; string[i] != 0;)
        i = process_input(string, i, ERROR_FONT, ERROR_BACK);
}

// Number stuff
void printBaseError(uint64_t value, uint32_t base)
{
    uintToBase(value, buffer, base);
    printColor(buffer, ERROR_FONT, ERROR_BACK);
}

void printBase(uint64_t value, uint32_t base)
{
    uintToBase(value, buffer, base);
    print(buffer);
}

void printDec(uint64_t value)
{
    printBase(value, 10);
}

void printDecError(uint64_t value)
{
    printBaseError(value, 10);
}

void printHex(uint64_t value)
{
    printBase(value, 16);
}

/**
 * Salta una linea y vuelve al principio de linea. Si se termina la pantalla, scrollea
 */
void newLine()
{
    cursorX = 0;
    if (cursorY / SCALED_CHARACTER_HEIGHT < HEIGHT_IN_CHARS - 2)
    {
        cursorY += SCALED_CHARACTER_HEIGHT; // avanzo normalmente, sigo teniendo pantalla
    }
    else
    { // si llegue a la anteultima linea
        // como la pantalla esta en memoria, voy a copiar a partir de la segunda linea y pegarlo todo de vuelta
        // en el framebuffer. luego limpio la ultima linea para volver a escribir
        uintptr_t framebuffer = (uintptr_t)VBE_mode_info->framebuffer;
        uint16_t height = VBE_mode_info->height;
        uint16_t pitch = VBE_mode_info->pitch;
        /*
         * memcpy(destino, origen, bytes)
         * destino: framebuffer (inicio de la pantalla)
         * origen: segunda linea de la pantalla (pitch es bytes por linea de pixeles)
         * bytes: toda la pantalla menos las ultimas dos lineas
         */
        memcpy((void *)framebuffer,
               (void *)(framebuffer + pitch * SCALED_CHARACTER_HEIGHT),
               pitch * (height - SCALED_CHARACTER_HEIGHT * 2));
        /*
         * memset(destino, valor, bytes)
         * destino: anteultima linea
         * bytes: toda la linea
         */
        memset((void *)(framebuffer + pitch * (height - SCALED_CHARACTER_HEIGHT * 2)), 0, pitch * SCALED_CHARACTER_HEIGHT);
    }
}

/**
 * Elimina el ultimo caracter ingresado, con soporte para saltos de linea
 */
void delChar()
{
    repoCursor();
    printChar(' ');
    repoCursor();
}

void repoCursor()
{
    if (cursorX == 0 && cursorY > 0)
    {
        cursorY -= SCALED_CHARACTER_HEIGHT;
        cursorX = WIDTH_IN_CHARS;
    }
    cursorX -= SCALED_CHARACTER_WIDTH;
}

void clear()
{
    for (int i = 0; i < VBE_mode_info->width; i++)
    {
        for (int j = 0; j < VBE_mode_info->height; j++)
        {
            putPixel(DEFAULT_BACK, i, j);
        }
    }
    cursorX = 0;
    cursorY = 0;
}

//================================================================================================================================
//========= INPUT PROCESSING
//================================================================================================================================
int process_input(const char *string, int index, uint32_t fontColor, uint32_t bgColor)
{
    char to_process = string[index];

    if (to_process == '\n')
    {
        newLine();
        return index + 1;
    }

    else if (to_process == '\b')
    {
        delChar();
        return index + 1;
    }

    else if (to_process == '\033')
    {
        if (string[index + 1] != '[')
            return index + 1;

        if (string[index + 2] >= '0' && string[index + 2] <= '9')
        {
            if (string[index + 3] == 'F')
            {
                changeFontSize(string[index + 2] - '0');
                return index + 4;
            }
        }

        else if (string[index + 2] == 'J')
        {
            clear();
            return index + 3;
        }

        else if (string[index + 2] == 'C')
        {
            flush_buffer();
            return index + 3;
        }
        else
            return index + 1;
    }

    else
    {
        printCharColor(string[index], fontColor, bgColor);
    }

    return index + 1;
}

//================================================================================================================================
