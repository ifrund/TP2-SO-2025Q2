// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdint.h>
#include "include/lib_math.h"
#include "include/naiveConsole.h"

#define VIDEO_ADDRESS (void *)0xB8000

static char buffer[64];
static uint8_t *video = (uint8_t *)VIDEO_ADDRESS;
static uint8_t *currentVideo = (uint8_t *)VIDEO_ADDRESS;
static const uint32_t width = 80;
static const uint32_t height = 25;

// *currentVideo -> char mostrado
// currentVideo -> direccion

//:================================================================================
//:=============================== BLANK PRINTS ===================================
//:================================================================================

void ncPrint(const char *string)
{
	int i;

	for (i = 0; string[i] != 0; i++)
		ncPrintChar(string[i]);
}

void ncPrintChar(char character)
{
	*currentVideo = character;
	currentVideo += 2;
}

void ncPrintCant(const char *string, int num)
{
	for (int i = 0; i < num; i++)
	{
		ncPrintChar(string[i]);
	}
}

//:================================================================================
//:=============================== COLOR PRINTS ===================================
//:================================================================================

void ncPrintColor(const char *string, int color)
{
	int i;

	for (i = 0; string[i] != 0; i++)
		ncPrintCharColor(string[i], color);
}

void ncPrintCharColor(char character, int color)
{
	*currentVideo = character;
	currentVideo++;
	*currentVideo = color;
	currentVideo++;
}

void ncPrintColorCant(const char *string, int num, int color)
{
	for (int i = 0; i < num; i++)
	{
		ncPrintCharColor(string[i], color);
	}
}

void ncNewline()
{
	do
	{
		ncPrintChar(' ');
	} while ((uint64_t)(currentVideo - video) % (width * 2) != 0);
}

void ncPrintDec(uint64_t value)
{
	ncPrintBase(value, 10);
}

void ncPrintHex(uint64_t value)
{
	ncPrintBase(value, 16);
}

void ncPrintBin(uint64_t value)
{
	ncPrintBase(value, 2);
}

void ncPrintBase(uint64_t value, uint32_t base)
{
	uintToBase(value, buffer, base);
	ncPrint(buffer);
}

void ncClear()
{
	int i;

	for (i = 0; i < height * width; i++)
		video[i * 2] = ' ';
	currentVideo = video;
}

uint8_t *getCurrentVideo()
{
	return currentVideo;
}

void setCurrentVideo(uint8_t *location)
{
	currentVideo = location;
}