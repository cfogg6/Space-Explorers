#include <stdlib.h>
#include <string.h>
#include "asteroid0.h"
#include "asteroid1.h"
#include "asteroid2.h"
#include "asteroid3.h"
#include "myLib.h"
#include <time.h>

unsigned short *videoBuffer  = (unsigned short *)0x6000000;

void drawImage3(int r, int c, int width, int height, const unsigned short* image)
{
	//draw the image by row
	for (int i = 0; i < height; i++) {
		DMA[DMA_CHANNEL_3].src = image + OFFSET(i, 0, width);
		DMA[DMA_CHANNEL_3].dst = videoBuffer + OFFSET(i + r, c, 240);
		DMA[DMA_CHANNEL_3].cnt = width | DMA_ON;
	}
}

void drawOverImage(int r, int c, int width, int height)
{
	//draws a black rectangle
	for (int i = 0; i < height; i++) {
		volatile unsigned short black = BLACK;
		DMA[3].src = &black;
		DMA[3].dst = &videoBuffer[OFFSET(r+i, c, 240)];
		DMA[3].cnt = width | DMA_ON | DMA_SOURCE_FIXED;
	}
}

void waitForVblank()
{
	while(SCANLINECOUNTER > 160);
	while(SCANLINECOUNTER < 160);
}

//detect a rectangular collision
int collision(int width1, int height1, int row1, int col1, int width2, int height2, int row2, int col2) {
	//if obj2 has atleast one horizontal edge within the horizontal range of obj1's edges
	if (((row1 > row2) && (row1 < (row2 + height2))) 
		|| (((row1 + height1) > row2) && ((row1 + height1) < (row2 + height2)))) {
		//if obj1 has atleast one vertical edge withing the vertical range of obj2's edges
		if (((col2 > col1) && ((col2 < col1 + width1)))
			|| (((col2 + width2) > col1) && ((col2 + width2) < (col1 + width1)))) {
			return 1;
		}
	}

	//if obj2 has atleast one horizontal edge within the horizontal range of obj1's edges
	if (((col1 > col2) && (col1 < (col2 + width2))) 
		|| (((col1 + width1) > col2) && ((col1 + width1) < (col2 + width2)))) {
		//if obj1 has atleast one vertical edge withing the vertical range of obj2's edges
		if (((row2 > row1) && ((row2 < row1 + height1)))
			|| (((row2 + height2) > row1) && ((row2 + height2) < (row1 + height1)))) {
			return 1;
		}
	}
	return 0;
}

int shipRoidCollision(SHIP ship, ASTERIOD roid) {
	//detect a collision between a roid and a ship, with 2 pixels of leniency
	return collision(ship.width - 8, ship.height - 2, ship.row + 2, ship.col + 2, 
		roid.imWidth - 2, roid.imHeight - 2, roid.asterRow + 2, roid.asterCol + 2);
}

int shotsRoidCollision(SHOT *shot, ASTERIOD roid) {
	//detect collision between a shot and a roid with 2 pixels of leniency
	for (int i = 0; i < 5; i++) {
		if (shot[i].isShot == 1) {
			if (collision(shot[i].width - 2, shot[i].height - 2, shot[i].row + 2, shot[i].col + 2, 
				roid.imWidth - 2, roid.imHeight - 2, roid.asterRow + 2, roid.asterCol + 2)) {
				//if there is a collision, return one and make the shot disappear
				//and stop drawing itself
				(shot + i)->isShot = 0;
				drawRect(shot->row, shot->col, shot->height, shot->width, BLACK);
				return 1;
			}
		}
	}
	return 0;
}

