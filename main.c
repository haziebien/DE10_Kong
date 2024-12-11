/*

 * main.c
 *
 *  Created on: Dec 2, 2024
 *      Author: "The Kongers"
 */


#include "altera_up_avalon_video_pixel_buffer_dma.h"
#include <altera_up_avalon_video_character_buffer_with_dma.h>
#include "altera_up_avalon_accelerometer_spi.h"
#include "altera_avalon_pio_regs.h"
#include "system.h"
#include <stdio.h>
#include <unistd.h>
// For Randomizer
#include <stdlib.h>
#include <time.h>

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BURGUNDY 0xc066
#define BROWN 0x8261
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define LADDER_HORIZONTAL_SPEED 1 		// >0 How fast you climb ladder
#define HORIZONTAL_SPEED 1				// >0 How fast you move sideways
#define AIR_HORIZONTAL_ACCEL 0.1		// >0 How much freedom you have to move sideways midair (small values for minor control)
#define JUMP_SPEED 2					// >0 How fast you jump
#define JUMP_ADJUSTMENT_ACCEL 0.33		// 0-GRAVITY How much freedom you have to adjust jump height (must be less than gravity, or you will fly)
#define MAXIMUM_FALL_SPEED 5			// >0 Maximum speed that you fall
#define X_FRICTION 0.9					// 0-1 How quickly you come to a stop in both ground and air
#define X_GROUND_FRICTION 0.5			// 0-1 How quickly you stop on the ground
#define GRAVITY 0.4 					// >0 How quickly you accelerate downward at ALL TIMES

#define ss_dark	0b11111111;
#define ss_0 	0b11000000;
#define ss_1	0b11111001;
#define ss_2	0b10100100;
#define ss_3	0b10110000;
#define ss_4	0b10011001;
#define ss_5	0b10010010;
#define ss_6	0b10000010;
#define ss_7	0b11111000;
#define ss_8	0b10000000;
#define ss_9	0b10011000;
#define ss_a	0b10001000;
#define ss_b	0b10000011;
#define ss_c	0b10100111;
#define ss_d	0b10100001;
#define ss_e	0b10000110;
#define ss_f	0b10001110;

typedef enum {
    FALSE = 0,
    TRUE = 1
} bool;

typedef enum {
    IDLE = 0,
    WALKING = 1,
	JUMPING = 2,
	CLIMBING = 3
} state;

typedef enum {
	RIGHT = 0,
	LEFT = 1
} state_direction;

typedef struct{
	float x, y, w, h, xvel, yvel, xprev, yprev, dir; // position, dimensions, velocity, previous position, direction
} DynamicObject;
typedef struct{
	int x, y, w, h; // position, dimensions, velocity, previous position
} StaticObject;

