#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "white_ship_small.h"
#include "asteroid0.h"
#include "asteroid1.h"
#include "asteroid2.h"
#include "asteroid3.h"
#include "start_screen.h"
#include "game_over.h"
#include "myLib.h"
#include "text.h"

int main() {
	//Initialize the gba and variables
	REG_DISPCNT = MODE_3 | BG2_EN;
	enum GBAState state = START;
	SHIP myShip;
	myShip.row = 70;
	myShip.col = 5;
	myShip.oldRow = myShip.row;
	myShip.oldCol = myShip.col;
	myShip.width = WHITE_SHIP_SMALL_WIDTH;
	myShip.height = WHITE_SHIP_SMALL_HEIGHT;
	myShip.oldWidth = myShip.width;
	myShip.oldHeight = myShip.height;
	myShip.speed = 1;
	myShip.image = white_ship_small;
	myShip.oldImage = myShip.image;
	ASTERIOD asteroids[10];
	ASTERIOD* currentRoid;
	unsigned int asteroidNum = 0;
	int transition = 0;
	unsigned int score = 0;
	char timer[] = "Time: 00:00:00";
	char oldTimer[] = "Time: 00:00:01";
	char destroyedRoidLabel[] = "Roids destroyed:";
	volatile unsigned short white = WHITE;
	volatile unsigned short black = BLACK;
	int fireTimer = 0;
	int destroyedRoidCount = 0;
	int oldDestroyedRoidCount = 1;
	//seed the random number generator with SCANLINECOUNTER
	srand(SCANLINECOUNTER);	
	while(1)  // The Game Loop
	{
		switch(state) {
			case START:
				//draw the start screen
				drawImage3(0, 0, START_SCREEN_WIDTH, START_SCREEN_HEIGHT, start_screen);	
				char space[] = "SPACE";
				drawString(57, 69, space, white);
				char explorers[] = "EXPLORERS!";
				drawString(75, 55, explorers, white);
				drawRect(145, 130, 15, 110, BLACK);
				char zToStart[] = "Press Z to Start";
				drawString(147, 132, zToStart, white);
				//transition to start_nodraw
				state = START_NODRAW;
				waitForVblank();
			case START_NODRAW:
				//if the key is clicked but not held down from finish
				if (KEY_DOWN_NOW(BUTTON_A)) {
					if (transition == 0) {
						state = PLAY_GAME;
						transition = 1;
						//initialize the game elements and display
						drawOverImage(0, 0, 240, 160);
						myShip.row = 70;
						myShip.col = 5;
						myShip.speed = 1;
						myShip.image = white_ship_small;
						asteroidNum = 0;
						destroyedRoidCount = 0;
						drawString(147, 5, oldTimer, white);
						char destroyedRoidString[4];
						sprintf(destroyedRoidString, "%i", oldDestroyedRoidCount);
						drawString(147, 230, destroyedRoidString, white);
						drawString(147, 115, destroyedRoidLabel, white);
						for (int i = 0; i < 10; i++) {
							initRoid(asteroids + i);
						}
						for (int i = 0; i < 5; i++) {
							(myShip.shotsFired + i)->isShot = 0;
						}
						break;
					}
				} else  {
					transition = 0;
				}
				break;
			case PLAY_GAME:
				//exit game if you press select
				if (KEY_DOWN_NOW(BUTTON_SELECT)) {
					state = START;
					break;
				}
				//randomly add asteroids. Add them more frequently the further you
				//are in the game
				if (((asteroidNum < ((score / 600) + 1)) && rand() % 101 == 0) 
					&& asteroidNum < 9) {
					int notDone = 1;
					for (int i = 0; i < 10 && notDone; i ++) {
						if (!asteroids[i].isShowing) {
							initRoid((asteroids + i));
							(asteroids + i)->isShowing = 1;
							(asteroids + i)->oldAsterRow = (asteroids + i)->asterRow;
							(asteroids + i)->oldAsterCol = (asteroids + i)->asterCol;
							(asteroids + i)->prevWidth = (asteroids + i)->imWidth;
							(asteroids + i)->prevHeight = (asteroids + i)->imHeight;
							(asteroids + i)->oldImage = (asteroids + i)->asterImage;
							notDone = 0;
						}
					}
					asteroidNum++;
				}
				//adjust the ship's position on arrow clicks/holds
				if (KEY_DOWN_NOW(BUTTON_RIGHT) && myShip.col < 205) {
					myShip.col = myShip.col + 1;
				}
				if (KEY_DOWN_NOW(BUTTON_LEFT) && myShip.col > 0) {
					myShip.col = myShip.col - 1;
				}
				if (KEY_DOWN_NOW(BUTTON_DOWN) && myShip.row < 120) {
					myShip.row = myShip.row + 1;
				}
				if (KEY_DOWN_NOW(BUTTON_UP) && myShip.row > 0) {
					myShip.row = myShip.row - 1;
				}
				//fire a shot when x is pressed
				if (KEY_DOWN_NOW(BUTTON_B)) {
					if (fireTimer == 0) {
						fire(myShip.row, myShip.col, &myShip);
						fireTimer++;					
					}
				}
				//only allow one shot to be fired per second assuming fpm = 60
				if (fireTimer > 0 && fireTimer < 60) {
					fireTimer++;
				} else {
					fireTimer = 0;
				}
				waitForVblank();
				//update asteroids for movement and collisions
				for (unsigned int i = 0; i < 10; i++) {
					currentRoid = asteroids + i;
					if (currentRoid->isShowing) {
						//draw over old roid
						drawRect(currentRoid->oldAsterRow, currentRoid->oldAsterCol, 
								currentRoid->prevWidth, currentRoid->prevHeight, black);
						//update roid position
						currentRoid->asterCol -= currentRoid->speed;
						if (currentRoid->asterCol > 0) {
							if (shipRoidCollision(myShip, *currentRoid)) {
								//there was a collision, end the game
								state = GAME_OVER;
							} else if (shotsRoidCollision(myShip.shotsFired, *currentRoid)) {
								//the roid was shot, increment score and don't redraw the roid
								initRoid(currentRoid);
								asteroidNum--;
								destroyedRoidCount++;
							} else {
								//no collision to draw the moved asteroid and update 'old' vars
								drawImage3(currentRoid->asterRow, currentRoid->asterCol, 
									currentRoid->imWidth, currentRoid->imHeight, currentRoid->asterImage);
								currentRoid->oldAsterRow = currentRoid->asterRow;
								currentRoid->oldAsterCol = currentRoid->asterCol;
								currentRoid->oldImage = currentRoid->asterImage;
								currentRoid->prevHeight = currentRoid->imHeight;
								currentRoid->prevWidth = currentRoid->imWidth;
							}
						} else {
							//send roid back to run the screen again at a new size/speed
							initRoid(currentRoid);
							currentRoid->isShowing = 1;
						}
					}
				}
				//draw the update ship and increment the score
				score++;
				drawShip(&myShip);
				//update the timer
				char scoreNum[2];
				int fpm = 60;
				int digit = ((score / fpm) % 10);
				sprintf(scoreNum, "%i", digit);
				timer[13] = *scoreNum;
				digit = ((score / fpm) % 100) / 10;
				sprintf(scoreNum, "%i", digit);
				timer[12] = *scoreNum;
				digit = ((score / fpm) % 1000) / 100;
				sprintf(scoreNum, "%i", digit);
				timer[10] = *scoreNum;
				digit = ((score / fpm) % 10000) / 1000;
				sprintf(scoreNum, "%i", digit);
				timer[9] = *scoreNum;
				digit = ((score / fpm) % 100000) /10000;
				sprintf(scoreNum, "%i", digit);
				timer[7] = *scoreNum;
				digit = ((score / fpm) % 1000000) /100000;
				sprintf(scoreNum, "%i", digit);
				timer[6] = *scoreNum;
				if (!compareArrays(oldTimer, timer, 14)) {
					waitForVblank();
					drawString(147, 5, oldTimer, black);
					strncpy(oldTimer, timer, 14);
					drawString(147, 5, timer, white);
				}
				//update the destroyed riod count
				if (destroyedRoidCount != oldDestroyedRoidCount) {
					char destroyedRoidString[4];
					sprintf(destroyedRoidString, "%i", oldDestroyedRoidCount);
					drawString(147, 230, destroyedRoidString, black);
					oldDestroyedRoidCount = destroyedRoidCount;
					sprintf(destroyedRoidString, "%i", destroyedRoidCount);
					drawString(147, 230, destroyedRoidString, white);
				}
				break;
			case GAME_OVER:
				//draw the game over pic and text with the score and destroyed roids text
				drawImage3(0, 0, START_SCREEN_WIDTH, START_SCREEN_HEIGHT, game_over);
				char zToRestart[] = "Press Z to return to the home screen";
				drawString(147, 5, zToRestart, white);
				char finalScoreString[] = "FINAL TIME:";
				drawString(100, 20, finalScoreString, white);
				char finalRoidString[] = "FINAL DESTROYED ROIDS:";
				drawString(100, 100, finalRoidString, white);
				char destroyedRoidString[4]; 
				sprintf(destroyedRoidString, "%i", destroyedRoidCount);
				drawString(120, 160, destroyedRoidString, white);
				drawString(120, 27, timer + 6, white);
				score = 0;
				//transition to game_over_nodraw to wait for button click
				state = GAME_OVER_NO_DRAW;
				break;
			case GAME_OVER_NO_DRAW:
				//go to start by pressing z or select
				if (KEY_DOWN_NOW(BUTTON_A)) {
					if (transition == 0) {
						state = START;
						transition = 1;
						break;
					}
				} else  {
					transition = 0;
				}
				if (KEY_DOWN_NOW(BUTTON_SELECT)) {
					state = START;
					break;
				}
				break;
		}				
	}
}