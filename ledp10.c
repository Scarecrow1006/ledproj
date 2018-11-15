#include "ledp10.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <pthread.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <ft2build.h>
#include FT_FREETYPE_H

int ledp10_init(LEDP10_T *panel, char *spi, char *a, char *b, char *lat,
		int img_width)
{
	int i;

	*panel=(LEDP10_T) malloc(sizeof(struct ledp10));
	(*panel)->spifd=open(spi, O_RDWR);
	if((*panel)->spifd==-1)
	{
		perror("LEDP10: Could not open spi device");
		return -1;
	}
	(*panel)->gpioa=open(a, O_WRONLY);
	if((*panel)->gpioa==-1)
	{
		perror("LEDP10: Could not open gpio device A");
		return -1;
	}
	(*panel)->gpiob=open(b, O_WRONLY);
	if((*panel)->gpiob==-1)
	{
		perror("LEDP10: Could not open gpio device B");
		return -1;
	}
	(*panel)->gpioc=open(lat, O_WRONLY);
	if((*panel)->spifd==-1)
	{
		perror("LEDP10: Could not open gpio device LAT");
		return -1;
	}

	(*panel)->panel_height=PANEL_HEIGHT;
	(*panel)->panel_width=PANEL_WIDTH;
	(*panel)->img_width=img_width;

	(*panel)->image=(uint8_t **) malloc((*panel)->panel_height*
			sizeof(uint8_t *));
	if((*panel)->image==NULL)
	{
		perror("LEDP10: Could not allocate enough memory");
		return -1;
	}
	for(i=0; i<(*panel)->panel_height; i++)
	{
		(*panel)->image[i]=(uint8_t *) malloc((*panel)->img_width);
		if((*panel)->image[i]==NULL)
		{
			perror("LEDP10: Could not allocate enough memory");
			return -1;
		}
	}

	(*panel)->buf=(uint8_t **) malloc((*panel)->panel_height>>2 *
			sizeof(uint8_t *));
	if((*panel)->buf==NULL)
	{
		perror("LEDP10: Could not allocate enough memory");
		return -1;
	}
	for(i=0; i<(*panel)->panel_height>>2; i++)
	{
		(*panel)->buf[i]=(uint8_t *) malloc((*panel)->panel_width>>1);
		if((*panel)->buf[i]==NULL)
		{
			perror("LEDP10: Could not allocate enough memory");
			return -1;
		}
	}

	if(FT_Init_FreeType(&((*panel)->lib)))
	{
		printf("LEDP10: Freetype Library initialization failed\n");
		return -1;
	}

	if(pthread_create(&((*panel)->child), NULL, 
				drivepanel, (void *)(*panel))!=0)
	{
		printf("LEDP10: Could not start thread\n");
		return -1;
	}

	(*panel)->prtscr=0;
}