const int player_mushroom_bitmap[] = {
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x2a2a, 0xf8f8, 0x9b2a, 0x8800, 0x8800, 0x1700, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0xcece, 0xf8f8, 0xf8f8, 0x2a2a, 0xb800, 0xb800, 0xedce, 0xcece, 0x0000, 0x0000,
		0x0000, 0x7c7c, 0xf8f8, 0xf8f8, 0xf87c, 0xf800, 0xf800, 0xf800, 0xc300, 0xf8f8, 0x7c7c, 0x0000,
		0x0000, 0x7c7c, 0xf8f8, 0xf8ce, 0xf800, 0xf8ce, 0xf8f8, 0xf8f8, 0xf82a, 0x2a2a, 0x7c7c, 0x0000,
		0x1700, 0xa000, 0xed00, 0xf800, 0xf87c, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xed00, 0xa000, 0x1700,
		0x1700, 0xc07c, 0xf8f8, 0xf800, 0xf87c, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xed00, 0xa000, 0x1700,
		0x2a2a, 0xf8f8, 0xf8f8, 0xf8ce, 0xf87c, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xed00, 0xd87c, 0x2a2a,
		0x2a2a, 0xf8f8, 0xf8f8, 0xedce, 0xb800, 0xedce, 0xf8f8, 0xf8f8, 0x2a2a, 0x2a2a, 0xf8f8, 0x2a2a,
		0x1700, 0xc07c, 0xf8f8, 0x8800, 0x8800, 0x8800, 0x8800, 0x8800, 0x8800, 0x9b2a, 0xf8f8, 0x2a2a,
		0x1700, 0x8800, 0x8800, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x8800, 0xc07c, 0x2a2a,
		0x0000, 0x0000, 0x0000, 0xf8f8, 0x7c7c, 0xcece, 0xcece, 0x7c7c, 0xf8f8, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0xcece, 0xf8f8, 0x7c7c, 0xcece, 0xcece, 0x7c7c, 0xf8f8, 0xcece, 0x0000, 0x0000,
		0x0000, 0x0000, 0xcece, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xcece, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0xf8f8, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
};
const int player_kong_bitmap[] = {
		0x0000, 0x0000, 0x0000, 0x0000, 0x0800, 0xa9c2, 0xa181, 0x3060, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x7121, 0xf54e, 0xd3e9, 0xaa85, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x1860, 0x5141, 0xd50e, 0xbaa5, 0xfed8, 0xee15, 0xe48c, 0xdd2e, 0x5141, 0x1860, 0x0000,
		0x0000, 0xb304, 0xa161, 0xe48b, 0xd3c9, 0xfdd0, 0xdc4b, 0xe4cc, 0xe48c, 0xa161, 0xb304, 0x0000,
		0x3901, 0xba42, 0xa161, 0xeced, 0xeced, 0xdc0a, 0xd3c9, 0xdc4a, 0xed2e, 0xa161, 0xba42, 0x3901,
		0xcb44, 0xa161, 0xa161, 0xcb68,	0xe4ac, 0xfdf0, 0xfdf0, 0xfdd0, 0xcba8, 0xa161, 0xa161, 0xd364,
		0xb283, 0xa161, 0xa161, 0xa161, 0xcb88, 0xfdd0, 0xfdd0, 0xe48c,	0xa161, 0xa161, 0xa161, 0xbaa3,
		0xa9c1, 0xa161, 0xa9c1, 0xa9c1, 0xa9a1, 0xa9a1, 0xa9a1, 0xa9a1, 0xa9c1, 0xa9c1, 0xa161, 0xa9a1,
		0xc282, 0xa161, 0xdb44, 0xa161, 0xf50c, 0xeccb, 0xeccb, 0xf50c, 0xb1e1, 0xdb44, 0xa161, 0xba62,
		0xb2a3, 0xa161, 0xcac3, 0xe3c5,	0xfd8e, 0xfd6c, 0xfd4c, 0xfd8e, 0xe3a4, 0xc2a3, 0xa161, 0xbaa3,
		0x7981, 0xa161, 0xd324, 0xecaa, 0xed2d, 0xfdd0, 0xfdd0, 0xed2d,	0xdbc7, 0xd303, 0xa161, 0x7981,
		0x2040, 0xa161, 0xfcc8, 0xe44a, 0xec68, 0xfc87, 0xfc87, 0xec68, 0xe44a, 0xfcc9, 0xa161, 0x2040,
		0x0000, 0xc283, 0xd40a, 0xc2c4, 0xec47, 0xfc65, 0xfc65, 0xec47, 0xc2c4, 0xdc2a, 0xc283, 0x0000,
		0x0000, 0xa9c1, 0xf52d, 0xba63,	0xe4ab, 0xfdae, 0xfdae, 0xe4ab, 0xb243, 0xf52d, 0xa9c1, 0x0000,
		0x3123, 0xbae6, 0xb244, 0xc2e6, 0x4941, 0x8a83, 0x8a83, 0x4941,	0xbac5, 0xba85, 0xbae6, 0x3123,
		0xdccb, 0xfdae, 0xf56e, 0xc42b, 0x0000, 0x0000, 0x0000, 0x0000, 0xbc2b, 0xf56f, 0xed2e, 0xe4aa
};

