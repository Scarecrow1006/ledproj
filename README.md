# User-Space Library for LED Panels

### Overview
This is a brief documentation for a user-space library for a HUB12 Protocol
based LED Panels.

Communication between the board and the panel is done using SPI Protocol and the
handshake signals are transmitted using GPIO. This library was developed to use
the sysfs interface for spidev and the default GPIO driver in RaspberryPi.

### Dependencies
This library uses [FreeType](https://www.freetype.org/) to render text Glyphs
using system fonts. This can be downloaded and installed for free from their 
website.

### Installation and Usage
Installation of the library is as simple as cloning this repository into your 
source tree. 

The library can be included in the source by adding the following line:
```c
#include "ledp10.h"
```

To complile your source, you would have to specify the following flags
```
-I(path to FreeType header files) -L(path to library files) -lfreetype -lpthread
```
This would typically look something like:
```
-I/usr/include/freetype2 -L/usr/local/lib -lfreetype -lpthread
```
The ```ledp10.c``` source file must also be compiled with the rest of the code.

### Hardware build
#### What you will need

- RaspberryPi configured with Raspbian.
- LEDP10 16x32 monocolour panel.
- 5V (approx 2A for all LEDs on) power supply.
- Jumpers

#### How to proceed
![HUB12 Pinout](/hub12.png)

### Functional Description

#### Basic Variables
```c
typedef struct ledp10 * LEDP10_T
```
The user creates an instance of `LEDP10_T`. This variable is used by the library 
for all it's functions. 

#### Library Initialization
```c
int ledp10_init(LEDP10_T *panel, char *spi, char *a, 
					char *b, char *lat, int img_width);
```
##### Parameters
1. `LEDP10_T *panel`: A pointer to the ledp10 structure to be initialised.
2. `char *spi`: Path to the SPI Device file. Typically something like 
	`"/dev/spidev0.0".
3. `char *a`: Path to the gpio/value file for pin 2(A).
4. `char *b`: Path to the gpio/value file for pin 4(B).
5. `char *lat`: Path to the gpio/value file for the GPIO pin to be used as STR.
6. `int img_width`: Length of the required Image buffer. This can not be
	changed later.
	
##### Returns
`0` on success and `-1` on failure. On success `panel` is initialized to required 
values and memory is allocated to the structure and it's components using
`malloc()`. On success a new thread is created that interacts with the SPI and 
GPIO hardware. This thread rapidluy switches between the four channels to give an
illusion of a continuous display.

#### Selecting a font
```c
int ledp10_font(LEDP10_T panel, char *font)
```
##### Parameters
1. `LEDP10_T panel`: An ledp10 structure previously initialised using `ledp10_init()`.
2. `char *foint`: Path to the font file. Freetype is capable of producing high
	quality output for most vector and bitmap font formats.
	
##### Returns
`0` on success and `-1` on failure. `panel` is set up to load glyphs from the 
specified font file for UTF-8 encoding format on success.

#### Render text for specified font
```c
int ledp10_render(LEDP10_T panel, char *str)
```
##### Parameters
1. `LEDP10_T panel`: An ledp10 structure initialized using `ledp10_init()` and for
	which a font has been chosen using `ledp10_font`.
2. `char *str`: The text string to be rendered.

##### Returns
`0` on success and `-1` on failure. On success the image buffer in `panel` is filled
with the pixel data for `str`. If the string is too long for the buffer, it is 
truncated to the required size. This buffer can then be previewed using 
`ledp10_preview()`.

#### Preview Rendered Image
```c
void ledp10_preview(LEDP10_T panel)
```
##### Parameters
1. `LEDP10_T panel`: An ledp10 struture that contains an image rendered using
	`ledp10_render()`. 
	
Prints the image buffer to stdout. Prints in a graphical format where every `0` is
printed as a `.` and any other greyscale value is printed as `*`.

#### Display to LED Panel
```c
void ledp10_display(LEDP10_T panel, int start_col)
```
##### Parameters
1. `LEDP10_T panel`: The ledp10 structure to be displayed.
2. `int start_col`: The first column to be displayed.

Displays the image buffer in `panel` on the LED Panel associated with the struct
starting at column number `start_col`. Keep `start_col=0` to display first 32
columns of the buffer.
