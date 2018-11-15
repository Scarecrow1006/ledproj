#ifndef _LEDP10H
#define _LEDP10H

#include <stdint.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct ledp10
{
	int spifd;
	int gpioa;
	int gpiob;
	int gpioc;
	int panel_height;
	int panel_width;
	int img_width;
	uint8_t **image;
	uint8_t **buf;
	FT_Library lib;
	FT_Face face;
	FT_UInt index;
	FT_GlyphSlot slot;
	int pipefd[2];
	pthread_t child;
	int prtscr;
};

typedef struct ledp10* LEDP10_T;

int ledp10_init(LEDP10_T *panel, char *spi, char *a, char *b, char *lat,
		int img_width);
void *drivepanel(void *arg);
void transfer(int fd, uint8_t* tx);
int ledp10_font(LEDP10_T panel, char *font);
int ledp10_render(LEDP10_T panel, char *str);
void draw_in_buf(FT_Bitmap *bmp, FT_Int x, FT_Int y, uint8_t **image);
void ledp10_display(LEDP10_T panel, int start_col);
void img2buf(LEDP10_T panel, int start_col);
void ledp10_stop(LEDP10_T panel);
void ledp10_exit(LEDP10_T panel);

#define WIMAGE 0
#define PANEL_HEIGHT 16
#define PANEL_WIDTH 32
#endif