const int player_bitmap[] = {
		0x0000, 0x0000, 0xf7fe, 0xf1c0, 0xf1c0, 0xf1c0, 0xf980, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0xf9a1, 0xf9a0, 0xf9a0, 0xf9a0, 0xf1c0, 0xf9a0, 0xf9a0, 0xf1c0, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0018, 0x0017, 0x0037, 0x0036, 0xf676, 0xf696, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0035, 0xf675, 0xf675, 0x0017, 0xf676, 0xf695, 0x0016, 0xee96, 0xf676, 0xf695, 0x0000,
		0x0000, 0x0017, 0xf676, 0xf676, 0x0015, 0x0018, 0xf676, 0xee75, 0x0058, 0xee96, 0xee96, 0xee94,
		0x0036, 0x0017, 0x0017, 0xf677, 0xf696, 0xf696, 0xf677, 0x0017, 0x0017, 0x0017, 0x0017, 0x0000,
		0x0000, 0x0000, 0xffdf, 0xf676, 0xf676, 0xf676, 0xf696, 0xf677, 0xf696, 0xf675, 0xffdf, 0x0000,
		0x0000, 0x0000, 0x0015, 0x0036, 0x0017, 0x0017, 0x0017, 0x0017, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0016, 0x0017, 0x0017, 0x1016, 0xe9e0, 0xf1a2, 0x0017, 0x0037, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0017, 0x0017, 0x0017, 0xf9c0, 0xf1c0, 0xf695, 0xf1a0, 0xf1c0, 0x0000, 0x0000, 0x0000,
		0xfffe, 0x0017, 0x0058, 0x0017, 0xe9e0, 0xf1c0, 0xf9a0, 0xf1c0, 0xf9c0, 0xf1c0, 0x0000, 0x0000,
		0xffdf, 0xf9a0, 0x0038, 0xee76, 0xf695, 0xee76, 0xf9a0, 0xf9a0, 0xf9c0, 0xf9c0, 0x0000, 0x0000,
		0x0000, 0xf9c0, 0xf9a1, 0xf695, 0xf697, 0xf1c0, 0xf9c0, 0xf9c0, 0xf1a0, 0xf1c0, 0x0000, 0x0000,
		0x0000, 0xf1a0, 0xf1a0, 0xf1c0, 0xf9a0, 0x0000, 0xf1a0, 0xf1a0, 0xf1c0, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0017, 0x0017, 0x0017, 0xfffe, 0x0000, 0x0017, 0x0017, 0x0017, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0017, 0x0017, 0x0017, 0x0017, 0x0000, 0x0017, 0x0017, 0x0018, 0x0017, 0x0000, 0x0000,
};

void draw_block(alt_up_pixel_buffer_dma_dev *buffer, int x1, int y1, int width, int height, unsigned short color, int backbuffer) {
    int x2 = x1 + width - 1;
    int y2 = y1 + height - 1;
    alt_up_pixel_buffer_dma_draw_box(buffer, x1, y1, x2, y2, color, backbuffer);
}

 void draw_player(alt_up_pixel_buffer_dma_dev *buffer, int x, int y, int w, int h, int direction, const int bitmap[]){
 	// Width 12, Height 16
	if (direction == RIGHT){
	 	for(int i = 0; i < h; i++){
	 		for (int j=0; j< w; j++){
	 	 		alt_up_pixel_buffer_dma_draw(buffer, bitmap[j+12*i], x+j, y+i);
	 		}
	 	}
	}
	else if (direction == LEFT){
	 	for(int i = 0; i < h; i++){
	 		for (int j=0; j< w; j++){
	 	 		alt_up_pixel_buffer_dma_draw(buffer, bitmap[j+12*i], x+(w-1)-j, y+i);
	 		}
	 	}
	}
 }

// Algorithm to check for collisions between two objects
int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
	// Returns 0 if no collision, otherwise returns 1
    if (x1 > (x2 + w2) || x2 > (x1 + w1)) {return 0;} // Check Sides
    else if ((y1+h1) < y2 || (y2+h2) < y1) {return 0;} // Check Top/Bottom
    else {return 1;}
}

