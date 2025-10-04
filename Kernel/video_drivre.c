#include <videoDriver.h>
#include <fonts.h>
#include <lib.h>
#include <stdint.h>



#define ERROR_FONT 0xDADADA
#define ERROR_BACK 0xa70000

typedef struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;	    // height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
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
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
}__attribute__((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;
uint16_t pitch; 
uint16_t width; 
uint8_t bpp;

static char buffer[64] = {'0'};


uint16_t cursor_location_x = 0x0000;
uint16_t cursor_location_y = 0x0000;

//================================================================================================================================
// Getters for videoMode
//================================================================================================================================
//================================================================================================================================
uint16_t getScreenHeight(){
	return VBE_mode_info->height;
}

uint16_t getScreenWidth(){
	return VBE_mode_info->width;
}

//================================================================================================================================
// Draw for videoMode
//================================================================================================================================
//================================================================================================================================
uint8_t DRAW_SIZE = 3; //size for bitmaps

void changeDrawSize(uint8_t size){
	DRAW_SIZE = size;
}

uint8_t getDrawSize(){
	return DRAW_SIZE;
}

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y){
	if( (x < getScreenWidth()) && (y < getScreenHeight())){
		uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
		uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch); //3 ya que son RGB(3) (24bits)
		framebuffer[offset]=(hexColor) & 0x000000FF;	//agarro la parte baja del hexColor que es el azul
		framebuffer[offset+1]=(hexColor >> 8) & 0x0000FF;
		framebuffer[offset+2]=(hexColor >> 16) & 0x00FF;
        
	}
	if( (x < getScreenWidth()) && (y < getScreenHeight())){
		uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
		uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch); //3 ya que son RGB(3) (24bits)
		framebuffer[offset]=(hexColor) & 0x000000FF;	//agarro la parte baja del hexColor que es el azul
		framebuffer[offset+1]=(hexColor >> 8) & 0x0000FF;
		framebuffer[offset+2]=(hexColor >> 16) & 0x00FF;
	}
}

void draw_rectangle(uint64_t ancho, uint64_t alto, uint32_t color, uint64_t init_x, uint64_t init_y){
	for(uint64_t i=0; i<alto; i++){
		for(uint64_t j=0;j<ancho;j++){
			putPixel(color,j+init_x,i+init_y);
		}
	}
}

void printBitmap(uint16_t * bitmap, uint32_t color, uint16_t alto ,uint64_t x, uint64_t y){
	for(uint64_t i=0; i<(alto*DRAW_SIZE); i++){
		for(uint64_t j=0; j<(16*DRAW_SIZE); j++){//16 is because of the bitmap datasize (uint16_t)
			if( ((bitmap[(i/DRAW_SIZE)]<<(j/DRAW_SIZE)) & (1<<(15))) ){// 1<<charWidth permite leer de a un bit de izq a der del row de la font (fixed in 15 in this implementation)
				putPixel(color,x+j,y+i);
			}
		}
	}
}

//================================================================================================================================
// Text for videoMode
//================================================================================================================================
//================================================================================================================================

uint8_t SCALE = 1;	//size variable

void changeFontSize(uint8_t size){
	SCALE = size;
}

uint8_t getFontSize(){
	return SCALE;
}

void putChar(char character, uint32_t colorFont, uint32_t colorBg, uint64_t init_x, uint64_t init_y){
	for(uint64_t i=0; i<(charHeight*SCALE); i++){
		for(uint64_t j=0; j<(charWidth*SCALE); j++){
			if( ((font[(int) character][(i/SCALE)]<<(j/SCALE)) & (1<<(charWidth-1))) ){// 1<<charWidth permite leer de a un bit de izq a der del row de la font
				putPixel(colorFont,init_x+j,init_y+i);
			}
			else{
				putPixel(colorBg,init_x+j,init_y+i);
			}
			
		}
	}
}

void printChar(char character){
    printCharColor(character, DEFAULT_FONT, DEFAULT_BACK);
}

void printCharColor(char character, uint32_t fontColor, uint32_t bgColor){
    if (character == 0XA){
        newLine();
    }

    else if (character == '\b'){
        delChar();
    }


    else{
        putChar(character, fontColor, bgColor, cursor_location_x, cursor_location_y);
        cursor_location_x += (charWidth * SCALE);
        if(cursor_location_x >= (VBE_mode_info->width)){
            cursor_location_x = 0;
            cursor_location_y += (charHeight * SCALE);
        }
    }
}

void print(const char * string){
	for(int i=0; string[i]!=0;)
        i = process_input(string, i, DEFAULT_FONT, DEFAULT_BACK); 
}

void printCant(const char * string, uint64_t cant){
	for(int i=0; string[i]!=0 && i<cant;)
        i = process_input(string, i, DEFAULT_FONT, DEFAULT_BACK); 
}

void printColor(const char * string, uint32_t fontColor, uint32_t bgColor){
	for(int i=0; string[i]!=0;)
		i = process_input(string, i, fontColor, bgColor);
}

void printColorCant(const char * string, uint64_t cant, uint32_t fontColor, uint32_t bgColor){
	for(int i=0; string[i]!=0 && i<cant;)
		i = process_input(string, i, fontColor, bgColor);
}


void printError(const char * string){
	for(int i=0; string[i]!=0;)
        i = process_input(string, i, ERROR_FONT, ERROR_BACK); 
}

// Number stuff
void printBaseError(uint64_t value, uint32_t base){
    uintToBase(value, buffer, base);
    printColor(buffer, ERROR_FONT, ERROR_BACK);
}

void printBase(uint64_t value, uint32_t base){
    uintToBase(value, buffer, base);
    print(buffer);
}

void printDec(uint64_t value){
    printBase(value, 10);
}

void printDecError(uint64_t value){
    printBaseError(value, 10);
}

void printHex(uint64_t value){
    printBase(value, 16);
}



void newLine(){
    cursor_location_y += charHeight * SCALE;
    cursor_location_x = 0;
}

void delChar(){
    repoCursor();
    printChar(' ');
    repoCursor();
}


void repoCursor(){
    if (!cursor_location_x){
        cursor_location_y -= charHeight * SCALE;
        cursor_location_x = VBE_mode_info->width - charWidth * SCALE;
    }
    else
        cursor_location_x -= charWidth * SCALE;
}

void clear(){
    cursor_location_x = 0;
    cursor_location_y = 0;

    for (int y = 0; y < VBE_mode_info->height; y++){
        for (int x = 0; x < VBE_mode_info->width; x++){
            putPixel(DEFAULT_BACK, x, y);
        }
    }
}

//================================================================================================================================
//========= INPUT PROCESSING
//================================================================================================================================
int process_input(const char* string, int index, uint32_t fontColor, uint32_t bgColor){
    char to_process = string[index];

    if (to_process == '\n'){
        newLine();
        return index + 1;
    }

    else if (to_process == '\b'){
        delChar();
        return index + 1;
    }

    else if (to_process == '\033'){
        if (string[index + 1] != '[')
            return index + 1;

        if (string[index + 2] >= '0' && string[index + 2] <= '9'){
            if (string[index + 3] == 'F'){
                changeFontSize(string[index + 2] - '0');
                return index + 4;
            }
        }

        else if (string[index + 2] == 'J') {
                clear();
                return index + 3;
        }

        else if (string[index + 2] == 'C') {
                flush_buffer();
                return index + 3;
        }
        else
            return index + 1;
    }

    else {
        printCharColor(string[index], fontColor, bgColor);
    }

	return index + 1;
}

//================================================================================================================================