void *drivepanel(void *arg)
{
	LEDP10_T panel=(LEDP10_T) arg;
	unsigned char one='1';
	unsigned char zero='0';
	int ret=0;
	uint8_t mode;
	uint8_t bits = 8;
	uint32_t speed = 500000;
	int g2=panel->gpioa;
	int g3=panel->gpiob;
	int g4=panel->gpioc;
	uint8_t **buf=panel->buf;

	ret = ioctl(panel->spifd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
	{
		perror("can't set spi mode");
		return NULL;
	}

	ret = ioctl(panel->spifd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
	{
		perror("can't get spi mode");
		return NULL;
	}

	/*
	 * bits per word
	 */
	ret = ioctl(panel->spifd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		perror("can't set bits per word");
		return NULL;
	}

	ret = ioctl(panel->spifd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
	{
		perror("can't get bits per word");
		return NULL;
	}

	/*
	 * max speed hz
	 */
	ret = ioctl(panel->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		perror("can't set max speed hz");
		return NULL;
	}

	ret = ioctl(panel->spifd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
	{
		perror("can't get max speed hz");
		return NULL;
	}

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	while (1)
	{
		while(!panel->prtscr);

		write(g2, &zero, 1);
		write(g3, &zero, 1);
		transfer(panel->spifd, buf[1]);
		usleep(2000);
		write(g4, &one, 1);
		write(g4, &zero, 1);
		
		write(g2, &one, 1);
		write(g3, &zero, 1);
		transfer(panel->spifd, buf[2]);
		usleep(2000);
		write(g4, &one, 1);
		write(g4, &zero, 1);

		write(g2, &zero, 1);
		write(g3, &one, 1);
		transfer(panel->spifd, buf[0]);
		usleep(2000);
		write(g4, &one, 1);
		write(g4, &zero, 1);

		write(g2, &one, 1);
		write(g3, &one, 1);
		transfer(panel->spifd, buf[3]);
		usleep(2000);
		write(g4, &one, 1);
		write(g4, &zero, 1);
	}

	return NULL;
}

void transfer(int fd, uint8_t* tx)
{
	write(fd, tx, 16);
}

int ledp10_font(LEDP10_T panel, char *font)
{
	if(FT_New_Face(panel->lib, font, 0, &(panel->face)))
	{
		printf("font could not be loaded\n");
		return -1;
	}

	if(FT_Set_Pixel_Sizes(panel->face, 0, 16))
	{
		printf("could not set font size\n");
		return -1;
	}
}

int ledp10_render(LEDP10_T panel, char *str)
{
	int i;
	int len=strlen(str);
	int penx=0, peny=13;

	for(i=0; i<len; i++)
	{
		if(FT_Load_Char(panel->face, str[i], FT_LOAD_RENDER))
		{
			printf("Glyph render error\n");
			return -1;
		}

		panel->slot=panel->face->glyph;
		if(penx+(panel->slot->advance.x >> 6) > panel->img_width)
			continue;

		draw_in_buf(&(panel->slot->bitmap),
				penx+panel->slot->bitmap_left,
				peny-panel->slot->bitmap_top,
				panel->image);

		penx+=panel->slot->advance.x>>6;
		peny+=panel->slot->advance.y>>6;
	}
	return 0;
}

void draw_in_buf(FT_Bitmap *bmp, FT_Int x, FT_Int y, uint8_t **image)
{
	int i,j;
	int xmax, ymax;
	int p,q;

	xmax=x+bmp->width;
	ymax=y+bmp->rows;

	for(i=x; i<xmax; i++)
	{
		for(j=y; j<ymax; j++)
		{
			p=i-x;
			q=j-y;
			image[j][i]=bmp->buffer[q*bmp->width+p];
		}
	}
}

void ledp10_display(LEDP10_T panel, int start_col)
{
	img2buf(panel, start_col);
	panel->prtscr=1;
}

void img2buf(LEDP10_T panel, int start_col)
{
	int row, col, chan, i;
	uint8_t **image=panel->image;
	uint8_t **buf=panel->buf;
	for(chan=0; chan<4; chan++)
	{
	for(col=0; col<4; col++)
	{
	for(row=0; row<4; row++)
	{
		buf[chan][16-(col*4+4-row)]=0;
		for(i=0; i<8; i++)
		{
			buf[chan][16-(col*4+4-row)]|=
				((image[row*4+chan]
				  [(col*8+i+start_col) % (panel->img_width)]==0)
				? 1:0) << i;
		}
	}
	}
	}
}

void ledp10_stop(LEDP10_T panel)
{
	panel->prtscr=0;
}

void ledp10_exit(LEDP10_T panel)
{
	int i;
	panel->prtscr=0;
	for(i=0; i<panel->panel_height; i++)
		free(panel->image[i]);
	free(panel->image);
	for(i=0; i<4; i++)
		free(panel->buf[i]);
	free(panel->buf);
	FT_Done_Face(panel->face);
	FT_Done_FreeType(panel->lib);
	free(panel);
}