//Give roid a random size, position, and speed
//make roid not showing
void initRoid(ASTERIOD *asterPtr) {
	asterPtr->asterRow = 10 + (rand() % 100);
	asterPtr->asterCol = 210;
	asterPtr->speed = 1 + rand() % 3;
	int roidSize = rand() % 4;
	if (roidSize == 3) {
		asterPtr->imWidth = ASTEROID3_WIDTH;
		asterPtr->imHeight = ASTEROID3_HEIGHT;
		asterPtr->asterImage = asteroid3;
	} else if (roidSize == 2) {
		asterPtr->imWidth = ASTEROID2_WIDTH;
		asterPtr->imHeight = ASTEROID2_HEIGHT;
		asterPtr->asterImage = asteroid2;
	} else if (roidSize == 1) {
		asterPtr->imWidth = ASTEROID1_WIDTH;
		asterPtr->imHeight = ASTEROID1_HEIGHT;
		asterPtr->asterImage = asteroid1;
	} else {
		asterPtr->imWidth = ASTEROID0_WIDTH;
		asterPtr->imHeight = ASTEROID0_HEIGHT;
		asterPtr->asterImage = asteroid0;
	}
	asterPtr->isShowing = 0;
}

//set pixel, used to write text
void setPixel(int row, int col, unsigned short color)
{
	videoBuffer[OFFSET(row, col, 240)] = color;
}

//draw a rectangle of a given color
void drawRect(int row, int col, int height, int width, volatile unsigned short color)
{
	int r;
	for(r=0; r<height; r++)
	{
		DMA[3].src = &color;
		DMA[3].dst = &videoBuffer[OFFSET(row+r, col, 240)];
		DMA[3].cnt = width | DMA_ON | DMA_SOURCE_FIXED;
	}
}

//compare char arrays, used to see if score needs to be redrawn
int compareArrays(char a[], char b[], int n) {
  int ii;
  for(ii = 1; ii <= n; ii++) {
    if (a[ii] != b[ii]) return 0;
  }
  return 1;
}

//fire a shot by making initializing its vars and set isShot to true
//finds the next available shot in the ship's array of 5 shots, won't fire if all shots are fired
void fire(int row, int col, SHIP *myShip) {
	int done = 0;
	for (int i = 0; i < 5 && done == 0; i++) {
		if (myShip->shotsFired[i].isShot == 0) {
			(myShip->shotsFired + i)->row = row + (myShip->height/2) -2;
			(myShip->shotsFired + i)->col = col + (myShip->width) - 5;
			(myShip->shotsFired + i)->oldRow = row;
			(myShip->shotsFired + i)->oldCol = col;
			(myShip->shotsFired + i)->width = 10;
			(myShip->shotsFired + i)->height = 3;
			(myShip->shotsFired + i)->oldWidth = 10;
			(myShip->shotsFired + i)->oldHeight = 3;
			(myShip->shotsFired + i)->speed = 3;
			(myShip->shotsFired + i)->isShot = 1;
			done = 1;
		}
	}
}

//draw over the old ship, draw the new ship, update vars, and update all of the ship's shots
void drawShip(SHIP *myShip) {
	drawOverImage(myShip->oldRow, myShip->oldCol, myShip->width, myShip->height);
	myShip->oldRow = myShip->row;
	myShip->oldCol = myShip->col;
	drawImage3(myShip->row, myShip->col, myShip->width, myShip->height, myShip->image);
	//update shots
	for (int i = 0; i < 5; i++) {
		SHOT* shotPtr = myShip->shotsFired + i;
		if (shotPtr->isShot == 1 && shotPtr->col < 230) {
			drawRect(shotPtr->row, shotPtr->col, shotPtr->height, shotPtr->width, BLACK);
			shotPtr->oldCol = shotPtr->col;
			shotPtr->col = shotPtr->col + shotPtr->speed;
			drawRect(shotPtr->row, shotPtr->col, shotPtr->height, shotPtr->width, GREEN);
		} else if (shotPtr->isShot) {
			drawRect(shotPtr->row, shotPtr->col, shotPtr->height, shotPtr->width, BLACK);
			shotPtr->isShot = 0;
		}
	}
}