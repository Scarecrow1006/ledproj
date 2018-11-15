#include "ledp10.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define DEVICE "/dev/spidev0.0"
#define GPIOA "/sys/class/gpio/gpio2/value"
#define GPIOB "/sys/class/gpio/gpio3/value"
#define GPIOC "/sys/class/gpio/gpio4/value"

LEDP10_T x;

int main(int argc, char * argv[])
{
	int i=0;
	if(argc!=3)
	{
		printf("Usage: %s font_file string\n", argv[0]);
		return -1;
	}

	if(ledp10_init(&x, DEVICE, GPIOA, GPIOB, GPIOC, 150)==-1)
	{
		printf("init failed\n");
		return -1;
	}

	/*if(ledp10_set(x, WIMAGE, 150)==-1)
	{
		printf("set-option failed");
		return -1;
	}*/

	if(ledp10_font(x, argv[1])==-1)
	{
		printf("Font load failed\n");
		return -1;
	}

	if(ledp10_render(x, argv[2])==-1)
	{
		printf("Could not render text");
		return -1;
	}

	while(1)
	{
		ledp10_display(x, i);
		i=(i+1)%150;
		usleep(100000);
	}
	return 0;
}
