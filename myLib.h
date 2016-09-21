typedef unsigned short u16;
typedef unsigned int u32;
typedef struct {
	const volatile void *src;
	volatile void *dst;
	volatile u32 cnt;
} DMA_CONTROLLER;

#define REG_DISPCNT *(unsigned short*) 0x4000000
#define MODE_3 3
#define BG2_EN (1 << 10)
#define SCANLINECOUNTER *(volatile unsigned short *)0x4000006

#define COLOR(R,G,B) (((R) & 0x1F) | (((G) & 0x1F) << 5) | (((B) & 0x1F) << 10))
#define BLACK COLOR(0 , 0 , 0 )
#define GREEN COLOR(0, 31,0)
#define WHITE COLOR(31, 31, 31)

#define BUTTONS       *(volatile unsigned short*) 0x4000130
#define BUTTON_A      (1 << 0)
#define BUTTON_B      (1 << 1)
#define BUTTON_SELECT (1 << 3)
#define BUTTON_RIGHT  (1 << 4)
#define BUTTON_LEFT   (1 << 5)
#define BUTTON_UP     (1 << 6)
#define BUTTON_DOWN   (1 << 7)
#define KEY_DOWN_NOW(key) (~(BUTTONS) & key)

#define DMA ((volatile DMA_CONTROLLER*) 0x40000B0)
#define DMA_CHANNEL_3 3

#define DMA_DESTINATION_INCREMENT (0 << 21)
#define DMA_DESTINATION_DECREMENT (1 << 21)
#define DMA_DESTINATION_FIXED (2 << 21)
#define DMA_DESTINATION_RESET (3 << 21)

#define DMA_SOURCE_INCREMENT (0 << 23)
#define DMA_SOURCE_DECREMENT (1 << 23)
#define DMA_SOURCE_FIXED (2 << 23)

#define DMA_REPEAT (1 << 25)

#define DMA_16 (0 << 26)
#define DMA_32 (1 << 26)

#define DMA_NOW (0 << 28)
#define DMA_AT_VBLANK (1 << 28)
#define DMA_AT_HBLANK (2 << 28)
#define DMA_AT_REFRESH (3 << 28)

#define DMA_IRQ (1 << 30)
#define DMA_ON (1 << 31)
#define OFFSET(r, c, rowlen)  ((r)*(rowlen)+(c))
extern unsigned short *videoBuffer;

typedef struct 
{
	int asterRow;
	int asterCol;
	int oldAsterRow;
	int oldAsterCol;
	int imWidth;
	int imHeight;
	int prevWidth;
	int prevHeight;
	int speed;
	int isShowing;
	const unsigned short* asterImage;
	const unsigned short* oldImage;
} ASTERIOD;

typedef struct 
{
	int row;
	int col;
	int oldRow;
	int oldCol;
	int width;
	int height;
	int oldWidth;
	int oldHeight;
	int speed;
	int isShot;
} SHOT;

typedef struct 
{
	int row;
	int col;
	int oldRow;
	int oldCol;
	int width;
	int height;
	int oldWidth;
	int oldHeight;
	int speed;
	SHOT shotsFired[5];
	const unsigned short* image;
	const unsigned short* oldImage;
} SHIP;

enum GBAState {
	START,
	START_NODRAW,
	PLAY_GAME,
	GAME_OVER,
	GAME_OVER_NO_DRAW
};

void drawImage3(int r, int c, int width, int height, const unsigned short* image);

void drawOverImage(int r, int c, int width, int height);

void waitForVblank();

int collision(int width1, int height1, int row1, int col1, int width2, int height2, int row2, int col2);

int shipRoidCollision(SHIP ship, ASTERIOD roid);

// int roidRoidCollision(ASTERIOD roid1, ASTERIOD roid2);

void initRoid(ASTERIOD *asterPtr);

void setPixel(int row, int col, unsigned short color);

void drawRect(int row, int col, int height, int width, volatile unsigned short color);

int compareArrays(char a[], char b[], int n);

void fire(int row, int col, SHIP *ship);

void drawShip(SHIP *myShip);

int shotsRoidCollision(SHOT *shot, ASTERIOD roid);