// Draw Ladders
void draw_ladder(alt_up_pixel_buffer_dma_dev *pixel_buf_dma_dev, int x, int y, int w, int h, unsigned short color, int backbuffer) {
    // Vertical line for ladder
    alt_up_pixel_buffer_dma_draw_line(pixel_buf_dma_dev, x, y, x, y+h, color, backbuffer);
    // Horizontal rungs spaced by 10 pixels
    for (int i = y; i <= y+h; i += 10) {
        alt_up_pixel_buffer_dma_draw_line(pixel_buf_dma_dev, x - w, i, x + w, i, color, backbuffer);
    }
}

void draw_level(alt_up_pixel_buffer_dma_dev *pixel_buf_dma_dev, StaticObject*platforms, StaticObject*ladders, int backbuffer) {
    // Define angled platforms (top-left corner, width, height, slope)
    // Draw platforms
//  int num_platforms = sizeof(platforms) / sizeof(platforms[0]);

	// num_platforms doesn't seem to work with arrays/structs pointers
	// May have to pass a value in
	int num_platforms = 4;
    for (int i = 0; i < num_platforms; i++) {
        int x1 = platforms[i].x;
        int y1 = platforms[i].y;
        draw_block(pixel_buf_dma_dev, x1, y1, platforms[i].w, platforms[i].h, BURGUNDY, backbuffer);
    }

    // Draw ladders
//  int num_ladders = sizeof(ladders) / sizeof(ladders[0]);
    int num_ladders = 3;
    for (int i = 0; i < num_ladders; i++) {
        draw_ladder(pixel_buf_dma_dev, ladders[i].x, ladders[i].y, ladders[i].w, ladders[i].h, WHITE, backbuffer);
    }

}

