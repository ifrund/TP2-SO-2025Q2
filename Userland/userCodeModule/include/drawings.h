#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>

//================================================================================================================================
//colors
#define LIGHTGRAY 0x00B8B8B8
#define WHITELINE 0x00FBFBFB
#define WHITE 0x00F6F6F6
#define BLACK 0x00000000
#define DARKGRAY 0x00414141
#define DARKERGRAY 0x00343434
#define RED 0x00FF290E

#define WHITETEXT 0x00FFFFFF

#define SCREEN_BG_COLOR1 0x008ED17F
#define SCREEN_BG_COLOR2 0x0086C578
#define SCREEN_BG_MARGIN 0x0086C578

//title colors
#define S_COLOR 0x00FF0000
#define N_COLOR 0x000000FF
#define A_COLOR 0x00FF8900
#define K_COLOR 0x00FFF300
#define E_COLOR 0x0000FF00

#define SKYBLUE 0x00010053
#define TITLEGOLD 0x00FAEF02

//================================================================================================================================
//bitmaps
#define dibHeight 16
#define dibWidth 16

extern uint16_t square[][dibHeight];

extern uint16_t apple[][dibHeight];

extern uint16_t snakehead_left[][dibHeight];
extern uint16_t snakehead_right[][dibHeight];
extern uint16_t snakehead_up[][dibHeight];
extern uint16_t snakehead_down[][dibHeight];

extern uint16_t snaketail_left[][dibHeight];
extern uint16_t snaketail_right[][dibHeight];
extern uint16_t snaketail_up[][dibHeight];
extern uint16_t snaketail_down[][dibHeight];

extern uint16_t snakecurve_downright[][dibHeight];
extern uint16_t snakecurve_downleft[][dibHeight];
extern uint16_t snakecurve_upright[][dibHeight];
extern uint16_t snakecurve_upleft[][dibHeight];

extern uint16_t snakebody_horizontal1[][dibHeight];
extern uint16_t snakebody_horizontal2[][dibHeight];
extern uint16_t snakebody_vertical[][dibHeight];

//================================================================================================================================
//functions

//================================================================================================================================
// dibujan cuerpo de la snake.
//================================================================================================================================

void draw_snakehead_left(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakehead_right(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakehead_up(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakehead_down(uint64_t init_x, uint64_t init_y, uint32_t traincolor);

void draw_snaketail_left(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snaketail_right(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snaketail_up(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snaketail_down(uint64_t init_x, uint64_t init_y, uint32_t traincolor);

void draw_snakecurve_downright(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakecurve_downleft(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakecurve_upright(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakecurve_upleft(uint64_t init_x, uint64_t init_y, uint32_t traincolor);

void draw_snakebody_horizontal1(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakebody_horizontal2(uint64_t init_x, uint64_t init_y, uint32_t traincolor);
void draw_snakebody_vertical(uint64_t init_x, uint64_t init_y, uint32_t traincolor);

void draw_snake(uint16_t bitmap[][dibHeight], uint16_t layers, uint32_t colors[], uint64_t init_x, uint64_t init_y, uint32_t traincolor);

//================================================================================================================================
// dibuja manzana.
//================================================================================================================================

void draw_manzana(uint64_t init_x, uint64_t init_y);

//================================================================================================================================
//title for snake
extern uint16_t snake_title[][dibHeight];

extern uint16_t select_player[][dibHeight];

extern uint16_t points_digits[][dibHeight];

extern uint16_t gameover_text[][dibHeight];

//================================================================================================================================
// dibuja titulo de juego.
//================================================================================================================================

void draw_snake_letter(uint16_t bitmap[][dibHeight], uint64_t init_x, uint64_t init_y, uint32_t color, uint8_t drawSize);

void putSnakeTitle();

//================================================================================================================================
// Manejo movimiento teclas.
//================================================================================================================================

void selectHover(uint8_t selection);

//================================================================================================================================
// Background Drawing Logic.
//================================================================================================================================

uint32_t getBgColor(int column, int row);
void putBg(uint8_t width, uint8_t height, uint64_t bg_initialx, uint64_t bg_initialy, uint8_t squareWidth, uint8_t squareHeight);

void selectColor(uint8_t selection, uint32_t* color1, uint32_t* color2);

//================================================================================================================================
// Main Menu Stuff
//================================================================================================================================

extern uint16_t star[][dibHeight];

uint8_t mainMenu();
void menuHover(uint8_t selection, int optionX1, int optionX2, int optionsY);

#endif