// Draws and erase barrels
void draw_barrels(alt_up_pixel_buffer_dma_dev *pixbuf, DynamicObject*barrels, int num_barrels, int color, int backbuffer){
    for (int i = 0; i < num_barrels; i++) {
		alt_up_pixel_buffer_dma_draw_rectangle(pixbuf, barrels[i].x, barrels[i].y, barrels[i].x+barrels[i].w-1, barrels[i].y+barrels[i].h-1, color, backbuffer);
    	// draw_block(pixbuf, barrels[i].x, barrels[i].y, barrels[i].w, barrels[i].h, color, backbuffer);
    }
}
void erase_barrels(alt_up_pixel_buffer_dma_dev *pixbuf, DynamicObject*barrels, int num_barrels, int color, int backbuffer){
    for (int i = 0; i < num_barrels; i++) {
		alt_up_pixel_buffer_dma_draw_rectangle(pixbuf, barrels[i].xprev, barrels[i].yprev, barrels[i].xprev+barrels[i].w-1, barrels[i].yprev+barrels[i].h-1, color, backbuffer);
    }
}
// function to initialize barrels
void initialize_barrels(DynamicObject* barrels, int num_barrels, StaticObject* platforms, int num_platforms) {
   	int platform_index = 2;
    for(int i = 0; i < num_barrels; i++){
        // randomly cloose a platform
    	platform_index = rand()%num_platforms;
    	while (platform_index == 0){ // make sure it is not the bottom platform (platform[3] in this case)
    	     platform_index = rand()%num_platforms;
    	}
        // set barrels position based on the chosen platform
        barrels[i].y = platforms[platform_index].y - barrels[i].h;
        barrels[i].x = platforms[platform_index].x + rand()%280 + 20; // randomize x within platform width
        barrels[i].xvel = (rand() % 2 == 0) ? -barrels[i].xvel : barrels[i].xvel; // randomize initial direction
    }
}
//Function for converting to displayable hex number
alt_u8 hextosseg(unsigned int input){
	alt_u8 output;
	switch(input){
	   			case 0x00:
	   				output = ss_0;
	   				break;
	   			case 0x01:
	   				output = ss_1;
	   				break;
	   			case 0x02:
	   				output = ss_2;
	   			   	break;
				case 0x03:
					output = ss_3;
	   				break;
	   			case 0x04:
	   				output = ss_4;
	   				break;
	   			case 0x05:
	   				output = ss_5;
	   			   	break;
	   			case 0x06:
	   				output = ss_6;
	   			   	break;
	   			case 0x07:
	   				output = ss_7;
	   				break;
	   			case 0x08:
	   				output = ss_8;
	   			   	break;
	   			case 0x09:
	   				output = ss_9;
	   				break;
	   			case 0x0a:
	   				output = ss_a;
	   				break;
	   			case 0x0b:
	   				output = ss_b;
	   			   	break;
				case 0x0c:
					output = ss_c;
	   			   	break;
	   			case 0x0d:
	   				output = ss_d;
	   				break;
	   			case 0x0e:
	   				output = ss_e;
	   				break;
	   			case 0x0f:
	   				output = ss_f;
	   			   	break;
	   			default:
	   				output = ss_dark;
	   				break;
	   	   }
	return output;
}
int main(){
	int win_count = 0x0000;
	unsigned int hexa = 0x00;
	unsigned int hexb = 0x00;
	alt_u8 bottom_nibble;
	alt_u8 top_nibble;
	alt_u16 allnibs = 0xffff;
	srand(time(NULL));
	// Initialize
    printf("NIOS II DE10-Lite Donkey Kong-like Program\n");
    alt_up_pixel_buffer_dma_dev *pixbuf;
    alt_up_accelerometer_spi_dev *accbuf;
    alt_up_char_buffer_dev * charbuf;
    pixbuf = alt_up_pixel_buffer_dma_open_dev("/dev/video_pixel_buffer_dma_0");
    if (pixbuf == NULL) {
        printf("Error: Unable to open Pixel Buffer DMA device.\n");
        return -1;
    }
    printf("Opened Pixel Buffer DMA.\n");
    accbuf = alt_up_accelerometer_spi_open_dev("/dev/accelerometer_spi_0");
    if (accbuf == NULL) {
        printf("Error: Unable to open Accelerometer SPI device.\n");
        return -1;
    }
    // initialize character buffer DMA
    charbuf = alt_up_char_buffer_open_dev("/dev/video_character_buffer_with_dma_0");
    if(charbuf == NULL)
    	printf("Error: Could not open character buffer device ): \n");
    else
    	printf("Opened character buffer device! :D \n");

    //
    // Objects
    //
    // Note: Mario is 12x16. Updating this WILL update the draw_player, so mario will look jumbled
    DynamicObject player = {
    		20, 205, 12, 16, 0, 0, 180, 120, LEFT
    };

    // height must be multiple of 10
    StaticObject ladders[] = {
            {260, 170, 3, 70},  // Bottom right
            {60, 110, 3, 70}, // Ladder from second to third platform
            {260, 50, 3, 70},    // Ladder from third to top platform
        };
    DynamicObject barrels[] = {
    		{20, 20, 5, 20, 1.5, 0, 20, 20},
    		{20, 20, 15, 15, 2.5, 0, 20, 20},
			{20, 20, 30, 05, 3, 0, 20, 20},
    		{20, 20, 15, 15, 0.5, 0, 20, 20},
    		{20, 20, 15, 15, 2.2, 0, 20, 20},
//    		{20, 20, 5, 5, 10, 0, 20, 20},
    };
    StaticObject platforms[] = {
    		{0, 230, 320, 10}, // Spaced Levels
    		{0, 170, 260, 10},
    		{60, 110, 260, 10},
			{0, 50, 260, 10},
    };
	StaticObject win_box = {0, 0, 20, 20}; // green box at (0,0) with size 20x20


    //
    // Variables
    //

    bool touchingLadder = 0;
    bool collision;
    bool game_start = FALSE;
    alt_32 accelerometer_x;
	alt_32 accelerometer_y;
	int num_platforms = sizeof(platforms) / sizeof(platforms[0]);
	int num_ladders = sizeof(platforms) / sizeof(platforms[0]);
	int num_barrels = sizeof(barrels) / sizeof(barrels[0]);
	int current_ladder = 42;

	// Player Variables
	state state = IDLE; // Player State

	char splash_text0[40] = "Donkey Kong-Like";
	char splash_text1[40] = "Enable SW0 to begin";
	char splash_text2[10] = "YOU DIED";
	alt_u8 sw;

	hexa = win_count & 0x0f;
	hexb = win_count/16;

	bottom_nibble = hextosseg(hexa);
	top_nibble = hextosseg(hexb);

	allnibs = top_nibble << 8;
	allnibs = allnibs + bottom_nibble;
	IOWR_ALTERA_AVALON_PIO_DATA(HEX_PIO_BASE, allnibs);

while (1){
    alt_up_pixel_buffer_dma_clear_screen(pixbuf, 0);
    alt_up_pixel_buffer_dma_clear_screen(pixbuf, 1);

	//
	// Title Screen
	//
	alt_up_char_buffer_clear(charbuf);
    alt_up_char_buffer_string(charbuf, splash_text0, 29, 29);
    alt_up_char_buffer_string(charbuf, splash_text1, 29, 30);
    IOWR_ALTERA_AVALON_PIO_DATA(HEX_PIO_BASE, allnibs);
    // Reset Player - ensures that nothing weird happens after you reset
    state = IDLE;
    player = (DynamicObject){20, 205, 12, 16, 0, 0, 180, 120, LEFT}; // Wow This actually works!
    while(game_start == FALSE){
		sw = IORD_ALTERA_AVALON_PIO_DATA(SW_PIO_BASE);
		if (sw & 0x01){
			game_start = TRUE;
		}
    }


	alt_up_char_buffer_clear(charbuf);

	// Draw Level
	draw_level(pixbuf, platforms, ladders, 0);
	draw_level(pixbuf, platforms, ladders, 1);
	draw_block(pixbuf, win_box.x, win_box.y, win_box.w, win_box.h, 0x07E0, 1); // green color
	draw_block(pixbuf, win_box.x, win_box.y, win_box.w, win_box.h, 0x07E0, 0); // green color
    initialize_barrels(barrels, num_barrels, platforms, num_platforms);
    player.x = 20; player.y=205;

    printf("Starting Main Loop\n");
	while(game_start == TRUE){
		sw = IORD_ALTERA_AVALON_PIO_DATA(SW_PIO_BASE);
		//16 bit output to hex display
		IOWR_ALTERA_AVALON_PIO_DATA(HEX_PIO_BASE, allnibs);
		//
		// Erase Previous Positions + Redraw any objects
		//

		// Erase Previous Position on Pixel Buffer
		draw_block(pixbuf, player.xprev, player.yprev, player.w, player.h, BLACK, 1); // erase player
		erase_barrels(pixbuf, barrels, num_barrels, BLACK, 1);

		// Manually redraw ladders
    	draw_ladder(pixbuf, ladders[0].x, ladders[0].y, ladders[0].w, ladders[0].h, WHITE, 1);
    	draw_ladder(pixbuf, ladders[1].x, ladders[1].y, ladders[1].w, ladders[1].h, WHITE, 1);
    	draw_ladder(pixbuf, ladders[2].x, ladders[2].y, ladders[2].w, ladders[2].h, WHITE, 1);

		//
	    // Movement Physics - State Machine
		//

		// Accelerometer Check
	    alt_up_accelerometer_spi_read_x_axis(accbuf, &accelerometer_x);
	    alt_up_accelerometer_spi_read_y_axis(accbuf, &accelerometer_y);
		// Idle or Walking
		if (state == IDLE || state == WALKING){
			if (accelerometer_x > 30 && player.x > 5) {
		   		player.xvel = -HORIZONTAL_SPEED; // Move left
	   		}
	    	else if (accelerometer_x < -30 && player.x < 315) {
		    	player.xvel = HORIZONTAL_SPEED; // Move right
	    	}
	    	else{
	    		 player.xvel = player.xvel*X_GROUND_FRICTION; // More friction when you are on the ground
				state = IDLE;
	    	}
		    if (accelerometer_y < -30 && player.y > 5) {// Jump Input and Above Boundary
		    	player.yvel = -JUMP_SPEED;
		    	touchingLadder = 0;
		    	state = JUMPING;
	    	}
		}
		// Jumping
		else if (state == JUMPING){
			// Change Midair X velocity
			if (accelerometer_x > 30 && player.x > 5) {
	    		player.xvel -= AIR_HORIZONTAL_ACCEL; // Accelerate character for midair movement
	   		}
	    	else if (accelerometer_x < -30 && player.x < 315) {
	    		player.xvel += AIR_HORIZONTAL_ACCEL;
	    	}
			// Change Midair Y velocity
		    if (accelerometer_y < -30 && player.y > 5) {// Jump Input and Above Boundary
	        	player.yvel -= JUMP_ADJUSTMENT_ACCEL; // Allows for more control over jump height
	    	}
		    // Collision Check with Ladders
	    	for (int i=0; i<num_ladders; i++){
	    		touchingLadder = check_collision(player.x, player.y, player.w, player.h, ladders[i].x, ladders[i].y, ladders[i].w, ladders[i].h);
	    		if (touchingLadder == 1){
	    			state = CLIMBING;
	    			player.xvel = 0;
	    			player.yvel = 0;
	    			current_ladder = i;
	    			break;
	    		}
	    		else{
	    			state = JUMPING;
	    		}
	    	}
	    	touchingLadder = 0; // reset touchingLadder check
		}
		// Climbing
		else if(state == CLIMBING) {
			touchingLadder = 0;
    		if(accelerometer_y < -30)
        		player.yvel = -LADDER_HORIZONTAL_SPEED; // moving down
    		else if(accelerometer_y > 30)
    			player.yvel = LADDER_HORIZONTAL_SPEED;
    		else
        		player.yvel = 0; // stop climbing
    		// Break out of Climbing

	    	touchingLadder = check_collision(player.x, player.y, player.w, player.h, ladders[current_ladder].x, ladders[current_ladder].y, ladders[current_ladder].w, ladders[current_ladder].h);
	    	if (!touchingLadder){
	    		player.xvel = 0;
	    		player.yvel = 0;
	    		state = IDLE;
	    	}
		}

		//
	    // Direction Updates
		//

	    if (player.xvel > 0.2){
			player.dir = RIGHT;
	    }
	    else if (player.xvel < -0.2){
	    	player.dir = LEFT;
	    }

	    // Record Player Last Position
	    player.xprev = player.x;
	    player.yprev = player.y;

	    // Update Player Position
	    player.x += player.xvel;
	    player.y += player.yvel;

		// Acceleration Modifiers on Velocity for Next Iteration
	    player.xvel = player.xvel*X_FRICTION;
	    if (player.yvel < MAXIMUM_FALL_SPEED){
		    player.yvel = player.yvel + GRAVITY;
	    }
	//
	// Update Barrel Positions
	//
		for (int i=0; i<num_barrels; i++){
	    	barrels[i].xprev = barrels[i].x;
	    	barrels[i].yprev = barrels[i].y;
	    	if (barrels[i].x < 2 + abs(barrels[i].xvel) || barrels[i].x > 320){ // barrel MUST NOT travel past left side of screen or it breaks (right side of screenis fine)
		    	barrels[i].xvel = -barrels[i].xvel;
	    	}
	    	barrels[i].x += barrels[i].xvel;
		}

	//
	// Collision Response
	//

	    // Collision Check with Platforms
	    for (int i=0; i<num_platforms; i++){
	    	collision = check_collision(player.x, player.y, player.w, player.h, platforms[i].x, platforms[i].y, platforms[i].w, platforms[i].h);
		    if (collision == 1){
		    	// Simple Y Collision: Place player on top of colliding block
		    	player.y = platforms[i].y - player.h; // Place Player on top of Platform
		    	player.yvel = 0;
		    	state = IDLE; // Exit Jump Phase
		    }
	    }
		// Collision Check with Barrels
	    for (int i=0; i<num_barrels; i++){
	    	collision = check_collision(player.x, player.y, player.w, player.h, barrels[i].x, barrels[i].y, barrels[i].w, barrels[i].h);
		    if (collision == 1){

		    	//
		    	// DEATH SCREEN
		    	//
		    	alt_up_pixel_buffer_dma_clear_screen(pixbuf, 0);
		        alt_up_pixel_buffer_dma_clear_screen(pixbuf, 1);
		    	draw_block(pixbuf, 0, 0, 320, 240, 0xF800, 1);
		    	alt_up_char_buffer_clear(charbuf);
		       	alt_up_char_buffer_string(charbuf, splash_text2, 29, 29);

		       	// Swap buffer into red background
		        alt_up_pixel_buffer_dma_swap_buffers(pixbuf);
		        while (alt_up_pixel_buffer_dma_check_swap_buffers_status(pixbuf)){};

		        usleep(3000000);
		    	// reset game_start for next loop
		    	game_start = FALSE;
		    	alt_up_char_buffer_clear(charbuf);
		    	}
		    }

		if (player.x < win_box.w && player.y < win_box.h) {
			// Win Screen
			alt_up_pixel_buffer_dma_clear_screen(pixbuf, 0);
			alt_up_pixel_buffer_dma_clear_screen(pixbuf, 1);
			draw_block(pixbuf, 0, 0, 320, 240, 0x07E0, 1); // Green background
			alt_up_char_buffer_clear(charbuf);
			alt_up_char_buffer_string(charbuf, "YOU WIN!", 35, 29);
			win_count ++;


			// Swap buffer into green background
			alt_up_pixel_buffer_dma_swap_buffers(pixbuf);
			while (alt_up_pixel_buffer_dma_check_swap_buffers_status(pixbuf)) {};

			usleep(3000000);
			game_start = FALSE; // Reset game loop
			alt_up_char_buffer_clear(charbuf);
		}

	//
	// Draw Objects to Screen
	//
		draw_barrels(pixbuf, barrels, num_barrels, BROWN, 1);
		if (sw & 0b0001){
			draw_player(pixbuf, player.x, player.y, player.w, player.h, player.dir, player_bitmap);
		}
		if (sw & 0b100){
			draw_player(pixbuf, player.x, player.y, player.w, player.h, player.dir, player_mushroom_bitmap);
		}
		if (sw & 0b1000){
					draw_player(pixbuf, player.x, player.y, player.w, player.h, player.dir, player_kong_bitmap);
				}
//		Testing
//		draw_block(pixbuf, player.x, player.y, player.w, player.h, 0xF800, 1); // AABB Box

		// Buffer Update
	    alt_up_pixel_buffer_dma_swap_buffers(pixbuf);
	    while (alt_up_pixel_buffer_dma_check_swap_buffers_status(pixbuf)){};

		// Exit Game Loop
		if (sw & 0b10){

			//
			// DEATH SCREEN
			//
			alt_up_pixel_buffer_dma_clear_screen(pixbuf, 0);
		    alt_up_pixel_buffer_dma_clear_screen(pixbuf, 1);
			draw_block(pixbuf, 0, 0, 320, 240, 0xF800, 1);
			alt_up_char_buffer_clear(charbuf);
		   	alt_up_char_buffer_string(charbuf, splash_text2, 29, 29);

		   	// Swap buffer into red background
		    alt_up_pixel_buffer_dma_swap_buffers(pixbuf);
		    while (alt_up_pixel_buffer_dma_check_swap_buffers_status(pixbuf)){};

		    usleep(3000000);
			// reset game_start for next loop
			game_start = FALSE;
			alt_up_char_buffer_clear(charbuf);
			}
		}
	   hexa = win_count & 0x0f;
	   hexb = win_count/16;

	   bottom_nibble = hextosseg(hexa);
	   top_nibble = hextosseg(hexb);

	   allnibs = top_nibble << 8;
	   allnibs = allnibs + bottom_nibble;

	    //
	    // Framerate Control
	    //
	    usleep(10000);
	}
}
