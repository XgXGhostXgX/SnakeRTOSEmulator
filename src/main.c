#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "levelDraw.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Font.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"

#include "AsyncIO.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 3

#define STATE_ONE 0
#define STATE_TWO 1
#define STATE_THREE 2

#define NEXT_TASK 0
#define PREV_TASK 1

#define STARTING_STATE STATE_ONE

#define STATE_DEBOUNCE_DELAY 300

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR
#define CAVE_SIZE_X SCREEN_WIDTH / 2
#define CAVE_SIZE_Y SCREEN_HEIGHT / 2
#define CAVE_X CAVE_SIZE_X / 2
#define CAVE_Y CAVE_SIZE_Y / 2
#define CAVE_THICKNESS 25
#define LOGO_FILENAME "../resources/menuButton.png"
#define LOGO_FILENAME2 "../resources/menuButtonGray.png"
#define HOME_FILENAME "../resources/homeButton.png"
#define HOME_FILENAME2 "../resources/homeButtonGray.png"
#define CAPS_LOCK "../resources/CapsLock.png"
#define CAPS_LOCK_HOVER "../resources/CapsLockHover.png"
#define BACKSPACE "../resources/BackSpace.png"
#define BACKSPACE_HOVER "../resources/BackSpaceHover.png"
#define UDP_BUFFER_SIZE 2000
#define UDP_TEST_PORT_1 1234
#define UDP_TEST_PORT_2 4321
#define MSG_QUEUE_BUFFER_SIZE 1000
#define MSG_QUEUE_MAX_MSG_COUNT 10
#define TCP_BUFFER_SIZE 2000
#define TCP_TEST_PORT 2222

#ifdef TRACE_FUNCTIONS
#include "tracer.h"
#endif

//Variables for RTOS States
int currentState=1;

//Variables enterPlayerName
int capsPressed=0;
int caps = 0;
int gameStarted = 0;

//Variables for Highscorescreen
int entered = 0;
int entered2 = 0;
int update = 0;
int pointButtonClicked=0;
int pointSort = 0;
char highscore[20];
int readHighscoreFile=1;
FILE *file;

//Variables for Pre Settings and Settings
int level=1;
int wallBorder=1;
int settingsClicked = 0;

//Divide and Conquer
int foodPointer=0;

//Variables for Time
clock_t start_t, start_t2, end_t, total_t;
int seconds =0;
int minutes=0;
int hours=0;
int startClock=0;

//Variable for Pause
int pausePressed=0;

//Variable for Infoscreen
bool infosPressed = true;

//Variable for SnakeGameScreen
bool initial=true;
int initialStop=1;
bool levelInitialized = false;

//Variables for graphics
int levelAnimate=0;
int showDevelopmentHUD=0;
int drawlevel=1;
int animateLevelDesign=1;
int animateLevel=0;

//Variables for the Foodelements
int food1X, food1Y, food2X, food2Y;
int randFood=1;
int twoFoodElements = 0;

//Variable for PickUpItem
int disableSpawn=0;

//Variables for the superfood
static int superFoodTimer = 0;
static int superFoodX = 0;
static int superFoodY = 0;
static bool superFoodCoverUpDrawn = false;

//Variables for FreezeItem
static bool freezeExists = false;
static int freezeX, freezeY;
static int playerOneFrozen = 0;
static int playerTwoFrozen = 0;

//Variables for ReduceItem
static int reduceExists = 0;
static int reduceX, reduceY;
static int reduceSnakeOne = 0;
static int reduceSnakeTwo = 0;

//Variables for InverseItem
static int inverseExists = 0;
static int inverseX, inverseY;
static int inverseControlOne = 0;
static int inverseControlTwo = 0;

//Variables of the players
char p1Direction;
char p1NextDirection;
char p2Direction;
char p2NextDirection;
static int p1Size=1;
static int p2Size=1;
static int p1X = 0;
static int p1Y = 0;
static int p2X = 0;
static int p2Y = 0;
char p1Name [10] = { 0 };
char p2Name [10] = { 0 };
int deleteP1Name=0;
int deleteP2Name=0;
int p1Selected = 1;
int p1Letter = 0;
int p2Letter = 0;
static bool p1Ready;
static bool p2Ready;

//Variables for PowerUps
static bool p1PowerUpButtonPressed = false;
static bool p2PowerUpButtonPressed = false;
//What PowerUp every Player has, Numbers same as in FieldArray
/*
 * 0 = Nothing
 * 12 = freeze
 * 13 = reduce
 * 14 = inverse
 */
static int p1PowerUp = 0;
static int p2PowerUp = 0;

static int winnerPlayer = 0;
static int frame = 0;
int frameTicks = 5;
static bool gameLost = false;
static bool multiplayer;
double cpu_time_used;

//temporary Variables for various uses
static int tempX;
static int tempY;
static int tempDirection;
static int tempNextDirection;
static int fieldValue;
int x,y;
//static clock_t start, end;

/*
 * Array for representation of the complete field
 * Variables:
 *   0 = Field is free
 *   1 = player one
 *   2 = player two
 *   3 = wall
 *  10 = normal food
 *  11 = superfood
 *  12 = freeze
 *  13 = reduce
 *  14 = inverse
 *  20 = teleport
 */
static int fieldArray[39][29];

//Variables for the KI
static int KIState = 0;
static int KITurnMem = 0;
static int turnx = 0;
static int turny = 0;

//Variables for the "+1" popup
static int plusSizeCounter;
static int plusSizeX;
static int plusSizeY;
static bool plusSizeWriteGreen;
static bool plusSizeWrite = false;
static int plusSizeIncrease;
static char plusSizeCharArr[100];


const unsigned char next_state_signal = NEXT_TASK;
const unsigned char prev_state_signal = PREV_TASK;

static TaskHandle_t StateMachine = NULL;
static TaskHandle_t BufferSwap = NULL;
static TaskHandle_t MainMenu = NULL;
static TaskHandle_t HighscoreScreen = NULL;
static TaskHandle_t GameScreen = NULL;

static QueueHandle_t StateQueue = NULL;
static SemaphoreHandle_t DrawSignal = NULL;
static SemaphoreHandle_t ScreenLock = NULL;

static image_handle_t logo_image = NULL;
static image_handle_t logo_image2 = NULL;
static image_handle_t home_image = NULL;
static image_handle_t home_image2 = NULL;
static image_handle_t capsLock_image = NULL;
static image_handle_t capsLock_hover_image= NULL;
static image_handle_t backSpace_image = NULL;
static image_handle_t backSpace_hover_image = NULL;


typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

typedef struct position {
	int x;
	int y;
}position;

//LinkedList
typedef struct node {
	struct node * head; // Zeiger auf erstes Element der Liste
	struct node * next; // Zeiger auf nächsten Knoten
	struct node * tail; // Zeiger auf letzes Element der Liste
	struct position * data; // Daten im node
}node;

//Struct Methods
node * createLinkedList(position * toStore);
node * addAsLast(node * list , position * toStore);
node * addAsLast2(node * list , position * toStore);
node * addAsFirst(node * list, position * toStore);
node * removeLast(node * list);
position * newPosition(int x, int y);
position * setPosition(position * pos, int newX, int newY);

/**
 * Method: Create a LinkedList
 * Parameter: toStore (Position) is the first Element of this LinkedList
 * Return: LinkedList (node)
 */
node * createLinkedList(position * toStore){
	node * head = NULL;
	head = (node*)malloc(sizeof(node));
	head -> data = toStore;
	head -> head = head;
	head -> tail = head;
	return head;
}

/**
 * Method: Add new Element at last position to Player1
 * Parameter: list (Node) is the list which new Element should be added
 * 				toStore (Position) is the new Element
 * Return: updated LinkedList (node)
 */
node * addAsLast(node * list, position * toStore){
	list = list -> head;
	node * temp = NULL;
	node * iterator = NULL;
	temp = (node*)malloc(sizeof(node));
	temp -> data = toStore;
	temp -> head = list;
	temp -> tail = temp;
	list -> tail = temp;
	iterator = list->next;
	int counter = 1;
	while (iterator -> next != NULL){
		iterator -> tail = temp;
		iterator = iterator -> next;
		counter++;
	}
	p1Size= counter;
	iterator->next = temp;
	iterator-> tail = temp;
	temp->next = NULL;
	return list;
}

/**
 * Method: Add new Element at last position to Player2
 * Parameter: list (Node) is the list which new Element should be added
 * 				toStore (Position) is the new Element
 * Return: updated LinkedList (node)
 */
node * addAsLast2(node * list, position * toStore){
	list = list -> head;
	node * temp = NULL;
	node * iterator = NULL;
	temp = (node*)malloc(sizeof(node));
	temp -> data = toStore;
	temp -> head = list;
	temp -> tail = temp;
	list -> tail = temp;
	iterator = list->next;
	int counter = 1;
	while (iterator -> next != NULL){
		iterator -> tail = temp;
		iterator = iterator -> next;
		counter++;
	}
	p2Size= counter;
	iterator->next = temp;
	iterator-> tail = temp;
	temp->next = NULL;
	return list;
}

/**
 * Method: Add new Element into first position
 * Parameter: list (Node) is the list which new Element should be added
 * 				toStore (Position) is the new Element
 * Return: updated LinkedList (node)
 */
node * addAsFirst(node * list, position * toStore){
	list = list -> head;
	node * temp = NULL;
	node * iterator = NULL;
	temp = (node*)malloc(sizeof(node));
	temp -> data = toStore;
	temp -> head = temp;
	temp -> next = list;
	temp -> tail = temp -> next -> tail;
	iterator = list;
	while (iterator -> next != NULL){
		iterator -> head = temp;
		iterator = iterator -> next;
	}
	iterator -> head = temp;
	return temp; 
}	

/**
 * Method: remove last Element from List
 * Parameter: list (Node) is the list which removed the last Element
 * Return: updated LinkedList (node)
 */
node * removeLast(node * list){
	list = list -> head;
	node * iterator = NULL;
	node * temp = NULL;
	iterator = list;
	while (iterator -> next -> next!= NULL){
		iterator = iterator -> next;
	}
	temp = iterator;
	iterator = list;
	temp -> next = NULL;
	while (iterator -> next != NULL){
		iterator -> tail = temp;
		iterator = iterator -> next;
	}
	iterator->tail=temp;
	return list;
}

/**
 * Method: Create new Position
 * Parameter: x is the x value, y is the y value
 * Return: Position (position)
 */
position * newPosition(int x, int y){
	position * pos = NULL;
	pos = (position*)malloc(sizeof(position));
	pos -> x = x;
	pos -> y = y;
	return pos;
}

/**
 * Method: Set old Position to new Position
 * Parameter: pos is the old Position which should be changed
 * 				 x is the x value, y is the y value
 * Return: updated Position (position)
 */
position * setPosition (position * pos, int newX, int newY){
	pos -> x = newX;
	pos -> y = newY;
	return pos;
}

//Functions that move the players
node * playerStep(node * snake, int player);
node * teleport(node * snake, int player);

//Funkcitons that return random free fields on the board
position * getRandomFreeField();
position * randomStart;

//Queues for the snakes
node * snake1;
node * snake2;

static buttons_buffer_t buttons = { 0 };

/**
 * Method: Checking if any Drawing Type is Null or empty, then throw [ERROR]
 * Parameter: status is if error is there, msg what error was it
 */
void checkDraw(unsigned char status, const char *msg)
{
    if (status) {
        if (msg){
            //Comment out, because Error Showing for player no Name entered
	    //fprintf(stderr, "[ERROR] %s, %s\n", msg,tumGetErrorMessage());
        }
        else {
	    //Comment out, because Error Showing for player no Name entered
            //fprintf(stderr, "[ERROR] %s\n", tumGetErrorMessage());
        }
    }

}
/*
 * Method: Changes the state, either forwards of backwards
 * Parameter: ? (something for the RTOS)
 */
void changeState(volatile unsigned char *state, unsigned char forwards)
{
    switch (forwards) {
        case NEXT_TASK:
            if (*state == STATE_COUNT - 1) {
                *state = 0;
            }
            else {
                (*state)++;
            }
            break;
        case PREV_TASK:
            if (*state == 0) {
                *state = STATE_COUNT - 1;
            }
            else {
                (*state)--;
            }
            break;
        default:
            break;
    }
}

/*
 * Method: Example basic state machine with sequential states
 * Parameter: ? (something for the RTOS)
 */
void basicSequentialStateMachine(void *pvParameters)
{
    unsigned char current_state = STARTING_STATE; // Default state
    unsigned char state_changed =
        1; // Only re-evaluate state if it has changed
    unsigned char input = 0;

    const int state_change_period = STATE_DEBOUNCE_DELAY;

    TickType_t last_change = xTaskGetTickCount();

    while (1) {
        if (state_changed) {
            goto initial_state;
        }

        // Handle state machine input
        if (StateQueue)
            if (xQueueReceive(StateQueue, &input, portMAX_DELAY) ==
                pdTRUE)
                if (xTaskGetTickCount() - last_change >
                    state_change_period) {
                    changeState(&current_state, input);
                    state_changed = 1;
                    last_change = xTaskGetTickCount();
                }

initial_state:
        // Handle current state
        if (state_changed) {
            switch (current_state) {
                case STATE_ONE:
                    if (GameScreen) {
                        vTaskSuspend(GameScreen);
                    }
                    if (HighscoreScreen) {
                    	vTaskSuspend(HighscoreScreen);
                    }
                    if (MainMenu) {
                        vTaskResume(MainMenu);
                    }
                    break;
                case STATE_TWO:
                    if (MainMenu) {
                        vTaskSuspend(MainMenu);
                    }
                    if (HighscoreScreen) {
                    	vTaskSuspend(HighscoreScreen);
                    }
                    if (GameScreen) {
                        vTaskResume(GameScreen);
                    }
                    break;
                case STATE_THREE:
                	if (MainMenu) {
                		vTaskSuspend(MainMenu);
                	}
                	if (HighscoreScreen) {
                		vTaskResume(HighscoreScreen);
                	}
                	if (GameScreen) {
                		vTaskSuspend(GameScreen);
                	}
                	break;
                default:
                    break;
            }
            state_changed = 0;
        }
    }
}

void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    tumDrawBindThread(); // Setup Rendering handle with correct GL context

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
            tumDrawUpdateScreen();
            tumEventFetchEvents();
            xSemaphoreGive(ScreenLock);
            xSemaphoreGive(DrawSignal);
            vTaskDelayUntil(&xLastWakeTime,
                            pdMS_TO_TICKS(frameratePeriod));
        }
    }
}

/**
 * Method: Checking during Loop for ButtonsInputs
 */
void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
    if(buttons.buttons[KEYCODE(F2)]==1){
    	buttons.buttons[KEYCODE(F2)]=0;
    	if(showDevelopmentHUD){
    		showDevelopmentHUD=0;
    	}
    	else{
    		showDevelopmentHUD=1;
    	}
    }
}

/**
 * Method: Drawing the Setting screen
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void drawSettings(int mousePosX, int mousePosY){
	static char str1[100] = { 0 };
	sprintf(str1,"Level1");
	//Level1 Button
	if(mousePosX >= 60 && mousePosX <= 250 && mousePosY >= 90 && mousePosY <= 210){
		if(level==1){
			checkDraw(tumDrawFilledBox(55,85,170,130,Red),__FUNCTION__);
		}
		checkDraw(tumDrawFilledBox(60,90,160,120,Grey),__FUNCTION__);
		checkDraw(tumDrawFilledBox(65,95,150,110,Black),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			level=1;
		}
	}
	else{
		if(level==1){
			checkDraw(tumDrawFilledBox(55,85,170,130,Red),__FUNCTION__);
		}
		checkDraw(tumDrawFilledBox(60,90,160,120,White),__FUNCTION__);
		checkDraw(tumDrawFilledBox(65,95,150,110,Black),__FUNCTION__);
	}

	sprintf(str1,"Level2");
	//LEVEL2 Button
	if(mousePosX >= 250 && mousePosX <= 410 && mousePosY >= 90 && mousePosY <= 210){
		if(level==2){
			checkDraw(tumDrawFilledBox(245,85,170,130,Red),__FUNCTION__);
		}
		checkDraw(tumDrawFilledBox(250,90,160,120,Grey),__FUNCTION__);
		checkDraw(tumDrawFilledBox(255,95,150,110,Black),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			level=2;
		}
	}
	else{
		if(level==2){
			checkDraw(tumDrawFilledBox(245,85,170,130,Red),__FUNCTION__);
		}
		checkDraw(tumDrawFilledBox(250,90,160,120,White),__FUNCTION__);
		checkDraw(tumDrawFilledBox(255,95,150,110,Black),__FUNCTION__);
	}

	sprintf(str1,"Level3");
	//Level3 Button
	if(mousePosX >= 440 && mousePosX <= 600 && mousePosY >= 90 && mousePosY <= 210){
		if(level==3){
			checkDraw(tumDrawFilledBox(435,85,170,130,Red),__FUNCTION__);
		}
		checkDraw(tumDrawFilledBox(440,90,160,120,Grey),__FUNCTION__);
		checkDraw(tumDrawFilledBox(445,95,150,110,Black),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			level=3;
		}
	}
	else{
		if(level==3){
			checkDraw(tumDrawFilledBox(435,85,170,130,Red),__FUNCTION__);
		}
		checkDraw(tumDrawFilledBox(440,90,160,120,White),__FUNCTION__);
		checkDraw(tumDrawFilledBox(445,95,150,110,Black),__FUNCTION__);
	}
	
	sprintf(str1,"Level4");
		//Level4 Button
		if(mousePosX >= 60 && mousePosX <= 250 && mousePosY >= 220 && mousePosY <= 340){
			if(level==4){
				checkDraw(tumDrawFilledBox(55,215,170,130,Red),__FUNCTION__);
			}
			checkDraw(tumDrawFilledBox(60,220,160,120,Grey),__FUNCTION__);
			checkDraw(tumDrawFilledBox(65,225,150,110,Black),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				level=4;
			}
		}
		else{
			if(level==4){
				checkDraw(tumDrawFilledBox(55,215,170,130,Red),__FUNCTION__);
			}
			checkDraw(tumDrawFilledBox(60,220,160,120,White),__FUNCTION__);
			checkDraw(tumDrawFilledBox(65,225,150,110,Black),__FUNCTION__);
		}

		sprintf(str1,"Level5");
		//LEVEL5 Button
		if(mousePosX >= 250 && mousePosX <= 410 && mousePosY >= 220 && mousePosY <= 340){
			if(level==5){
				checkDraw(tumDrawFilledBox(245,215,170,130,Red),__FUNCTION__);
			}
			checkDraw(tumDrawFilledBox(250,220,160,120,Grey),__FUNCTION__);
			checkDraw(tumDrawFilledBox(255,225,150,110,Black),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				level=5;
			}
		}
		else{
			if(level==5){
				checkDraw(tumDrawFilledBox(245,215,170,130,Red),__FUNCTION__);
			}
			checkDraw(tumDrawFilledBox(250,220,160,120,White),__FUNCTION__);
			checkDraw(tumDrawFilledBox(255,225,150,110,Black),__FUNCTION__);
		}

		sprintf(str1,"Level6");
		//Level6 Button
		if(mousePosX >= 440 && mousePosX <= 600 && mousePosY >= 220 && mousePosY <= 340){
			if(level==6){
				checkDraw(tumDrawFilledBox(435,215,170,130,Red),__FUNCTION__);
			}
			checkDraw(tumDrawFilledBox(440,220,160,120,Grey),__FUNCTION__);
			checkDraw(tumDrawFilledBox(445,225,150,110,Black),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				level=6;
			}
		}
		else{
			if(level==6){
				checkDraw(tumDrawFilledBox(435,215,170,130,Red),__FUNCTION__);
			}
			checkDraw(tumDrawFilledBox(440,220,160,120,White),__FUNCTION__);
			checkDraw(tumDrawFilledBox(445,225,150,110,Black),__FUNCTION__);
		}


	sprintf(str1,"Wall");
	//Wall Button
	if(mousePosX >= 225 && mousePosX <= 245 && mousePosY >= 380 && mousePosY <= 400){
		checkDraw(tumDrawFilledBox(225,380,20,20,Grey),__FUNCTION__);
		checkDraw(tumDrawFilledBox(230,385,10,10,Black),__FUNCTION__);
		checkDraw(tumDrawText(str1,180,380, Grey),__FUNCTION__);
		if(!wallBorder){
			checkDraw(tumDrawFilledBox(230,385,10,10,Red),__FUNCTION__);
		}
		if(tumEventGetMouseLeft()){
			wallBorder=0;
		}
	}
	else{
		checkDraw(tumDrawFilledBox(225,380,20,20,White),__FUNCTION__);
		checkDraw(tumDrawFilledBox(230,385,10,10,Black),__FUNCTION__);
		checkDraw(tumDrawText(str1,180,380, White),__FUNCTION__);
		if(!wallBorder){
			checkDraw(tumDrawFilledBox(230,385,10,10,Red),__FUNCTION__);
		}
	}

	sprintf(str1,"No Wall");
	//No Wall Button
	if(mousePosX >= 415 && mousePosX <= 435 && mousePosY >= 380 && mousePosY <= 400){
		checkDraw(tumDrawFilledBox(415,380,20,20,Grey),__FUNCTION__);
		checkDraw(tumDrawFilledBox(420,385,10,10,Black),__FUNCTION__);
		checkDraw(tumDrawText(str1,350,380, Grey),__FUNCTION__);
		if(wallBorder){
			checkDraw(tumDrawFilledBox(420,385,10,10,Red),__FUNCTION__);
		}
		if(tumEventGetMouseLeft()){
			wallBorder=1;
		}
	}
	else{
		checkDraw(tumDrawFilledBox(415,380,20,20,White),__FUNCTION__);
		checkDraw(tumDrawFilledBox(420,385,10,10,Black),__FUNCTION__);
		checkDraw(tumDrawText(str1,350,380, White),__FUNCTION__);
		if(wallBorder){
			checkDraw(tumDrawFilledBox(420,385,10,10,Red),__FUNCTION__);
		}
	}
	
	//LEVEL1
	level1(70,100,200,190,wallBorder);
	//LEVEL2
	level1(260,100,390,190,wallBorder);
	for(int x=260;x<=310;x+=5){
		checkDraw(tumDrawFilledBox(x,130,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,160,5,5,White), __FUNCTION__);
	}
	for(int x=340;x<=390;x+=5){
		checkDraw(tumDrawFilledBox(x,130,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,160,5,5,White), __FUNCTION__);
	}
	//LEVEL3
	level1(450,100,580,190,wallBorder);
	for(int x=480;x<=550;x+=5){
		checkDraw(tumDrawFilledBox(x,130,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,160,5,5,White), __FUNCTION__);
	}
	for(int x=450;x<=480;x+=5){
		checkDraw(tumDrawFilledBox(x,100,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,105,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,110,5,5,White), __FUNCTION__);

		checkDraw(tumDrawFilledBox(x,180,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,185,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,190,5,5,White), __FUNCTION__);
	}
	for(int x=550;x<=580;x+=5){
		checkDraw(tumDrawFilledBox(x,100,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,105,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,110,5,5,White), __FUNCTION__);

		checkDraw(tumDrawFilledBox(x,180,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,185,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,190,5,5,White), __FUNCTION__);
	}
	//LEVEL4
	level1(70,230,200,320,wallBorder);
	for(int x=100;x<=120;x+=5){
		checkDraw(tumDrawFilledBox(x,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,290,5,5,White), __FUNCTION__);
	}
	for(int x=150;x<=170;x+=5){
		checkDraw(tumDrawFilledBox(x,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(x,290,5,5,White), __FUNCTION__);
	}
	for(int y=250;y<=260;y+=5){
		checkDraw(tumDrawFilledBox(150,y,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(120,y,5,5,White), __FUNCTION__);
	}
	for(int y=290;y<=300;y+=5){
		checkDraw(tumDrawFilledBox(150,y,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(120,y,5,5,White), __FUNCTION__);
	}
	//LEVEL5
	level1(260,230,380,320,wallBorder);
	for(int y=230;y<=320;y+=5){
		checkDraw(tumDrawFilledBox(260,y,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(290,y,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(320,y,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(350,y,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(380,y,5,5,White), __FUNCTION__);
	}
	checkDraw(tumDrawFilledBox(275, 250, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(275, 300, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(305, 250, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(305, 300, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(335, 250, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(335, 300, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(365, 250, 5, 5, Purple), __FUNCTION__);
	checkDraw(tumDrawFilledBox(365, 300, 5, 5, Purple), __FUNCTION__);

	if(levelAnimate<=5){
		checkDraw(tumDrawFilledBox(275, 250, 5, 5, Yellow), __FUNCTION__);
	}
	if(5<levelAnimate && levelAnimate<=10){
		checkDraw(tumDrawFilledBox(305, 250, 5, 5, Yellow), __FUNCTION__);
	}
	if(10<levelAnimate && levelAnimate<15){
		checkDraw(tumDrawFilledBox(335, 250, 5, 5, Yellow), __FUNCTION__);
	}
	if(15<levelAnimate && levelAnimate<20){
		checkDraw(tumDrawFilledBox(365, 250, 5, 5, Yellow), __FUNCTION__);
	}
	if(35<levelAnimate && levelAnimate<=40){
		checkDraw(tumDrawFilledBox(365, 300, 5, 5, Yellow), __FUNCTION__);
	}
	if(40<levelAnimate && levelAnimate<=45){
		checkDraw(tumDrawFilledBox(335, 300, 5, 5, Yellow), __FUNCTION__);
	}
	if(45<levelAnimate && levelAnimate<50){
		checkDraw(tumDrawFilledBox(305, 300, 5, 5, Yellow), __FUNCTION__);
	}
	if(50<levelAnimate && levelAnimate<55){
		checkDraw(tumDrawFilledBox(275, 300, 5, 5, Yellow), __FUNCTION__);
	}


	//LEVEL6
	level1(450,230,580,320,wallBorder);
	if(levelAnimate<=25){
		checkDraw(tumDrawFilledBox(460,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(460,280,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(470,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(480,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,310,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(500,280,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(520,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(520,290,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(530,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(530,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(540,280,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(560,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(560,290,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,310,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,280,5,5,White), __FUNCTION__);
	}
	if(25<levelAnimate && levelAnimate<=50){
		checkDraw(tumDrawFilledBox(460,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(460,310,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(470,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(480,290,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,280,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(500,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(520,310,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(520,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(530,270,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(530,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(540,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(560,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(560,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,290,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,250,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,250,5,5,White), __FUNCTION__);
	}
	if(50<levelAnimate){
		checkDraw(tumDrawFilledBox(460,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(460,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(470,310,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(480,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,270,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,290,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(490,270,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(500,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(520,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(520,260,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(530,300,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(530,280,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(540,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(560,280,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(560,310,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,270,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,240,5,5,White), __FUNCTION__);
		checkDraw(tumDrawFilledBox(570,300,5,5,White), __FUNCTION__);
	}
	if(levelAnimate==75){
		levelAnimate=0;
	}
	levelAnimate++;



	//Print LevelText on Field
	if(level==1){
		sprintf(str1,"Level1");
		checkDraw(tumDrawText(str1,120,140, Red),__FUNCTION__);
	}
	if(level==2){
		sprintf(str1,"Level2");
		checkDraw(tumDrawText(str1,310,140, Red),__FUNCTION__);
	}
	if(level==3){
		sprintf(str1,"Level3");
		checkDraw(tumDrawText(str1,500,140, Red),__FUNCTION__);
	}
	if(level==4){
		sprintf(str1,"Level4");
		checkDraw(tumDrawText(str1,120,270, Red),__FUNCTION__);
	}
	if(level==5){
		sprintf(str1,"Level5");
		checkDraw(tumDrawText(str1,310,270, Red),__FUNCTION__);
	}
	if(level==6){
		sprintf(str1,"Level6");
		checkDraw(tumDrawText(str1,500,270, Red),__FUNCTION__);
	}

}

/**
 * Method: Drawing Logo (house or gear symbol)
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void vDrawLogo(int mousePosX, int mousePosY){
	static int image_width;
	if(!settingsClicked){
		if((image_width = tumDrawGetLoadedImageWidth(logo_image)) != -1 && tumDrawGetLoadedImageWidth(logo_image2) != -1){
			if(mousePosX >= SCREEN_WIDTH-10-image_width && mousePosX <= SCREEN_WIDTH-10 && mousePosY >= 10 && mousePosY <= 60){
				checkDraw(tumDrawLoadedImage(logo_image2,SCREEN_WIDTH-10-image_width, 10),__FUNCTION__);
				if(tumEventGetMouseLeft()){
					settingsClicked=1;
				}
			}
			else{
				checkDraw(tumDrawLoadedImage(logo_image,SCREEN_WIDTH-10-image_width, 10),__FUNCTION__);
			}
		}
	}
	else{
		if((image_width = tumDrawGetLoadedImageWidth(home_image)) != -1 && tumDrawGetLoadedImageWidth(home_image2) != -1){
			if(mousePosX >= 10 && mousePosX <= 60 && mousePosY >= 10 && mousePosY <= 60){
				checkDraw(tumDrawLoadedImage(home_image2, 10, 10),__FUNCTION__);
				drawSettings(mousePosX, mousePosY);
				if(tumEventGetMouseLeft()){
					settingsClicked=0;
				}
			}
			else{
				checkDraw(tumDrawLoadedImage(home_image, 10 , 10),__FUNCTION__);
				drawSettings(mousePosX, mousePosY);
			}
		}
		else{
			printf("LOAD PIC FAILED!!");
		}
	}
}

/**
 * Method: Draw Development HUD, activate during Game via F2
 */
void vDrawButtonText(void)
{
    static char str[100] = { 0 };

    sprintf(str, "Axis 1: %5d | Axis 2: %5d", p1X, p1Y);

    checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 0.5, White),
              __FUNCTION__);

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        sprintf(str, "W: %d | S: %d | A: %d | D: %d",
                buttons.buttons[KEYCODE(W)],
                buttons.buttons[KEYCODE(S)],
                buttons.buttons[KEYCODE(A)],
                buttons.buttons[KEYCODE(D)]);
        xSemaphoreGive(buttons.lock);
        checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 2, White),
                  __FUNCTION__);
    }

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        sprintf(str, "UP: %d | DOWN: %d | LEFT: %d | RIGHT: %d",
                buttons.buttons[KEYCODE(UP)],
                buttons.buttons[KEYCODE(DOWN)],
                buttons.buttons[KEYCODE(LEFT)],
                buttons.buttons[KEYCODE(RIGHT)]);
        xSemaphoreGive(buttons.lock);
        checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 3.5, White),
                  __FUNCTION__);
    }
}
/**
 * Method: Checking StateChange pressed and change
 * Return: 0 stateChanged no Problems
 * 			-1 stateChanged had Problems
 */
static int vCheckStateInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
    	if(currentState==1){
    		currentState=0;
            if (StateQueue) {
                xSemaphoreGive(buttons.lock);
                xQueueSend(StateQueue, &next_state_signal, 0);
                return -1;
            }
        }
    	if(currentState==-1){
    		currentState=0;
        	if (StateQueue) {
        		xSemaphoreGive(buttons.lock);
        		xQueueSend(StateQueue, &prev_state_signal,0);
        		return -1;
        	}
        }

        xSemaphoreGive(buttons.lock);
    }

    return 0;
}

/**
 * Method: Refreshing screen
 */
void refresh()
{
	//clear Color Array only for divide and Conquer
		//colorArray();
	//black background
	checkDraw(tumDrawClear(Black), __FUNCTION__);
	//HUD objects top left
	static char str[100] = { 0 };
	if(showDevelopmentHUD){
		vDrawButtonText();
		sprintf(str,"X: %d, Y: %d",food1X, food1Y);
		checkDraw(tumDrawText(str,170, DEFAULT_FONT_SIZE * 2, White),__FUNCTION__);
	}
	//HUD Size of snake1
	sprintf(str,"%s size: %d",p1Name,p1Size);
	checkDraw(tumDrawText(str,100,75, White),__FUNCTION__);
	//HUD Size of snake2
	if(multiplayer){
		static char str2[100] = { 0 };
		sprintf(str2,"%s size: %d",p2Name,p2Size);
		checkDraw(tumDrawText(str2,400,75, White),__FUNCTION__);
	}
	//drawWall
	if(!drawlevel){
		parameterReset();
		if(level==1){
			level1(100,100,500,400,wallBorder);
		}
		if(level==2){
			level2(100,100,500,400,wallBorder);
		}
		if(level==3){
			level3(100,100,500,400,wallBorder);
		}
		if(level==4){
			level4(100,100,500,400,wallBorder);
		}
		if(level==5){
			level5(100,100,500,400,wallBorder);
		}
		if(level==6){
			level6(100,100,500,400,wallBorder,fieldArray);
		}
	}
	//Start Timer after first Movement of snake1 and show on HUD
	if(startClock){
			seconds = (clock()-start_t)/CLOCKS_PER_SEC;
			minutes = seconds/60;
			hours = minutes/60;
	}
	sprintf(str,"%.2d:%.2d:%.2d",hours,minutes%60,seconds%60);
	checkDraw(tumDrawText(str,260,15, White),__FUNCTION__);
}


//OLD CODE FOODSPAWN Randome/Divide and Conquer
/**
 * Methode: Divide and Conquer return possible value of FoodElement
 * Parameter: snake1, check from position (x1,y1) until (x2,y2)
 * Return: Possible Possition for foodElement
int checkPosition(node* snake1,int x1, int y1,int x2, int y2){
	if(x2==x1+10 && y2==y1+10){
		if(checkColor(x1,y1,Black)){
			return 1;
		}
		return 0;
	}
	if(x1+20==x2 && y1+10==y2){
		return checkPosition(snake1,x1,y1,x2-10,y2) + checkPosition(snake1,x1+10,y1,x2,y2);
	}
	if(x1+30==x2 && y1+10==y2){
		return checkPosition(snake1,x1,y1,x2-20,y2)+checkPosition(snake1,x1+10,y1,x2-10,y2)+checkPosition(snake1,x1+20,y1,x2,y2);
	}
	else{
		return checkPosition(snake1,x1,y1,((x2-x1)/20)*10+x1,((y2-y1)/20)*10+y1)+checkPosition(snake1,((x2-x1)/20)*10+x1,y1,x2,((y2-y1)/20)*10+y1)+checkPosition(snake1,x1,((y2-y1)/20)*10+y1,((x2-x1)/20)*10+x1,y2)+checkPosition(snake1,((x2-x1)/20)*10+x1,((y2-y1)/20)*10+y1,x2,y2);
	}
}
 * Methode: Divide and Conquer return array with possible Position
 * Parameter: snake1, check from position (x1,y1) until (x2,y2)
void check(node* snake1,int x1, int y1,int x2, int y2, position * array[]){
	if(x2==x1+10 && y2==y1+10){
		if(checkColor(x1,y1,Black)){
			position * p = newPosition(x1,y1);
			array[foodPointer]=p;
			foodPointer++;
		}
		return;
	}
	if(x1+20==x2 && y1+10==y2){
		check(snake1,x1,y1,x2-10,y2,array);
		check(snake1,x1+10,y1,x2,y2,array);
		return;
	}
	if(x1+30==x2 && y1+10==y2){
		check(snake1,x1,y1,x2-20,y2,array);
		check(snake1,x1+10,y1,x2-10,y2,array);
		check(snake1,x1+20,y1,x2,y2,array);
		return;
	}
	else{
		check(snake1,x1,y1,((x2-x1)/20)*10+x1,((y2-y1)/20)*10+y1,array);
		check(snake1,((x2-x1)/20)*10+x1,y1,x2,((y2-y1)/20)*10+y1,array);
		check(snake1,x1,((y2-y1)/20)*10+y1,((x2-x1)/20)*10+x1,y2,array);
		check(snake1,((x2-x1)/20)*10+x1,((y2-y1)/20)*10+y1,x2,y2,array);
		return;
	}
}
 * Method: Place FoodElement on Randome Position
 * Parameter: snake1 is the List from all Position of the snake1
 *
void randomeFood(node* snake1, node* snake2){
	//Spawn Foodelement by Random Value within the Wall
	int zahl=0;
    bool divAndConqu = false;
	if(randFood && divAndConqu){
        start = clock();
		int n=checkPosition(snake1,110,110,490,390);
		srand(time(0));
		zahl = rand()%n;
		position * array[n];
		check(snake1,110,110,490,390,array);
		foodPointer=0;
		if(firstElement){
			x=array[zahl]->x;
			y=array[zahl]->y;
		}
		if(twoFoodElements){
			//Multiplayer create another Foodelement
			if(multiplayer){
				zahl = rand()%n;
				xx=array[zahl]->x;
				yy=array[zahl]->y;
			}
		}
		randFood=0;
        end = clock();
        cpu_time_used = ((double) (end-start)) / CLOCKS_PER_SEC;
        printf("Time used for randomeFood with divide and conquer = %f \n" , cpu_time_used);
	}
	else if(randFood && !divAndConqu){
        start = clock();
        
        //x 110 - 490, y 110 - 390
        
        position * freeField = getRandomFreeField();
        
        x = freeField -> x * 10 + 110;
        y = freeField -> y * 10 + 110;
        printf("freeField x = %d", freeField -> x);
        
        if(twoFoodElements){
			//Multiplayer create another Foodelement
			if(multiplayer){
				position * freeField = getRandomFreeField();
        
                xx = freeField -> x * 10 + 110;
                yy = freeField -> y * 10 + 110;
			}
		}

		randFood=0;
        
        end = clock();
        cpu_time_used = ((double) (end-start)) / CLOCKS_PER_SEC;
        printf("Size of Snake = %d , Time used for randomeFood with random search = %f \n" ,p1Size, cpu_time_used);
    }
	//singlePlayer
	if(!multiplayer){
		//If Player not hit the FoodElement draw the FoodElement
		if(!(p1X==x && p1Y==y)){
			checkDraw(tumDrawFilledBox(x,y,5,5,Green), __FUNCTION__);
		}
		//Else Player hit the FoodElement, size increment, enable Spawn Foodelement, create new Foodelement
		else{
			//Better Add when frame get refreshed, looks cleaner
			if(frame%frameTicks==0){
				position * newPiece = newPosition(snake1->tail->data->x,snake1->tail->data->y);
				snake1 = addAsLast(snake1,newPiece);
			}
			randFood=1;
            writePlusSize(1);
			foodchecker = 1;
		}
	}
	//multiplayer
	else{
		if(!(p1X==x && p1Y==y) && !(p2X==x && p2Y==y)){
			checkDraw(tumDrawFilledBox(x,y,5,5,Green), __FUNCTION__);
		}
		//Else Player hit the FoodElement, size increment, enable Spawn Foodelement, create new Foodelement
		else{
			if(p1X==x && p1Y==y && frame%frameTicks==0){
				position * newPiece = newPosition(snake1->tail->data->x,snake1->tail->data->y);
			    snake1 = addAsLast(snake1,newPiece);
			}
			if(p2X==x && p2Y==y && frame%frameTicks==0){
				position * newPiece = newPosition(snake2->tail->data->x,snake2->tail->data->y);
			    snake2 = addAsLast2(snake2,newPiece);
			}
			if(p1X==xx && p1Y==yy && frame%frameTicks==0){
				position * newPiece = newPosition(snake1->tail->data->x,snake1->tail->data->y);
				snake1 = addAsLast(snake1,newPiece);
			}
			if(p2X==xx && p2Y==yy && frame%frameTicks==0){
				position * newPiece = newPosition(snake2->tail->data->x,snake2->tail->data->y);
				snake2 = addAsLast2(snake2,newPiece);
			}
			firstElement=1;
			twoFoodElements=0;
            writePlusSize(1);
			randFood=1;
			foodchecker = 1;
		}
		if(!(p1X==xx && p1Y==yy) && !(p2X==xx && p2Y==yy)){
			checkDraw(tumDrawFilledBox(xx,yy,5,5,Green), __FUNCTION__);
		}
		//Else Player hit the FoodElement, size increment, enable Spawn Foodelement, create new Foodelement
		else{
			if(p1X==xx && p1Y==yy && frame%frameTicks==0){
				position * newPiece = newPosition(snake1->tail->data->x,snake1->tail->data->y);
				snake1 = addAsLast(snake1,newPiece);
			}
			if(p2X==xx && p2Y==yy && frame%frameTicks==0){
				position * newPiece = newPosition(snake2->tail->data->x,snake2->tail->data->y);
				snake2 = addAsLast2(snake2,newPiece);
			}
			firstElement=0;
			twoFoodElements=1;
			randFood=1;
            writePlusSize(1);
			foodchecker = 1;
		}
	}
}
*/

/**
 * Method: Write a note on snake if size increase or decrease
 * Parameter: increase is the value which should be shown on the screen
 * 				player is for which player should be draw the writePlusSize
 */
void writePlusSize(int increase, int player) {
    plusSizeCounter = 0;
    if(player == 1) {
        plusSizeX = p1X;
        plusSizeY = p1Y;
    }
    else {
        plusSizeX = p2X;
        plusSizeY = p2Y;
    }
    plusSizeWrite = true;
    plusSizeIncrease = increase;
    if(increase>0) {
        sprintf(plusSizeCharArr, "+%d", increase);
        plusSizeWriteGreen = true;
    }
    else {
        sprintf(plusSizeCharArr, "%d", increase);
        plusSizeWriteGreen = false;
    }
}


/**
 * Method: Show Infoscreen when Button 'I' is pressed
 */
void infoscreen() {
    //check if I is not pressed
	if(!infosPressed){
		if(buttons.buttons[KEYCODE(I)]){
            infosPressed = true;
			buttons.buttons[KEYCODE(I)]=0;
		}
	}
	//check if P is pressed
	if(infosPressed){
		//Check Button P is pressed
		if(buttons.buttons[KEYCODE(I)]){
			infosPressed = false;
			buttons.buttons[KEYCODE(I)]=0;
		}
	}
}

/**
 * Method: Pause game if Button 'P' is pressed
 */
void pause(){
	//check if P is not pressed
	if(!pausePressed){
		if(buttons.buttons[KEYCODE(P)]){
			//P pressed, catch clock at stopped time, set P pressed, button P is not activate
			start_t2 = clock();
            if(!gameLost) {
                pausePressed = 1;
            }
			buttons.buttons[KEYCODE(P)]=0;
		}
	}
	//check if P is pressed
	if(pausePressed && !gameLost){
		//Create Pause Element on Screen
		static char str[100] = { 0 };
		sprintf(str,"PAUSE");
		checkDraw(tumDrawText(str,250,75, Red),__FUNCTION__);

		//Check Button P is pressed
		if(buttons.buttons[KEYCODE(P)]){
			//catch the new starting Point, shift start time of start_t, et P  not pressed, button P is not activate
			end_t = clock();
			start_t += (end_t-start_t2);
			pausePressed = 0;
			buttons.buttons[KEYCODE(P)]=0;
		}
	}
}


/**
 * Method: snake1 plays automatic
 * Was created to test size, Spawn of Foodelement
 */
void KI(){
	//check if Foodelement is right of snake1, then move right with D
	if(p1X < x){
		buttons.buttons[KEYCODE(W)]=0;
		buttons.buttons[KEYCODE(A)]=0;
		buttons.buttons[KEYCODE(S)]=0;
		buttons.buttons[KEYCODE(D)]=1;
	}
	//check if Foodelement is above the snake1, then move up with W
	if(p1Y < y){
		buttons.buttons[KEYCODE(A)]=0;
		buttons.buttons[KEYCODE(S)]=0;
		buttons.buttons[KEYCODE(D)]=0;
		buttons.buttons[KEYCODE(W)]=1;
	}
	//check if Foodelement is left of snake1, then move left with A
	if(p1X > x){
		buttons.buttons[KEYCODE(W)]=0;
		buttons.buttons[KEYCODE(S)]=0;
		buttons.buttons[KEYCODE(D)]=0;
		buttons.buttons[KEYCODE(A)]=1;
	}
	//check if Foodelement is below the snake1, then move down with S
	if(p1Y > y){
		buttons.buttons[KEYCODE(W)]=0;
		buttons.buttons[KEYCODE(A)]=0;
		buttons.buttons[KEYCODE(D)]=0;
		buttons.buttons[KEYCODE(S)]=1;
	}
}



/**
 * Method: snake1 plays automatic
 * Highscore: 53
 */
void KILong(){
    if(initial) {
        KIState = 3;
    }
    if(KIState == 0) {
        if(p1X == x){
            
            int dist = y - p1Y;
            
            if(dist>140){            
                buttons.buttons[KEYCODE(W)]=0;
                buttons.buttons[KEYCODE(A)]=0;
                buttons.buttons[KEYCODE(S)]=1;
                buttons.buttons[KEYCODE(D)]=0;
                KIState = 10;
            }
            else {
                buttons.buttons[KEYCODE(W)]=1;
                buttons.buttons[KEYCODE(A)]=0;
                buttons.buttons[KEYCODE(S)]=0;
                buttons.buttons[KEYCODE(D)]=0;
                KIState = 11;
            }
        }
        else {
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
        }
    }
    else if(KIState == 10) {
        if(p1Y == y){
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
            KIState = 2;
            KITurnMem = p1X;
        }
        else {
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=1;
            buttons.buttons[KEYCODE(D)]=0;
        }
    }
    else if(KIState == 11) {
        if(p1Y == y){
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
            KIState = 2;
            KITurnMem = p1X;
        }
        else {
            buttons.buttons[KEYCODE(W)]=1;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=0;
        }
    }
    else if(KIState == 2) {
        if(p1X == KITurnMem + 10 || p1X == KITurnMem - 380){
            buttons.buttons[KEYCODE(W)]=1;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=0;
            KIState = 3;
        }
    }
    else if(KIState == 3) {
        if(p1Y == 110){
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
            KIState = 0;
        }
        else {
            buttons.buttons[KEYCODE(W)]=1;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=0;
        }
    }
    
}

/**
 * Method: snake1 plays automatic
 * Will play forever / until complete field is full, but is very slow at playing
 * For longterm stresstest
 */
void KISuperLong(){
    if(initial) {
        frameTicks = 1;
        turnx = 490;
        turny = 110;
        KIState = 0;
    }
    if(KIState == 0) {
        if(p1Y == 110){
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
            KIState = 1;
        }
        else {
            buttons.buttons[KEYCODE(W)]=1;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=0;
        }
    }
    else if(KIState == 1) {
        if(p1X == turnx && p1Y == turny){
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=1;
            buttons.buttons[KEYCODE(D)]=0;
            KIState = 2;
            turnx = turnx - 10;
            turny = turny + 10;
            KITurnMem = p1Y;
            
            if(turnx==100){
                turnx=490;
            }
            if(turny==400){
                turny=110;
            }
        }
        else {
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
        }
    }
    else if(KIState == 2) {
        if(p1Y == KITurnMem + 10 || p1Y == KITurnMem - 280){
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=0;
            buttons.buttons[KEYCODE(D)]=1;
            KIState = 1;
        }
        else {
            buttons.buttons[KEYCODE(W)]=0;
            buttons.buttons[KEYCODE(A)]=0;
            buttons.buttons[KEYCODE(S)]=1;
            buttons.buttons[KEYCODE(D)]=0;
        }
    }
}

/**
 * Method: Useful for fieldarray to convert Array index to Coordinates
 * Parameter: arrayPos is the index which should be converted
 * Return: Converted value
 */
int arrayPosToCoords(int arrayPos) {
    return (arrayPos + 11) * 10;
}

/**
 * Method: Useful for fieldarray to convert Coordinate to Array index
 * Parameter: Coordinate which should be converted into Array index
 * Return: Converted value
 */
int coordsToArrayPos(int coords) {
    return (coords / 10) - 11;
}

/**
 * Method: Drawing and creating PowerUps
 */
void powerUps() {
    if(!disableSpawn){
        
        //inverse Control
        if(!inverseExists && !initial && multiplayer){
			if(rand()%50 == 2) {
				inverseExists = 1;
				position * freeField = getRandomFreeField();
				inverseX = arrayPosToCoords(freeField -> x);
				inverseY = arrayPosToCoords(freeField -> y);
				fieldArray[freeField -> x] [freeField -> y] = 14;
			}
		}
        //Reduce
        if(!reduceExists && !initial && multiplayer){
			if(rand() % 50 == 2) {
				reduceExists = 1;
				position * freeField = getRandomFreeField();
				reduceX = arrayPosToCoords(freeField -> x);
				reduceY = arrayPosToCoords(freeField -> y);
				fieldArray[freeField -> x] [freeField -> y] = 13;
			}
		}
        
        //Superfood
        if(superFoodTimer == 0 && !initial) {
            if(rand() % 25 == 1) {
                superFoodTimer = 25;
                position * freeField = getRandomFreeField();
                superFoodX = arrayPosToCoords(freeField -> x);
                superFoodY = arrayPosToCoords(freeField -> y);
                fieldArray[freeField -> x][freeField -> y] = 11;
            }
        }
        else if(superFoodTimer > 0) {
            superFoodTimer = superFoodTimer-1;
            if(superFoodTimer == 0) {
                fieldArray[coordsToArrayPos(superFoodX)][coordsToArrayPos(superFoodY)] = 0;
            }
        }		
		
        //Freeze
        if(!freezeExists && initial == 0 && multiplayer) {
            if(rand() % 50 == 1) {
                freezeExists = true;
                position * freeField = getRandomFreeField();
                freezeX = arrayPosToCoords(freeField -> x);
                freezeY = arrayPosToCoords(freeField -> y);
                fieldArray[freeField -> x][freeField -> y] = 12;
            }
        }
        
        //Check if PowerUp is used by any player
        if(p1PowerUpButtonPressed) {
            p1PowerUpButtonPressed = false;
            if(p1PowerUp == 12) {
                p1PowerUp = 0;
                playerTwoFrozen = 15;
            }
            else if(p1PowerUp == 13){
				p1PowerUp = 0;
				reduceSnakeTwo = 1;
            }
       	    else if(p1PowerUp == 14){
				p1PowerUp = 0;
				inverseControlTwo = 50; 
			}
        }
        if(p2PowerUpButtonPressed) {
            p2PowerUpButtonPressed = false;
            if(p2PowerUp == 12) {
                p2PowerUp = 0;
                playerOneFrozen = 15;
            }
            else if(p2PowerUp == 13){
				p2PowerUp = 0;
				reduceSnakeOne = 1;
			}
			else if(p2PowerUp == 14){
				p2PowerUp = 0;
				inverseControlOne = 50; 
			}
        }
        
    }
}

/**
 * Method: Player1 has taken with superFood
 * Parameter: player is player1, snake is snake of player1
 * Return: updated list (node) from snake1
 */
node * superFoodFound(int player, node * snake) {
    writePlusSize(2, player);
    
    position * newPiece1 = newPosition(snake->tail->data->x,snake->tail->data->y);
    position * newPiece2 = newPosition(snake->tail->data->x,snake->tail->data->y);
    
    snake = addAsLast(snake,newPiece1);
    snake = addAsLast(snake,newPiece2);
    
    superFoodTimer = 0;
    
    return snake;
}

/**
 * Method: Player2 has taken superFood
 * Parameter: player is player2, snake is snake of player2
 * Return: updated list (node) from snake2
 */
node * superFoodFound2(int player, node * snake) {
    writePlusSize(2, player);

    position * newPiece1 = newPosition(snake->tail->data->x,snake->tail->data->y);
    position * newPiece2 = newPosition(snake->tail->data->x,snake->tail->data->y);

    snake = addAsLast2(snake,newPiece1);
    snake = addAsLast2(snake,newPiece2);

    superFoodTimer = 0;

    return snake;
}

/**
 * Method: Creating randomWalls for Level6
 */
void changeRandomWalls() {
    for(int i = 0; i<39; i++) {
        for (int j = 0; j<29; j++) {
            if(fieldArray[i][j] == 3) {
                fieldArray[i][j] = 0;
            }
        }
    }
    int i = 0;
    while(i<50) {
        position * randomPos = getRandomFreeField();
        if(randomPos -> x != coordsToArrayPos(p1X) && randomPos -> y != coordsToArrayPos(p1Y) && randomPos -> x != coordsToArrayPos(p2X) && randomPos -> y != coordsToArrayPos(p2Y)) {
            fieldArray[randomPos -> x][randomPos -> y] = 3;
            i++;
        }
    }
}

/**
 * Method: Creating Foodelement on random position
 */
void createRandomFood() {
    position * freeField = getRandomFreeField();

    //If food1 is still there, create food2
    if(fieldArray[coordsToArrayPos(food1X)][coordsToArrayPos(food1Y)] == 10) {
        food2X = arrayPosToCoords(freeField -> x);
        food2Y = arrayPosToCoords(freeField -> y);
    }
    else {
        food1X = arrayPosToCoords(freeField -> x);
        food1Y = arrayPosToCoords(freeField -> y);
    }

    fieldArray[freeField -> x][freeField -> y] = 10;
}

/**
 * Method: Player1 has taken foodElement
 * Parameter: player is player1, snake is snake of player1
 * Return: updated list (node) from snake1
 */
node * foodFound(int player, node * snake) {
    if (level == 6) {
        changeRandomWalls();
    }
    position * newPiece = newPosition(snake->tail->data->x,snake->tail->data->y);
    snake = addAsLast(snake,newPiece);
    writePlusSize(1, player);
    if(player == 1) {
        fieldArray[coordsToArrayPos(p1X)][coordsToArrayPos(p1Y)] = 0;
    }
    else {
        fieldArray[coordsToArrayPos(p2X)][coordsToArrayPos(p2Y)] = 0;
    }
    
    createRandomFood();
    
    return snake;
}

/**
 * Method: Player2 has taken foodElement
 * Parameter: player is player2, snake is snake of player2
 * Return: updated list (node) from snake2
 */
node * foodFound2(int player, node * snake) {
    if (level == 6) {
        changeRandomWalls();
    }
    position * newPiece = newPosition(snake->tail->data->x,snake->tail->data->y);
    snake = addAsLast2(snake,newPiece);
    writePlusSize(1, player);
    if(player == 1) {
        fieldArray[coordsToArrayPos(p1X)][coordsToArrayPos(p1Y)] = 0;
    }
    else {
        fieldArray[coordsToArrayPos(p2X)][coordsToArrayPos(p2Y)] = 0;
    }

    createRandomFood();

    return snake;
}

/**
 * Method: reset Game to startconfiguration, to start game new again
 */
void reset(){
	animateLevelDesign=1;
	initial = true;
	update=0;
	entered=0;
	entered2=0;
	readHighscoreFile=1;
    levelInitialized = false;
	animateLevel=0;
    superFoodTimer = 0;
	buttons.buttons[KEYCODE(W)]=0;
	buttons.buttons[KEYCODE(A)]=0;
	buttons.buttons[KEYCODE(S)]=0;
	buttons.buttons[KEYCODE(D)]=0;
	p1NextDirection='0';
	p1Direction='0';
	p1X=190;
	p1Y=190;
	p1Size=1;
	startClock=0;
	hours=0;
	minutes=0;
	seconds=0;
	gameLost=false;
	winnerPlayer = 0;
	initialStop = 1;
	p1Ready=false;
	p2Ready=false;
	frameTicks=5;
	disableSpawn=0;
	animateLevel=0;
    
    p1PowerUp = 0;
    
	if(multiplayer){
		buttons.buttons[KEYCODE(UP)]=0;
		buttons.buttons[KEYCODE(DOWN)]=0;
		buttons.buttons[KEYCODE(LEFT)]=0;
		buttons.buttons[KEYCODE(RIGHT)]=0;
		p2NextDirection='0';
		p2Direction='0';
		p2X=350;
		p2Y=190;
		p2Size=1;
        
        p2PowerUp = 0;
        
        freezeExists = false;
        playerOneFrozen = 0;
        playerTwoFrozen = 0;

        reduceExists = 0;
        reduceSnakeOne = 0;
        reduceSnakeTwo = 0;

        inverseExists = 0;
        inverseControlOne = 0;
        inverseControlTwo = 0;
	}
}

/**
 * Method: update the Highscore table
 */
void updateHighScore(){
	int hour[10];
	int minute[10];
	int second[10];
	char names[10][20];
	int point[10];
	char str[100];
	char levels[10][20];
	char walls[10][20];
	char currentLevel[10]="Level";
	char currentWall[10]="NoWall";

	if(level==1){
		currentLevel[5]='1';
	}
	if(level==2){
		currentLevel[5]='2';
	}
	if(level==3){
		currentLevel[5]='3';
	}
	if(level==4){
		currentLevel[5]='4';
	}
	if(level==5){
		currentLevel[5]='5';
	}
	if(level==6){
		currentLevel[5]='6';
	}
	if(!wallBorder){
		currentWall[0]=' ';
		currentWall[1]=' ';
	}

	file = fopen("../files/Highscore.txt","r");
	for(int i = 0; i<10; i++){
		sscanf(fgets(str,50,file),"%d:%d:%d %s %d %s %s",&hour[i],&minute[i],&second[i],&names[i][0],&point[i],&levels[i][0],&walls[i][0]);
	}
	fclose(file);
	remove("../files/Highscore.txt");
	file = fopen("../files/Highscore.txt","w+b");
	for(int i = 0; i < 10; i++){
		scanf(fgets(str,50,file),"%d:%d:%d %s %d %s %s",&hour[i],&minute[i],&second[i],&names[i][0],&point[i],&levels[i][0],&walls[i][0]);
	}
	int i = 0;
	for(int j = 0; j < 10 && i < 10; j++){
		if(p1Size > point[i] && !entered){
			fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hours,minutes%60,seconds%60,p1Name,p1Size,currentLevel,currentWall);
			entered = 1;
			j++;
		}
		if(p1Size == point[i] && !entered){
			entered=1;
			int tempTime = second[i]+minute[i]*60+hour[i]*3600;
			int gameTime = seconds%60+(minutes%60)*60+(hours)*3600;
			if(tempTime<gameTime){
				fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hour[i],minute[i],second[i],names[i],point[i],levels[i],walls[i]);
				j++;
				fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hours,minutes%60,seconds%60,p1Name,p1Size,currentLevel,currentWall);
				i++;
				j++;
			}
			else{
				fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hours,minutes%60,seconds%60,p1Name,p1Size,currentLevel,currentWall);
				j++;
			}
		}
		if(!(j==10 && i==9)){
			fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s\n", hour[i],minute[i],second[i],names[i],point[i],&levels[i][0],&walls[i][0]);
		}
		i++;
	}
	if(multiplayer){
		fclose(file);
		file = fopen("../files/Highscore.txt","r");
		for(int i = 0; i < 10; i++){
			sscanf(fgets(str,50,file),"%d:%d:%d %s %d %s %s",&hour[i],&minute[i],&second[i],&names[i][0],&point[i],&levels[i][0],&walls[i][0]);
		}
		fclose(file);
		remove("../files/Highscore.txt");
		
		file = fopen("../files/Highscore.txt","w+b");
		int i = 0;
		for(int j = 0; j < 10 && i < 10; j++){
			if(p2Size > point[i] && !entered2){
				fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hours,minutes%60,seconds%60,p2Name,p2Size,currentLevel,currentWall);
				entered2 = 1;
				j++;
			}
			if(p2Size == point[i] && !entered2){
				int tempTime = second[i]+minute[i]*60+hour[i]*3600;
				int gameTime = seconds%60+(minutes%60)*60+(hours)*3600;
				if(tempTime<gameTime){
					fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hour[i],minute[i],second[i],names[i],point[i],levels[i],walls[i]);
				}
				else{
					fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s \n", hours,minutes%60,seconds%60,p2Name,p2Size,currentLevel,currentWall);
				}
				entered2 = 1;
				j++;
			}
			if(!(j==10 && i==9)){
				fprintf(file,"%.2d:%.2d:%.2d %s %d %s %s\n", hour[i],minute[i],second[i],names[i],point[i],&levels[i][0],&walls[i][0]);
			}
			i++;
		}
	}
	fclose(file);
}

/**
 * Method: Drawing the Home Logo to get to Main Menu after loosing Game
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void gameOverDrawLogo(int mousePosX, int mousePosY){
	static int image_width;
	if((image_width = tumDrawGetLoadedImageWidth(home_image)) != -1 && tumDrawGetLoadedImageWidth(home_image2) != -1){
		if(mousePosX >= 10 && mousePosX <= 60 && mousePosY >= 10 && mousePosY <= 60){
			checkDraw(tumDrawLoadedImage(home_image2, 10, 10),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				initialStop=1;
				initial=true;
				//Reset fieldArray
				for(int i=0;i<39;i++){
					for(int j=0;j<29;j++){
						fieldArray[i][j]=0;
					}
				}

				//Reset Snake1 Tail
				while(p1Size>-1){
					removeLast(snake1);
					p1Size--;
				}
				//Reset Snake2 Tail
				while(multiplayer && p2Size>-1){
					removeLast(snake2);
					p2Size--;
				}
				parameterReset();
				reset();
				//Create snake1 List with 3 Elements
				randomStart = getRandomFreeField();
				p1X = arrayPosToCoords(randomStart -> x);
				p1Y = arrayPosToCoords(randomStart -> y);

				randomStart = getRandomFreeField();
				p2X = arrayPosToCoords(randomStart -> x);
				p2Y = arrayPosToCoords(randomStart -> y);

				position * p21 = newPosition(p1X,p1Y);
				snake1 = addAsFirst(snake1,p21);
				position * p31 = newPosition(p1X,p1Y);
				snake1 = addAsFirst(snake1,p31);
				fieldArray[coordsToArrayPos(p1X)][coordsToArrayPos(p1Y)] = 1;

				//Create snake2 List with 3 Elements
				if(multiplayer){
					position * p22 = newPosition(p1X,p1Y);
					snake2 = addAsFirst(snake2,p22);
					position * p32 = newPosition(p1X,p1Y);
					snake2 = addAsFirst(snake2,p32);
					twoFoodElements=1;
					fieldArray[coordsToArrayPos(p2X)][coordsToArrayPos(p2Y)] = 2;
				}
				p1Ready = false;
				p2Ready = false;

				animateLevelDesign=0;
				drawlevel=0;
				refresh();
				currentState--;
			}
		}
		else{
			checkDraw(tumDrawLoadedImage(home_image, 10 , 10),__FUNCTION__);
		}
	}
	else{
		printf("LOAD PIC FAILED!!");
	}
}

/**
 * Method: Checking if Game is lost in any case (hit wall, hit player, hit himself)
 * and create GameOverScreen
 */
void checkGameOver() {
	 static unsigned int mousePosX, mousePosY;
	 mousePosX = tumEventGetMouseX();
	 mousePosY = tumEventGetMouseY();
	 static char str1[100];
     
    if(winnerPlayer != 0) {
        gameLost = true;
    }
    if(gameLost) {
        //Create Lost Element on Screen
        static char str[100] = { 0 };
        if(!multiplayer) {
            sprintf(str, "%s",p1Name);
            strcat(str," LOST");
        }
        else {
            if(winnerPlayer == 0) {
                sprintf(str,"Game Over");
            }
            else if(winnerPlayer == 1) {
                sprintf(str, "%s",p1Name);
                strcat(str," WON");
            }
            else {
                sprintf(str,"%s",p2Name);
                strcat(str," WON");
            }
        }
        disableSpawn=1;
		if(!update){
			update = 1;
			updateHighScore();
		}
		gameLost=true;
        gameOverScreen();
        checkDraw(tumDrawText(str,250,75, Red),__FUNCTION__);
        //Buttons
        sprintf(str1,"Restart");
        if(mousePosX >=100  && mousePosX <=180  && mousePosY >=100  && mousePosY <=150 ){
        	checkDraw(tumDrawFilledBox(100,100,80,50,Grey),__FUNCTION__);
        	checkDraw(tumDrawFilledBox(105,105,70,40,Black),__FUNCTION__);
        	checkDraw(tumDrawText(str1,115,115, Grey),__FUNCTION__);
        	//Button Point Clicked
        	if(tumEventGetMouseLeft()){
        		initialStop=1;
        		initial=true;
        		//Reset fieldArray
        		for(int i=0;i<39;i++){
        			for(int j=0;j<29;j++){
        				fieldArray[i][j]=0;
        			}
        		}
        		//Reset Snake1 Tail
        		while(p1Size>-1){
        			removeLast(snake1);
        			p1Size--;
        		}
        		//Reset Snake2 Tail
        		while(multiplayer && p2Size>-1){
        			removeLast(snake2);
        			p2Size--;
        		}
        		//Set snake2 to NULL
        		if(multiplayer){
        			//snake2=NULL;
        		}
        		parameterReset();
        		reset();

        		//Create snake1 List with 3 Elements
        		randomStart = getRandomFreeField();
        		p1X = arrayPosToCoords(randomStart -> x);
        		p1Y = arrayPosToCoords(randomStart -> y);

        		randomStart = getRandomFreeField();
        		p2X = arrayPosToCoords(randomStart -> x);
        		p2Y = arrayPosToCoords(randomStart -> y);

        		position * p21 = newPosition(p1X,p1Y);
        		snake1 = addAsFirst(snake1,p21);
        		position * p31 = newPosition(p1X,p1Y);
        		snake1 = addAsFirst(snake1,p31);
        		fieldArray[coordsToArrayPos(p1X)][coordsToArrayPos(p1Y)] = 1;

        		//Create snake2 List with 3 Elements
        		if(multiplayer){
        			position * p22 = newPosition(p1X,p1Y);
        			snake2 = addAsFirst(snake2,p22);
        			position * p32 = newPosition(p1X,p1Y);
        			snake2 = addAsFirst(snake2,p32);
        			twoFoodElements=1;
        			fieldArray[coordsToArrayPos(p2X)][coordsToArrayPos(p2Y)] = 2;
        		}
        		p1Ready = false;
        		p2Ready = false;

        		animateLevelDesign=0;
        		drawlevel=0;
        		refresh();
        	}
        }
        else{
        	checkDraw(tumDrawFilledBox(100,100,80,50,White),__FUNCTION__);
        	checkDraw(tumDrawFilledBox(105,105,70,40,Black),__FUNCTION__);
        	checkDraw(tumDrawText(str1,115,115, White),__FUNCTION__);
        }

        sprintf(str1,"Highscore");
        if(mousePosX >=425  && mousePosX <=505  && mousePosY >=100  && mousePosY <=150 ){
        	checkDraw(tumDrawFilledBox(425,100,80,50,Grey),__FUNCTION__);
        	checkDraw(tumDrawFilledBox(430,105,70,40,Black),__FUNCTION__);
        	checkDraw(tumDrawText(str1,430,115, Grey),__FUNCTION__);
        	//Button Point Clicked
        	if(tumEventGetMouseLeft()){
        		currentState++;
        	}
        }
        else{
        	checkDraw(tumDrawFilledBox(425,100,80,50,White),__FUNCTION__);
        	checkDraw(tumDrawFilledBox(430,105,70,40,Black),__FUNCTION__);
        	checkDraw(tumDrawText(str1,430,115, White),__FUNCTION__);
        }
        gameOverDrawLogo(mousePosX,mousePosY);


    }
    else if(winnerPlayer != 0){
        //Create Winner is... Element on Screen
        static char str[100] = { 0 };
        if(winnerPlayer == 1) {
            sprintf(str, "%s", p1Name);
        }
        else {
            sprintf(str, "2");
        }
        sprintf(str, "%s", p2Name);
        disableSpawn=1;
        if(!update){
			update = 1;
			updateHighScore();
		}
        gameLost=true;
        gameOverScreen();
        checkDraw(tumDrawText(str,250,75, Red),__FUNCTION__);
        gameOverDrawLogo(mousePosX,mousePosY);
    }
}

/**
 * Method: Checking specific size reached, then increase speed
 * by decreasing framesTicks
 */
void incrementSpeed() {
    if(p1Size==25 || p2Size == 25){
        frameTicks=4;
    }
    else if((p1Size==50 || p2Size==50) && frameTicks == 4){
        frameTicks=3;
    }
    else if((p1Size==75 || p2Size==75) && frameTicks == 3){
        frameTicks=2;
    }
    else if((p1Size==100 || p2Size==100) && frameTicks == 2){
        frameTicks=1;
    }
}

/*
 * Method: Prevent snake1 movement in opposite direction
 */
void playerOneGetNextDirection(){
	if(inverseControlOne > 0){
		if(buttons.buttons[KEYCODE(W)] && p1Direction != 'U'){
			p1NextDirection = 'D';
		}
		if(buttons.buttons[KEYCODE(D)] && p1Direction != 'R'){
			p1NextDirection = 'L';
		}
		if(buttons.buttons[KEYCODE(S)] && p1Direction != 'D'){
			p1NextDirection = 'U';
		}
		if(buttons.buttons[KEYCODE(A)] && p1Direction != 'L'){
			p1NextDirection = 'R';
		}
		if(frame == frameTicks){
			inverseControlOne--;
		}
	}
	else{
		if(buttons.buttons[KEYCODE(W)] && p1Direction != 'D'){
			p1NextDirection = 'U';
		}
		if(buttons.buttons[KEYCODE(D)] && p1Direction != 'L'){
			p1NextDirection = 'R';
		}
		if(buttons.buttons[KEYCODE(S)] && p1Direction != 'U'){
			p1NextDirection = 'D';
		}
		if(buttons.buttons[KEYCODE(A)] && p1Direction != 'R'){
			p1NextDirection = 'L';
		}
	}
	if(buttons.buttons[SDL_SCANCODE_SPACE] && !p1PowerUpButtonPressed) {
        p1PowerUpButtonPressed = true;
    }
}

/*
 * Method: Prevent snake2 movement in opposite direction
 */
void playerTwoGetNextDirection(){
	if(inverseControlTwo > 0){
		if(buttons.buttons[KEYCODE(UP)] && p2Direction != 'U'){
			p2NextDirection = 'D';
		}
		if(buttons.buttons[KEYCODE(RIGHT)] && p2Direction != 'R'){
			p2NextDirection = 'L';
		}
		if(buttons.buttons[KEYCODE(DOWN)] && p2Direction != 'D'){
			p2NextDirection = 'U';
		}
		if(buttons.buttons[KEYCODE(LEFT)] && p2Direction != 'L'){
			p2NextDirection = 'R';
		}
		if(frame == frameTicks){
			inverseControlTwo--;
		}
	}
	else{
		if(buttons.buttons[KEYCODE(UP)] && p2Direction != 'D'){
			p2NextDirection = 'U';
		}
		if(buttons.buttons[KEYCODE(RIGHT)] && p2Direction != 'L'){
			p2NextDirection = 'R';
		}
		if(buttons.buttons[KEYCODE(DOWN)] && p2Direction != 'U'){
			p2NextDirection = 'D';
		}
		if(buttons.buttons[KEYCODE(LEFT)] && p2Direction != 'R'){
			p2NextDirection = 'L';
		}
	}
	if(buttons.buttons[SDL_SCANCODE_RSHIFT] && !p2PowerUpButtonPressed) {
        p2PowerUpButtonPressed = true;
    }
}

/**
 * Method: Checking Collision from each player with anything on the screen via fieldArray
 * Parameter: snake1 is snake from player1, snake2 is snake from player2
 */
void collisionDetection(node * snake1, node * snake2) {
    tempX = coordsToArrayPos(p1X);
    tempY = coordsToArrayPos(p1Y);
    fieldValue = fieldArray[tempX][tempY];

    if(fieldValue == 0) {
        //Field is empty, nothing happens
    }
    else if(fieldValue == 1) {
        //layer 1 hit player 1, lost if singleplayer, else player 2 or nobody wins
        if(!multiplayer) {
            gameLost = true;
        }
        else {
            winnerPlayer = 2;
        }
    }
    else if(fieldValue == 2) {
        //player 1 hit player 2, player 2 wins if he hits nothing
        winnerPlayer = 2;
    }
    else if(fieldValue == 3) {
        //player 1 hit wall, lost if singleplayer, else player 2 or nobody wins
        if(!multiplayer) {
            gameLost = true;
        }
        else {
            winnerPlayer = 2;
        }
    }
    else if(fieldValue == 10) {
        //player 1 hit foodelement
        snake1 = foodFound(1, snake1);
    }
    else if(fieldValue == 11) {
        //player 1 hit superfood
        snake1 = superFoodFound(1, snake1);
    }
    else if(fieldValue == 12) {
        p1PowerUp = 12;
        freezeExists = false;
    }
    else if(fieldValue == 13) {
		p1PowerUp = 13;
		reduceExists = 0;
	}
	else if(fieldValue == 14) {
		p1PowerUp  = 14;
		inverseExists = 0;
	}
	else if(fieldValue == 20) {
        snake1 = teleport(snake1, 1);
    }

    if(multiplayer) {
        tempX = coordsToArrayPos(p2X);
        tempY = coordsToArrayPos(p2Y);
        fieldValue = fieldArray[tempX][tempY];

        if(fieldValue == 0) {
            //Field is empty, nothing happens
        }
        else if(fieldValue == 1 || fieldValue == 2 || fieldValue == 3) {
            //player 2 hit player 1, player 2 or the wall, player 1 or nobody wins
            if(winnerPlayer != 2) {
                winnerPlayer = 1;
            }
            else {
				if(p1Size > p2Size){
					winnerPlayer = 1;
				}
				else if(p2Size > p1Size){
					winnerPlayer = 2;
				}
				else{
					winnerPlayer = 0;
					gameLost = true;
				}
            }
        }
        else if(fieldValue == 10) {
            //player 2 hit foodelement
            snake2 = foodFound2(2, snake2);
        }
        else if(fieldValue == 11) {
            //player 2 hit superfood
            snake2 = superFoodFound2(2, snake2);
        }
        else if(fieldValue == 12) {
			p2PowerUp = 12;
			freezeExists = false;
		}
		else if(fieldValue == 13) {
			p2PowerUp = 13;
			reduceExists = 0;
		}
		else if(fieldValue == 14) {
			p2PowerUp  = 14;
			inverseExists = 0;
		}
        else if(fieldValue == 20) {
            snake2 = teleport(snake2, 2);
        }
    }
}

/**
 * Method: Player Name enter Screen, possibility to get back to mainMenu
 * Parameter: snake1 is snake from player1, snake2 is snake from player2
 */
void backToMenu(node * snake1, node * snake2) {
	if(currentState==1){
		initialStop=1;
        initial=true;
        //Reset fieldArray
        for(int i=0;i<39;i++){
            for(int j=0;j<29;j++){
                fieldArray[i][j]=0;
            }
        }
        //Reset Snake1 Tail
        while(p1Size>-1){
            removeLast(snake1);
            p1Size--;
        }
        //Reset Snake2 Tail
        while(multiplayer && p2Size>-1){
            removeLast(snake2);
            p2Size--;
        }
        //Set snake2 to NULL
        if(multiplayer){
            //snake2=NULL;
        }
        parameterReset();
        reset();
        //Create snake1 List with 3 Elements
        position * p21 = newPosition(p1X,p1Y);
        snake1 = addAsFirst(snake1,p21);
        position * p31 = newPosition(p1X,p1Y);
        snake1 = addAsFirst(snake1,p31);

        //Create snake2 List with 3 Elements
        if(multiplayer){
            position * p22 = newPosition(p1X,p1Y);
            snake2 = addAsFirst(snake2,p22);
            position * p32 = newPosition(p1X,p1Y);
            snake2 = addAsFirst(snake2,p32);
        }
        refresh();
    }
}

/**
 * Method: drawing the player on the screen
 * Parameter: snake is the snake of player1/2,
 * 				color = 1 for player 1 create Red Box
 * 				color = 2 for player 2 create Blue Box
 */
void drawPlayer(node * snake, int color) {

    snake = snake -> head -> next;
    while(snake -> next != NULL){
        checkDraw(tumDrawFilledBox(snake->data->x,snake->data->y,5,5,White), __FUNCTION__);
        snake = snake -> next;
    }
    //Draw the last Element
    checkDraw(tumDrawFilledBox(snake->data->x,snake->data->y,5,5,White), __FUNCTION__);

    snake = snake -> head;
    if(color == 1) {
        checkDraw(tumDrawFilledBox(snake->head->data->x,snake->head->data->y,5,5,Red), __FUNCTION__);
    }
    else if(color == 2) {
        checkDraw(tumDrawFilledBox(snake->head->data->x,snake->head->data->y,5,5,Blue), __FUNCTION__);
    }


}

/**
 * Method: Drawing the Gameboard
 * Parameter: snake1 is snake from player1, snake2 is snake from player2
 */
void drawBoard(node * snake1, node * snake2) {
	//Draw Player1
    drawPlayer(snake1, 1);

    //Draw Player 2 and tail if multiplayer
    if(multiplayer) {
        drawPlayer(snake2, 2);
    }

    //Draw normalFood
    checkDraw(tumDrawFilledBox(food1X,food1Y, 5, 5, Green), __FUNCTION__);
    if(multiplayer) {
        checkDraw(tumDrawFilledBox(food2X,food2Y, 5, 5, Green), __FUNCTION__);
    }

    //Draw Superfood if it exists atm
    if(superFoodTimer > 0) {
        checkDraw(tumDrawFilledBox(superFoodX+2-(superFoodTimer / 2),superFoodY+2-(superFoodTimer / 2), superFoodTimer, superFoodTimer, Green), __FUNCTION__);
        superFoodCoverUpDrawn = false;
    }
    //Little Green Box stays, has to be drawn over once
    else if(!superFoodCoverUpDrawn && superFoodTimer == 0) {
        checkDraw(tumDrawFilledBox(superFoodX+2,superFoodY+2, 2, 2, Black), __FUNCTION__);
        superFoodCoverUpDrawn = true;
    }

    //Draw freezeItem if it exists atm
    if(freezeExists) {
        checkDraw(tumDrawFilledBox(freezeX,freezeY, 5, 5, Cyan), __FUNCTION__);
    }

    //Draw ReduceItem if it exists
    if(reduceExists) {
		checkDraw(tumDrawFilledBox(reduceX,reduceY, 5, 5, Yellow), __FUNCTION__);
	}

    //Draw InverseItem if it exists
    if(inverseExists) {
		checkDraw(tumDrawFilledBox(inverseX,inverseY, 5, 5, Grey), __FUNCTION__);
	}

    //A Little Note is drawn when the snake eats something.
    if(plusSizeWrite){
        if(plusSizeWriteGreen) {
            checkDraw(tumDrawText(plusSizeCharArr,plusSizeX-20,plusSizeY-20, Green),__FUNCTION__);
        }
        else {
            checkDraw(tumDrawText(plusSizeCharArr,plusSizeX-20,plusSizeY-20, Red),__FUNCTION__);
        }
        if(plusSizeCounter == 15) {
            plusSizeWrite = false;
        }
        plusSizeCounter++;
    }

    //Draw PowerUps in possesion of P1 and P2
    if(p1PowerUp != 0) {
        static char str[100] = { 0 };
        sprintf(str,"P1 Powerup:");
        checkDraw(tumDrawText(str, 5, 120, White),__FUNCTION__);
        if(p1PowerUp == 12) {
            checkDraw(tumDrawFilledBox(25, 148, 5, 5, Cyan), __FUNCTION__);
            sprintf(str, "Freeze");
            checkDraw(tumDrawText(str, 35, 140, White),__FUNCTION__);
        }
        else if(p1PowerUp == 13) {
            checkDraw(tumDrawFilledBox(25, 148, 5, 5, Yellow), __FUNCTION__);
            sprintf(str, "Reduce");
            checkDraw(tumDrawText(str, 35, 140, White),__FUNCTION__);
        }
        else if(p1PowerUp == 14) {
            checkDraw(tumDrawFilledBox(25, 148, 5, 5, Grey), __FUNCTION__);
            sprintf(str, "Inverse");
            checkDraw(tumDrawText(str, 35, 140, White),__FUNCTION__);
        }
    }
    if(p2PowerUp != 0) {
        static char str[100] = { 0 };
        sprintf(str,"P2 Powerup:");
        checkDraw(tumDrawText(str, 515, 120, White),__FUNCTION__);
        if(p2PowerUp == 12) {
            checkDraw(tumDrawFilledBox(535, 148, 5, 5, Cyan), __FUNCTION__);
            sprintf(str, "Freeze");
        }
        else if(p2PowerUp == 13) {
            checkDraw(tumDrawFilledBox(535, 148, 5, 5, Yellow), __FUNCTION__);
            sprintf(str, "Reduce");
        }
        else if(p2PowerUp == 14) {
            checkDraw(tumDrawFilledBox(535, 148, 5, 5, Grey), __FUNCTION__);
            sprintf(str, "Inverse");
        }
        checkDraw(tumDrawText(str, 545, 140, White),__FUNCTION__);
    }
    if(infosPressed) {
        //Create infoscreen
		static char p1c1[100] = { 0 };
        sprintf(p1c1, "Moving: W,A,S,D");
		static char p1c2[100] = { 0 };
        sprintf(p1c2, "PowerUp: Spacebar");
        static char p2c1[100] = { 0 };
        sprintf(p2c1, "Moving: Arrow keys");
		static char p2c2[100] = { 0 };
        sprintf(p2c2, "PowerUp: Right shift");
        static char pu1[100] = { 0 };
        sprintf(pu1, "Freeze");
        static char pu2[100] = { 0 };
        sprintf(pu2, "Reduce");
        static char pu3[100] = { 0 };
        sprintf(pu3, "Inverse");
        static char con1[100] = { 0 };
        sprintf(con1, "P:Pause");
        static char con2[100] = { 0 };
        sprintf(con2, "I:Infoscreen");
        static char con3[100] = { 0 };
        sprintf(con3, "Esc:Exit");

        checkDraw(tumDrawText(p1c1,5,410, White),__FUNCTION__);
        checkDraw(tumDrawText(p1c2,5,430, White),__FUNCTION__);

        checkDraw(tumDrawText(con1, 185, 430, White),__FUNCTION__);
        checkDraw(tumDrawText(con2, 265, 430, White),__FUNCTION__);
        checkDraw(tumDrawText(con3, 385, 430, White),__FUNCTION__);

        if(multiplayer) {
            checkDraw(tumDrawText(p2c1,480,410, White),__FUNCTION__);
            checkDraw(tumDrawText(p2c2,480,430, White),__FUNCTION__);

            checkDraw(tumDrawFilledBox(175, 418, 5, 5, Cyan), __FUNCTION__);
            checkDraw(tumDrawText(pu1, 185, 410, White),__FUNCTION__);
            checkDraw(tumDrawFilledBox(275, 418, 5, 5, Yellow), __FUNCTION__);
            checkDraw(tumDrawText(pu2, 285, 410, White),__FUNCTION__);
            checkDraw(tumDrawFilledBox(375, 418, 5, 5, Gray), __FUNCTION__);
            checkDraw(tumDrawText(pu3, 385, 410, White),__FUNCTION__);
        }

    }

    if(inverseControlOne > 0){
        static char str[100] = { 0 };
        sprintf(str,"Counter :");
        checkDraw(tumDrawText(str, 5, 160, White),__FUNCTION__);
		sprintf(str, "%d", inverseControlOne);
        checkDraw(tumDrawText(str, 75, 160, White),__FUNCTION__);
    }

    if(inverseControlTwo > 0){
		static char str[100] = { 0 };
        sprintf(str,"Counter :");
        checkDraw(tumDrawText(str, 515, 160, White),__FUNCTION__);
		sprintf(str, "%d", inverseControlTwo);
        checkDraw(tumDrawText(str, 585, 160, White),__FUNCTION__);
	}
}

/**
 * Method: Start snake1 Screen and the snake1 programm
 * Parameter: ? (something for the RTOS Emulator)
 */
void vGameScreen(void *pvParameters)
{
    //rand() always gets the same results in the beginning, adding a seed for the first actual call
    int mx = tumEventGetMouseX();
    int my = tumEventGetMouseY();
    int randomSeed = mx;
    randomSeed = randomSeed * my;
    randomSeed = randomSeed % 100;
    while(randomSeed > 0) {
        randomSeed--;
    }
    
    position * p0 = newPosition(0,0);
    snake1 = createLinkedList(p0);
    snake2 = createLinkedList(p0);

    

 	while(1) {
		if(DrawSignal) {
			if(xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
				pdTRUE) {
				//initial set snake1 direction, start timer, clock is started
				if(initial){
                    
                    if(!levelInitialized) {
                        if(level == 1) {
                            //Level 1 has no inner walls, do nothing
                        }
                        else if (level == 2) {
                            for(int i=0;i<15;i++){
                                fieldArray[i][9] = 3;
                                fieldArray[i][19] = 3;
                                fieldArray[i+24][9] = 3;
                                fieldArray[i+24][19] = 3;
                            }
                        }
                        else if(level == 3) {
                            for(int i=0; i<27; i++) {
                                fieldArray[i+6][9] = 3;
                                fieldArray[i+6][19] = 3;
                            }
                            for(int i = 0; i<36; i++) {
                                fieldArray[i%6][i/6] = 3;
                                fieldArray[i%6+33][i/6] = 3;
                                fieldArray[i%6][i/6+23] = 3;
                                fieldArray[i%6+33][i/6+23] = 3;
                            }
                        }
                        else if(level == 4){
                        	for(int i=4;i<15;i++){
                        		fieldArray[i][9]=3;
                        		fieldArray[i][19]=3;
                        		fieldArray[i+20][9]=3;
                        		fieldArray[i+20][19]=3;
                        	}
                        	for(int i=4;i<10;i++){
                        		fieldArray[14][i]=3;
                        		fieldArray[24][i]=3;
                        		fieldArray[14][i+15]=3;
                        		fieldArray[24][i+15]=3;
                        	}
                        }
                        else if(level == 5){
                             for(int i = 0; i<=28; i++) {
                                fieldArray[9][i] = 3;
                                fieldArray[19][i] = 3;
                                fieldArray[29][i] = 3;
                            }
                            fieldArray[4][6] = 20;
                            fieldArray[4][22] = 20;
                            fieldArray[14][6] = 20;
                            fieldArray[14][22] = 20;
                            fieldArray[24][6] = 20;
                            fieldArray[24][22] = 20;
                            fieldArray[34][6] = 20;
                            fieldArray[34][22] = 20;
                        }
                        else if(level==6){
                            for(int i = 0; i<50; i++) {
                                position * randomPos = getRandomFreeField();
                                addInitfield(randomPos->x,randomPos->y,i);
                                fieldArray[randomPos -> x][randomPos -> y] = 3;
                            }
                        }
                        
                        //Random position for player One
                        randomStart = getRandomFreeField();
    	                p1X = arrayPosToCoords(randomStart -> x);
                    	p1Y = arrayPosToCoords(randomStart -> y);

                    	//Random position for player Two
                    	randomStart = getRandomFreeField();
                    	p2X = arrayPosToCoords(randomStart -> x);
                    	p2Y = arrayPosToCoords(randomStart -> y);


                    	fieldArray[coordsToArrayPos(p1X)][coordsToArrayPos(p1Y)] = 1;
                    	position * p11 = newPosition(p1X,p1Y);
                    	snake1 = createLinkedList(p11);
                    	position * p21 = newPosition(p1X,p1Y);
                    	snake1 = addAsFirst(snake1,p21);
                    	position * p31 = newPosition(p1X,p1Y);
                    	snake1 = addAsFirst(snake1,p31);

                    	if(multiplayer){
                    		fieldArray[coordsToArrayPos(p2X)][coordsToArrayPos(p2Y)] = 2;
                    	}
                    	position * p12 = newPosition(p1X,p1Y);
                    	snake2 = createLinkedList(p12);
                    	position * p22 = newPosition(p1X,p1Y);
                    	snake2 = addAsFirst(snake2,p22);
                    	position * p32 = newPosition(p1X,p1Y);
                    	snake2 = addAsFirst(snake2,p32);

                    	p1Ready = false;
                    	p2Ready = false;
                        
                        createRandomFood();
                        if(multiplayer) {
                            createRandomFood();
                        }
                        levelInitialized = true;
                    }
                    
                    if(animateLevelDesign){
                    	if(frame==4){
                    		animateLevel=1;
                    	}
                    	if(animateLevel){
                    		initLevel(level,wallBorder,fieldArray);
                    	}
                    }
                    
                    
                    if(buttons.buttons[KEYCODE(W)] || buttons.buttons[KEYCODE(A)] || buttons.buttons[KEYCODE(S)] || buttons.buttons[KEYCODE(D)]) {
                        p1Ready = true;
                    }
                    if(buttons.buttons[KEYCODE(UP)] || buttons.buttons[KEYCODE(DOWN)] || buttons.buttons[KEYCODE(LEFT)] || buttons.buttons[KEYCODE(RIGHT)]) {
                        p2Ready = true;
                    }
                    if(p1Ready && (p2Ready || !multiplayer)) {
                        start_t = clock();
                        startClock=1;
                        drawlevel=0;
                        initial = false;
					    }
				}
				
				//To Activate KI Comment Out only Singleplayer snake1 played by Computer
                //KI();
                //KILong();
                //KISuperLong();
                
				//Input of Keyboard
				xGetButtonInput();
                
				//Check Pause Button is activated
                pause();
                
                //Check Infoscreen Button is activated
                infoscreen();
                
                //Increment Speed after every 25 Points for every snake
                incrementSpeed();

                //Check Game Over criteria
                checkGameOver();
                
                //Check next direction of Player and if PowerUpButtons is Pressed
                playerOneGetNextDirection();
                if(multiplayer) {
                    playerTwoGetNextDirection();
                }
                /*
                 * After every frameTicks
                 * Check next direction of Player
                 * ?
                 * set frame = 0
                 * refresh the screen
                 */
                if(frame == frameTicks) {
                    initialStop=0;
                    if(playerOneFrozen == 0) {
                        snake1 = playerStep(snake1, 1);
                    }
                    else {
                        playerOneFrozen--;
                    }
                    if(multiplayer) {
                        if(playerTwoFrozen == 0) {
                            snake2 = playerStep(snake2, 2);
                        }
                        else {
                            playerTwoFrozen--;
                        }
                    }
                    
                    if(reduceSnakeOne){
                        writePlusSize(-1, 1);
						reduceSnakeOne = 0;
                        if(p1Size > 0) {
                            fieldArray[coordsToArrayPos(snake1 -> tail -> data -> x)] [coordsToArrayPos(snake1 -> tail -> data -> y)] = 0;
                            removeLast(snake1);
                            p1Size--;
                        }
						else{
							winnerPlayer = 2;
						}
					}
					if(reduceSnakeTwo){
                        writePlusSize(-1, 2);
						reduceSnakeTwo = 0;
                        if(p2Size > 0) {
                            fieldArray[coordsToArrayPos(snake2 -> tail ->data ->x)] [coordsToArrayPos(snake2 -> tail ->data ->y)] = 0;
                            removeLast(snake2);
                            p2Size--;
                        }
                        else{
							winnerPlayer = 1;
						}
					}
					
                    if(!initial) {
                        collisionDetection(snake1, snake2);
                    }
                
                    //Spawn and manage powerUps
                    powerUps();
                    
                    frame = 0;
                    
                    refresh();
                }

                if(!gameLost){
                //Draw the entire Board
                	drawBoard(snake1, snake2);
                }
                
                //Switch Back to Menu
				backToMenu(snake1, snake2);
                
                if(!gameLost && winnerPlayer == 0 && !pausePressed && (!initial || initialStop == 1)) {
                    frame++;
                }
                
                xSemaphoreGive(ScreenLock);

				//Input eingaben von C oder P
				vCheckStateInput();
			}
        }
    }
}

/**
 * Method: Teleporting the Player1/2 Level5
 * Parameter: snake is the snake of player1/2,
 * 				player is player1/2
 * Return: updated list (node) from snake1/2
 */
node * teleport(node * snake, int player) {
    if(player==1) {
        if(coordsToArrayPos(p1Y) == 6) {
            p1X = arrayPosToCoords((coordsToArrayPos(p1X) + 10) % 40);
        }
        else {
            p1X = arrayPosToCoords((coordsToArrayPos(p1X) + 30) % 40);
        }
    }
    else {
        if(coordsToArrayPos(p2Y) == 4) {
        p1X = (p2X + 100) % 400;
        }
        else {
            p2X = (p2X - 100) % 400;
        }
    }
    return snake;
}

/**
 * Method: Search for a free field on the board
 * Return: Position (position) with free field on the board
 */
position * getRandomFreeField() {
    int randx = rand() % 39;
    int randy = rand() % 29;
        
    while(fieldArray[randx][randy] != 0){
        randx = rand() % 39;
        randy = rand() % 29;
    }
        
    position * randomFreeField = newPosition(randx, randy);
        
    return randomFreeField;
    
    randx = randx * 10 + 110;
    randy = randy * 10 + 110;
}

/**
 * Method: Check if Level Colision with Position
 * Parameter: position a is x and position b is y
 * Return: If Position a,b is on Wall return 1, else return 0
 */
int levelCollision(int a, int b){
    if(level==2 && wallBorder==0){
    	if((b==200 && ((a>105 && a<255) || (a>345 && a<500))) || (b==300 && ((a>105 && a<255) || (a>345 && a<500)))){
    		return 1;
    	}
    }

    if(level==3 && wallBorder ==0){
    	if((b==200 && (a>165 && a<435)) || (b==300 && (a>165 && a<435)) ){
    		return 1;
    	}
    	if((a<170 && b<165) || (a<170 && b>335) || (a>435 && b<165) || (a>435 && b>335)){
    		return 1;
    	}
    }

	return 0;
}

/**
 * Method: Update player1/2 snake1/2 for each Step
 * Parameter: snake is snake1/2 form player1/2, player is player1/2
 * Return: Updated list (node) of snake1/2
 */
node * playerStep(node * snake, int player){
    
    tempX = coordsToArrayPos(snake->tail->data->x);
    tempY = coordsToArrayPos(snake->tail->data->y);
    if(fieldArray[tempX][tempY]==player) {
        fieldArray[tempX][tempY] = 0;
    }
    
    if(player == 1) {
        tempX = p1X;
        tempY = p1Y;
        tempDirection = p1Direction;
        tempNextDirection = p1NextDirection;
    }
    else {
        tempX = p2X;
        tempY = p2Y;
        tempDirection = p2Direction;
        tempNextDirection = p2NextDirection;
    }
    if(fieldArray[coordsToArrayPos(tempX)][coordsToArrayPos(tempY)]!=20) {
        fieldArray[coordsToArrayPos(tempX)][coordsToArrayPos(tempY)] = player;
    }
    
    snake = removeLast(snake);
    snake = snake -> head;
    
    tempDirection = tempNextDirection;
    if(tempDirection == 'D') {
        tempY = tempY+10;               //Y-Achse invertierte
    }
    else if(tempDirection == 'L') {
        tempX = tempX-10;
    }
    else if(tempDirection == 'U') {
        tempY = tempY-10;               //Y-Achse invertiert
    }
    else if(tempDirection == 'R') {
        tempX = tempX+10;
    }
    
    if(level == 5) {
        if(tempX<105 || tempX>495) {
            if(!multiplayer) {
                gameLost = true;
            }
            else if(player == 1) {
                winnerPlayer = 2;
            }
            else if(winnerPlayer == 2) {
                winnerPlayer = 0;
                gameLost = true;
            }
            else {
                winnerPlayer = 1;
            }
        }
    }
    
    if(wallBorder == 0){
        if(tempX<105 || tempX>495 || tempY<105 || tempY>395){
            if(!multiplayer) {
                gameLost = true;
            }
            else if(player == 1) {
                winnerPlayer = 2;
            }
            else if(winnerPlayer == 2) {
                winnerPlayer = 0;
                gameLost = true;
            }
            else {
                winnerPlayer = 1;
            }
        }
    }
    if(wallBorder == 1){
        if(tempX<105){
            tempX=490;
        }
        else if(tempX>495){
            tempX=110;
        }
        else if(tempY<105){
            tempY=390;
        }
        else if(tempY>395){
            tempY=110;
        }
    }
    
    position * sec = newPosition(tempX, tempY);
    snake = addAsFirst(snake, sec);
    
    if(player == 1) {
        p1X = tempX;
        p1Y = tempY;
        p1Direction = tempDirection;
        p1NextDirection = tempNextDirection;
    }
    else {
        p2X = tempX;
        p2Y = tempY;
        p2Direction = tempDirection;
        p2NextDirection = tempNextDirection;
    }
    
    return snake;
}

/**
 * Method: Draw Logo on settingScreen
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void settings(int mousePosX, int mousePosY){
	vDrawLogo(mousePosX,mousePosY);
}

/**
 * Method: Check if Mouse is Over certain place x,y until x+35,y+35
 * Parameter: x is the x value of the certain place
 * 				y is the y value of the certain place
 * 				mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 * Return: 0 if mouse was not over
 * 			1 if mouse was over
 */
int mouseOver(int x, int y, int mousePosX, int mousePosY){
	if(mousePosX >= x && mousePosY >= y && mousePosX <= (x+35) && mousePosY <= (y+35)){
		return 1;
	}
	return 0;
}

/**
 * Method: draw KeyBoard for Playername entering
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 * 				multiplayer if multiplayer is activate (diffrent screen)
 */
void drawKeyBoard(int mousePosX,int mousePosY,int multiplayer){
	//Top row of Keyboard
	if(mouseOver(125,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(125,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'Q';
					}
					else{
						p1Name[p1Letter] = 'q';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'Q';
					}
					else{
						p2Name[p2Letter] = 'q';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(125,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(165,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(165,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'W';
					}
					else{
						p1Name[p1Letter] = 'w';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'W';
					}
					else{
						p2Name[p2Letter] = 'w';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(165,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(205,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(205,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'E';
					}
					else{
						p1Name[p1Letter] = 'e';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'E';
					}
					else{
						p2Name[p2Letter] = 'e';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(205,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(245,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(245,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'R';
					}
					else{
						p1Name[p1Letter] = 'r';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'R';
					}
					else{
						p2Name[p2Letter] = 'r';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(245,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(285,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(285,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'T';
					}
					else{
						p1Name[p1Letter] = 't';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'T';
					}
					else{
						p2Name[p2Letter] = 't';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(285,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(325,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(325,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'Y';
					}
					else{
						p1Name[p1Letter] = 'y';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'Y';
					}
					else{
						p2Name[p2Letter] = 'y';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(325,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(365,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(365,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'U';
					}
					else{
						p1Name[p1Letter] = 'u';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'U';
					}
					else{
						p2Name[p2Letter] = 'u';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(365,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(405,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(405,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'I';
					}
					else{
						p1Name[p1Letter] = 'i';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'I';
					}
					else{
						p2Name[p2Letter] = 'i';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(405,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(445,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(445,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'O';
					}
					else{
						p1Name[p1Letter] = 'o';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'O';
					}
					else{
						p2Name[p2Letter] = 'o';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(445,140,35,35,White),__FUNCTION__);
	}
	if(mouseOver(485,140,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(485,140,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'P';
					}
					else{
						p1Name[p1Letter] = 'p';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'P';
					}
					else{
						p2Name[p2Letter] = 'p';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(485,140,35,35,White),__FUNCTION__);
	}

	//Mid row of Keyboard
	if(mouseOver(142,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(142,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'A';
					}
					else{
						p1Name[p1Letter] = 'a';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'A';
					}
					else{
						p2Name[p2Letter] = 'a';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(142,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(182,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(182,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'S';
					}
					else{
						p1Name[p1Letter] = 's';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'S';
					}
					else{
						p2Name[p2Letter] = 's';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(182,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(222,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(222,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'D';
					}
					else{
						p1Name[p1Letter] = 'd';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'D';
					}
					else{
						p2Name[p2Letter] = 'd';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(222,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(262,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(262,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'F';
					}
					else{
						p1Name[p1Letter] = 'f';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'F';
					}
					else{
						p2Name[p2Letter] = 'f';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(262,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(302,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(302,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'G';
					}
					else{
						p1Name[p1Letter] = 'g';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'G';
					}
					else{
						p2Name[p2Letter] = 'g';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(302,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(342,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(342,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'H';
					}
					else{
						p1Name[p1Letter] = 'h';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'H';
					}
					else{
						p2Name[p2Letter] = 'h';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(342,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(382,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(382,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'J';
					}
					else{
						p1Name[p1Letter] = 'j';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'J';
					}
					else{
						p2Name[p2Letter] = 'j';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(382,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(422,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(422,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'K';
					}
					else{
						p1Name[p1Letter] = 'k';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'K';
					}
					else{
						p2Name[p2Letter] = 'k';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(422,180,35,35,White),__FUNCTION__);
	}
	if(mouseOver(462,180,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(462,180,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'L';
					}
					else{
						p1Name[p1Letter] = 'l';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'L';
					}
					else{
						p2Name[p2Letter] = 'l';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(462,180,35,35,White),__FUNCTION__);
	}

	//Last row of Keyboard
	if(mouseOver(142,220,mousePosX,mousePosY)){
		checkDraw(tumDrawLoadedImage(capsLock_hover_image,142,220),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			capsPressed=1;
		}
	}else{
		checkDraw(tumDrawLoadedImage(capsLock_image,142,220),__FUNCTION__);
	}
	if(mouseOver(182,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(182,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'Z';
					}
					else{
						p1Name[p1Letter] = 'z';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'Z';
					}
					else{
						p2Name[p2Letter] = 'z';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(182,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(222,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(222,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'X';
					}
					else{
						p1Name[p1Letter] = 'x';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'X';
					}
					else{
						p2Name[p2Letter] = 'x';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(222,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(262,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(262,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'C';
					}
					else{
						p1Name[p1Letter] = 'c';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'C';
					}
					else{
						p2Name[p2Letter] = 'c';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(262,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(302,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(302,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'V';
					}
					else{
						p1Name[p1Letter] = 'v';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'V';
					}
					else{
						p2Name[p2Letter] = 'v';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(302,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(342,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(342,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'B';
					}
					else{
						p1Name[p1Letter] = 'b';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'B';
					}
					else{
						p2Name[p2Letter] = 'b';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(342,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(382,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(382,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'N';
					}
					else{
						p1Name[p1Letter] = 'n';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'N';
					}
					else{
						p2Name[p2Letter] = 'n';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(382,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(422,220,mousePosX,mousePosY)){
		checkDraw(tumDrawFilledBox(422,220,35,35,Grey),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'M';
					}
					else{
						p1Name[p1Letter] = 'm';
					}
			}
			else{
					if(caps){
						p2Name[p2Letter] = 'M';
					}
					else{
						p2Name[p2Letter] = 'm';
					}
			}
		}
	}else{
		checkDraw(tumDrawFilledBox(422,220,35,35,White),__FUNCTION__);
	}
	if(mouseOver(462,220,mousePosX,mousePosY)){
		checkDraw(tumDrawLoadedImage(backSpace_hover_image,462,220),__FUNCTION__);
		if(tumEventGetMouseLeft()){
			if(p1Selected){
				deleteP1Name=1;
			}
			else{
				deleteP2Name=1;
			}
		}
	}else{
		checkDraw(tumDrawLoadedImage(backSpace_image,462,220),__FUNCTION__);
	}

	//KEYBOARDINPUT
	if(buttons.buttons[KEYCODE(Q)]==1){
				buttons.buttons[KEYCODE(Q)]=0;
				if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'Q';
					}
					else{
						p1Name[p1Letter] = 'q';
					}
				}
				else{
					if(caps){
						p2Name[p2Letter] = 'Q';
					}
					else{
						p2Name[p2Letter] = 'q';
					}
				}
			}
	if(buttons.buttons[KEYCODE(W)]==1){
			buttons.buttons[KEYCODE(W)]=0;
			if(p1Selected){
				if(caps){
					p1Name[p1Letter] = 'W';
				}
				else{
					p1Name[p1Letter] = 'w';
				}
			}
			else{
				if(caps){
					p2Name[p2Letter] = 'W';
				}
				else{
					p2Name[p2Letter] = 'w';
				}
			}
		}
	if(buttons.buttons[KEYCODE(E)]==1){
				buttons.buttons[KEYCODE(E)]=0;
				if(p1Selected){
					if(caps){
						p1Name[p1Letter] = 'E';
					}
					else{
						p1Name[p1Letter] = 'e';
					}
				}
				else{
					if(caps){
						p2Name[p2Letter] = 'E';
					}
					else{
						p2Name[p2Letter] = 'e';
					}
				}
			}
	if(buttons.buttons[KEYCODE(R)]==1){
					buttons.buttons[KEYCODE(R)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'R';
						}
						else{
							p1Name[p1Letter] = 'r';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'R';
						}
						else{
							p2Name[p2Letter] = 'r';
						}
					}
				}
	if(buttons.buttons[KEYCODE(T)]==1){
					buttons.buttons[KEYCODE(T)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'T';
						}
						else{
							p1Name[p1Letter] = 't';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'T';
						}
						else{
							p2Name[p2Letter] = 't';
						}
					}
				}
	if(buttons.buttons[KEYCODE(Y)]==1){
					buttons.buttons[KEYCODE(Y)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'Y';
						}
						else{
							p1Name[p1Letter] = 'y';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'Y';
						}
						else{
							p2Name[p2Letter] = 'y';
						}
					}
				}
	if(buttons.buttons[KEYCODE(U)]==1){
					buttons.buttons[KEYCODE(U)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'U';
						}
						else{
							p1Name[p1Letter] = 'u';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'U';
						}
						else{
							p2Name[p2Letter] = 'u';
						}
					}
				}
	if(buttons.buttons[KEYCODE(I)]==1){
					buttons.buttons[KEYCODE(I)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'I';
						}
						else{
							p1Name[p1Letter] = 'i';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'I';
						}
						else{
							p2Name[p2Letter] = 'i';
						}
					}
				}
	if(buttons.buttons[KEYCODE(O)]==1){
					buttons.buttons[KEYCODE(O)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'O';
						}
						else{
							p1Name[p1Letter] = 'o';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'O';
						}
						else{
							p2Name[p2Letter] = 'o';
						}
					}
				}
	if(buttons.buttons[KEYCODE(P)]==1){
					buttons.buttons[KEYCODE(P)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'P';
						}
						else{
							p1Name[p1Letter] = 'p';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'P';
						}
						else{
							p2Name[p2Letter] = 'p';
						}
					}
				}
	if(buttons.buttons[KEYCODE(A)]==1){
					buttons.buttons[KEYCODE(A)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'A';
						}
						else{
							p1Name[p1Letter] = 'a';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'A';
						}
						else{
							p2Name[p2Letter] = 'a';
						}
					}
				}
	if(buttons.buttons[KEYCODE(S)]==1){
					buttons.buttons[KEYCODE(S)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'S';
						}
						else{
							p1Name[p1Letter] = 's';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'S';
						}
						else{
							p2Name[p2Letter] = 's';
						}
					}
				}
	if(buttons.buttons[KEYCODE(D)]==1){
					buttons.buttons[KEYCODE(D)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'D';
						}
						else{
							p1Name[p1Letter] = 'd';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'D';
						}
						else{
							p2Name[p2Letter] = 'd';
						}
					}
				}
	if(buttons.buttons[KEYCODE(F)]==1){
					buttons.buttons[KEYCODE(F)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'F';
						}
						else{
							p1Name[p1Letter] = 'f';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'F';
						}
						else{
							p2Name[p2Letter] = 'f';
						}
					}
				}
	if(buttons.buttons[KEYCODE(G)]==1){
					buttons.buttons[KEYCODE(G)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'G';
						}
						else{
							p1Name[p1Letter] = 'g';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'G';
						}
						else{
							p2Name[p2Letter] = 'g';
						}
					}
				}
	if(buttons.buttons[KEYCODE(H)]==1){
					buttons.buttons[KEYCODE(H)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'H';
						}
						else{
							p1Name[p1Letter] = 'h';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'H';
						}
						else{
							p2Name[p2Letter] = 'h';
						}
					}
				}
	if(buttons.buttons[KEYCODE(J)]==1){
					buttons.buttons[KEYCODE(J)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'J';
						}
						else{
							p1Name[p1Letter] = 'j';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'J';
						}
						else{
							p2Name[p2Letter] = 'j';
						}
					}
				}
	if(buttons.buttons[KEYCODE(K)]==1){
					buttons.buttons[KEYCODE(K)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'K';
						}
						else{
							p1Name[p1Letter] = 'k';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'K';
						}
						else{
							p2Name[p2Letter] = 'k';
						}
					}
				}
	if(buttons.buttons[KEYCODE(L)]==1){
					buttons.buttons[KEYCODE(L)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'L';
						}
						else{
							p1Name[p1Letter] = 'l';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'L';
						}
						else{
							p2Name[p2Letter] = 'l';
						}
					}
				}
	if(buttons.buttons[KEYCODE(Z)]==1){
					buttons.buttons[KEYCODE(Z)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'Z';
						}
						else{
							p1Name[p1Letter] = 'z';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'Z';
						}
						else{
							p2Name[p2Letter] = 'z';
						}
					}
				}
	if(buttons.buttons[KEYCODE(X)]==1){
					buttons.buttons[KEYCODE(X)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'X';
						}
						else{
							p1Name[p1Letter] = 'x';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'X';
						}
						else{
							p2Name[p2Letter] = 'x';
						}
					}
				}
	if(buttons.buttons[KEYCODE(C)]==1){
					buttons.buttons[KEYCODE(C)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'C';
						}
						else{
							p1Name[p1Letter] = 'c';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'C';
						}
						else{
							p2Name[p2Letter] = 'c';
						}
					}
				}
	if(buttons.buttons[KEYCODE(V)]==1){
					buttons.buttons[KEYCODE(V)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'V';
						}
						else{
							p1Name[p1Letter] = 'v';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'V';
						}
						else{
							p2Name[p2Letter] = 'v';
						}
					}
				}
	if(buttons.buttons[KEYCODE(B)]==1){
					buttons.buttons[KEYCODE(B)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'B';
						}
						else{
							p1Name[p1Letter] = 'b';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'B';
						}
						else{
							p2Name[p2Letter] = 'b';
						}
					}
				}
	if(buttons.buttons[KEYCODE(N)]==1){
					buttons.buttons[KEYCODE(N)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'N';
						}
						else{
							p1Name[p1Letter] = 'n';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'N';
						}
						else{
							p2Name[p2Letter] = 'n';
						}
					}
				}
	if(buttons.buttons[KEYCODE(M)]==1){
					buttons.buttons[KEYCODE(M)]=0;
					if(p1Selected){
						if(caps){
							p1Name[p1Letter] = 'M';
						}
						else{
							p1Name[p1Letter] = 'm';
						}
					}
					else{
						if(caps){
							p2Name[p2Letter] = 'M';
						}
						else{
							p2Name[p2Letter] = 'm';
						}
					}
				}
	if(buttons.buttons[KEYCODE(CAPSLOCK)]==1){
		buttons.buttons[KEYCODE(CAPSLOCK)]=0;
		capsPressed=1;
	}
	if(buttons.buttons[KEYCODE(BACKSPACE)]==1){
		buttons.buttons[KEYCODE(BACKSPACE)]=0;
		if(p1Selected){
			deleteP1Name=1;
		}
		else{
			deleteP2Name=1;
		}
	}

	if(caps){
		checkDraw(tumDrawText("Q",135, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("W",175, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("E",215, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("R",255, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("T",295, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("Y",335, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("U",375, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("I",415, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("O",455, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("P",495, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("A",152, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("S",192, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("D",232, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("F",272, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("G",312, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("H",352, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("J",392, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("K",432, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("L",472, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("Z",192, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("X",232, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("C",272, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("V",312, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("B",352, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("N",392, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("M",432, 230, Black),__FUNCTION__);
	}
	else{
		checkDraw(tumDrawText("q",135, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("w",175, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("e",215, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("r",255, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("t",295, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("y",335, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("u",375, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("i",415, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("o",455, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("p",495, 150, Black),__FUNCTION__);
		checkDraw(tumDrawText("a",152, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("s",192, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("d",232, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("f",272, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("g",312, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("h",352, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("j",392, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("k",432, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("l",472, 190, Black),__FUNCTION__);
		checkDraw(tumDrawText("z",192, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("x",232, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("c",272, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("v",312, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("b",352, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("n",392, 230, Black),__FUNCTION__);
		checkDraw(tumDrawText("m",432, 230, Black),__FUNCTION__);
	}

	if(p1Name[p1Letter] != 0 && !tumEventGetMouseLeft()){
		p1Letter++;
	}

	if(p2Name[p2Letter] != 0 && !tumEventGetMouseLeft()){
			p2Letter++;
	}

	if(deleteP1Name && !tumEventGetMouseLeft()){
		if(p1Letter>0){
			p1Letter--;
		}
		p1Name[p1Letter]=0;
		deleteP1Name=0;
	}

	if(deleteP2Name && !tumEventGetMouseLeft()){
		if(p2Letter>0){
			p2Letter--;
		}
		p2Name[p2Letter]=0;
		deleteP2Name=0;
	}

	if(capsPressed && !tumEventGetMouseLeft()){
		caps = !caps;
		capsPressed=0;
	}

}

/**
 * Method: Entering player Name in the Box
 * 			if Box is empty Spieler1 and Spieler2 will automaticly set
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void enterPlayerName(int mousePosX, int mousePosY){
	static char str[100] = { 0 };
	static char name [20] = { 0 };
	static char name1 [20] = { 0 };
	static char name2 [20] = { 0 };
	sprintf(name,"Spieler Name:");
	sprintf(name1,"Spieler1 Name:");
	sprintf(name2,"Spieler2 Name:");
	sprintf(str,"Start Game");

	if(gameStarted){
		checkDraw(tumDrawClear(Black), __FUNCTION__);
		if(!multiplayer){
			checkDraw(tumDrawText(name,150, 100, White),__FUNCTION__);
			checkDraw(tumDrawFilledBox(275,105,200,15,White),__FUNCTION__);
			checkDraw(tumDrawText(p1Name,275, 103,Red),__FUNCTION__);
		}
		else{
			checkDraw(tumDrawText(name1,150, 50, White),__FUNCTION__);
			checkDraw(tumDrawText(name2,150, 100, White),__FUNCTION__);
			if(p1Selected){
				checkDraw(tumDrawFilledBox(275,105,200,15,White),__FUNCTION__);
				checkDraw(tumDrawFilledBox(275,55,200,15,Grey),__FUNCTION__);
			}
			else{
				checkDraw(tumDrawFilledBox(275,55,200,15,White),__FUNCTION__);
				checkDraw(tumDrawFilledBox(275,105,200,15,Grey),__FUNCTION__);
			}
			checkDraw(tumDrawText(p1Name,275, 53,Red),__FUNCTION__);
			checkDraw(tumDrawText(p2Name,275, 103,Red),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				if(mousePosX >= 275 && mousePosX <= 475 && mousePosY >= 55 && mousePosY <= 70){
					p1Selected = 1;
				}
				else if(mousePosX >= 275 && mousePosX <= 475 && mousePosY >= 105 && mousePosY <= 120){
					p1Selected = 0;
				}
			}
		}
		drawKeyBoard(mousePosX,mousePosY,multiplayer);
		if(mousePosX >= 150 && mousePosX <= 500 && mousePosY >= 350 && mousePosY <= 430){
			checkDraw(tumDrawFilledBox(150,350,350,80,Grey),__FUNCTION__);
			checkDraw(tumDrawFilledBox(155,355,340,70,Black),__FUNCTION__);
			checkDraw(tumDrawText(str,270,380, Grey),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				gameStarted = 0;
				if(p1Name[0] == 0){
					sprintf(p1Name,"Spieler1");
				}
				if(multiplayer){
					if(p2Name[0] == 0){
						sprintf(p2Name,"Spieler2");
					}
				}
				currentState++;
			}
		}
		else{
			checkDraw(tumDrawFilledBox(150,350,350,80,White),__FUNCTION__);
			checkDraw(tumDrawFilledBox(155,355,340,70,Black),__FUNCTION__);
			checkDraw(tumDrawText(str,270,380, White),__FUNCTION__);
		}
	}
}

/**
 * Method: Drawing Logo on Playername screen
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void vMainMenuDrawLogo(int mousePosX, int mousePosY){
	static int image_width;
	if((image_width = tumDrawGetLoadedImageWidth(home_image)) != -1 && tumDrawGetLoadedImageWidth(home_image2) != -1){
		if(mousePosX >= 10 && mousePosX <= 60 && mousePosY >= 10 && mousePosY <= 60){
			checkDraw(tumDrawLoadedImage(home_image2, 10, 10),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				gameStarted=0;
			}
		}
		else{
			checkDraw(tumDrawLoadedImage(home_image, 10 , 10),__FUNCTION__);
		}
	}
	else{
		printf("LOAD PIC FAILED!!");
	}
}

/**
 * Method: Show the Main Menu
 * Parameter: ? (something for the RTOS)
 */
void vMainMenu(void *pvParameters)
{
    while (1) {
        if (DrawSignal)
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
                pdTRUE) {
                xGetButtonInput();
                // Update global input
                
                xSemaphoreTake(ScreenLock, portMAX_DELAY);

                // Clear screen
                checkDraw(tumDrawClear(Black), __FUNCTION__);
                
                static unsigned int mousePosX, mousePosY;
                mousePosX = tumEventGetMouseX();
                mousePosY = tumEventGetMouseY();
				if(settingsClicked){
					settings(mousePosX,mousePosY);
				}
				else if(gameStarted){
					enterPlayerName(mousePosX,mousePosY);
					 vMainMenuDrawLogo(mousePosX,mousePosY);
				}
				else{
					static char str1[100] = { 0 };
					sprintf(str1,"One Player Game");
					static char str2[100] = { 0 };
					sprintf(str2,"Two Player Game");
					static char str3[100] = { 0 };
					sprintf(str3,"Highscore");
                    
                    
					
					if(mousePosX <= 40 && mousePosY <= 40){
                        
                        checkDraw(tumDrawFilledBox(5, 5, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(10, 5, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(15, 5, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(20, 5, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(25, 5, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 5, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 10, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 15, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 18, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(25, 18, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(20, 18, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(18, 18, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(18, 23, 5, 5, Grey), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(18, 38, 5, 5, Grey), __FUNCTION__);
                        
                        static char cont[100] = { 0 };
                        sprintf(cont, "Controls during the game:");
                        static char pCont1[100] = { 0 };
                        sprintf(pCont1, "Player One : W,A,S,D, Spacebar for use of PowerUp");
                        static char pCont2[100] = { 0 };
                        sprintf(pCont2, "Player Two : Arrow keys, right shift for use of PowerUp");
                        static char cont1[100] = { 0 };
                        sprintf(cont1, "Press 'P' for pause");
                        static char cont2[100] = { 0 };
                        sprintf(cont2, "Press 'I' for infoscreen");
                        static char cont3[100] = { 0 };
                        sprintf(cont3, "Press 'Esc' for exit");
                        
                        static char levelitem[100]= { 0 };
                        sprintf(levelitem, "Level Item (cannot picked up):");
                        static char levelitem1[100]= { 0 };
                        sprintf(levelitem1, "Teleport: Teleport the player from one Teleport Element to another");


                        static char items[100] = { 0 };
                        sprintf(items, "Items:");
                        static char items1[100] = { 0 };
                        sprintf(items1, "Food : Increses the size of the player by one");
                        static char items2[100] = { 0 };
                        sprintf(items2, "Superfood : Increases the size of the player by two");
                        static char items3[100] = { 0 };
                        sprintf(items3, "Freeze : Freeze the opponent for a short period of time");
                        static char items4[100] = { 0 };
                        sprintf(items4, "Reduce : Reduce the size of the opponent by one");
                        static char items5[100] = { 0 };
                        sprintf(items5, "Inverse : Reverse the controls of the opponent for a short period of time");
                        
                        
                        checkDraw(tumDrawText(cont,50,50, White),__FUNCTION__);
                        checkDraw(tumDrawText(pCont1,50,80, White),__FUNCTION__);
                        checkDraw(tumDrawText(pCont2,50,110, White),__FUNCTION__);
                        checkDraw(tumDrawText(cont1,50,140, White),__FUNCTION__);
                        checkDraw(tumDrawText(cont2,50,170, White),__FUNCTION__);
                        checkDraw(tumDrawText(cont3,50,200, White),__FUNCTION__);
                        
                        checkDraw(tumDrawText(levelitem,50,230,White),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(50, 268, 5, 5, Purple), __FUNCTION__);
                        checkDraw(tumDrawText(levelitem1,70,260,White),__FUNCTION__);

                        checkDraw(tumDrawText(items,50,290, White),__FUNCTION__);
                        
                        checkDraw(tumDrawFilledBox(50, 328, 5, 5, Green), __FUNCTION__);
                        checkDraw(tumDrawText(items1, 70, 320, White),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(50, 358, 10, 10, Green), __FUNCTION__);
                        checkDraw(tumDrawText(items2, 70, 350, White),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(50, 388, 5, 5, Cyan), __FUNCTION__);
                        checkDraw(tumDrawText(items3, 70, 380, White),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(50, 418, 5, 5, Yellow), __FUNCTION__);
                        checkDraw(tumDrawText(items4, 70, 410, White),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(50, 448, 5, 5, Gray), __FUNCTION__);
                        checkDraw(tumDrawText(items5, 70, 440, White),__FUNCTION__);
                            
                    }
                    else {
                        
                        checkDraw(tumDrawFilledBox(5, 5, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(10, 5, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(15, 5, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(20, 5, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(25, 5, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 5, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 10, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 15, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(30, 18, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(25, 18, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(20, 18, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(18, 18, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(18, 23, 5, 5, White), __FUNCTION__);
                        checkDraw(tumDrawFilledBox(18, 38, 5, 5, White), __FUNCTION__);
                        
                        if(mousePosX >= 150 && mousePosX <= 500 && mousePosY >= 90 && mousePosY <= 160){
                        checkDraw(tumDrawFilledBox(150,90,350,80,Grey),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(155,95,340,70,Black),__FUNCTION__);
                        checkDraw(tumDrawText(str1,270,120, Grey),__FUNCTION__);
                            if(tumEventGetMouseLeft()){
                                multiplayer = false;
                                twoFoodElements=0;
                                gameStarted = 1;
                            }
                        }
                        else{
                        checkDraw(tumDrawFilledBox(150,90,350,80,White),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(155,95,340,70,Black),__FUNCTION__);
                        checkDraw(tumDrawText(str1,270,120, White),__FUNCTION__);
                        }	
                            
                        if(mousePosX >= 150 && mousePosX <= 500 && mousePosY >= 190 && mousePosY <= 260){
                        checkDraw(tumDrawFilledBox(150,190,350,80,Grey),__FUNCTION__);
                        checkDraw(tumDrawFilledBox(155,195,340,70,Black),__FUNCTION__);
                        checkDraw(tumDrawText(str2,270,220, Grey),__FUNCTION__);
                            if(tumEventGetMouseLeft()){
                                multiplayer = true;
                                twoFoodElements=1;
                                gameStarted = 1;
                            }
                        }
                        else{
                            checkDraw(tumDrawFilledBox(150,190,350,80,White),__FUNCTION__);
                            checkDraw(tumDrawFilledBox(155,195,340,70,Black),__FUNCTION__);
                            checkDraw(tumDrawText(str2,270,220, White),__FUNCTION__);
                        }

                        if(mousePosX >= 150 && mousePosX <= 500 && mousePosY >= 290 && mousePosY <= 360){
                            checkDraw(tumDrawFilledBox(150,290,350,80,Grey),__FUNCTION__);
                            checkDraw(tumDrawFilledBox(155,295,340,70,Black),__FUNCTION__);
                            checkDraw(tumDrawText(str3,270,320, Grey),__FUNCTION__);
                            if(tumEventGetMouseLeft()){
                                currentState--;
                            }
                        }
                        else{
                            checkDraw(tumDrawFilledBox(150,290,350,80,White),__FUNCTION__);
                            checkDraw(tumDrawFilledBox(155,295,340,70,Black),__FUNCTION__);
                            checkDraw(tumDrawText(str3,270,320, White),__FUNCTION__);
                        }
        
                        vDrawLogo(mousePosX,mousePosY);
                    }
				}
				if(tumEventGetMouseLeft()){
					checkDraw(tumDrawCircle(tumEventGetMouseX(),tumEventGetMouseY(), 10, Grey), __FUNCTION__);
				}
				else{ 
					checkDraw(tumDrawCircle(tumEventGetMouseX(), tumEventGetMouseY(), 10, White), __FUNCTION__);
				}

                xSemaphoreGive(ScreenLock);

                // Get input and check for state change
                vCheckStateInput();
    	}
   	}
}

/**
 * Method: Drawing Logos on Top left corner
 * Parameter: mousePosX position of Mouse X value, mousePosY position of Mouse Y value
 */
void vHighscoreScreenDrawLogo(int mousePosX, int mousePosY){
	static int image_width;
	if((image_width = tumDrawGetLoadedImageWidth(home_image)) != -1 && tumDrawGetLoadedImageWidth(home_image2) != -1){
		if(mousePosX >= 10 && mousePosX <= 60 && mousePosY >= 10 && mousePosY <= 60){
			checkDraw(tumDrawLoadedImage(home_image2, 10, 10),__FUNCTION__);
			if(tumEventGetMouseLeft()){
				currentState++;
			}
		}
		else{
			checkDraw(tumDrawLoadedImage(home_image, 10 , 10),__FUNCTION__);
		}
	}
	else{
		printf("LOAD PIC FAILED!!");
	}
}

/**
 * Method: import Highscore.txt file
 * Return: 1 if file succesfull read
 * 			0 if file not succesfull red
 */
int readFile(){
	char* filename="../files/Highscore.txt";
	file=fopen(filename,"r");

	if(! file){
		char* filename="../files/Highscore.txt";
		file=fopen(filename,"r");
		if(! file){
			printf("Could not open file %s\n",filename);
			return 1;
		}
	}
	return 0;
}

/**
 * Method: Screen for Highscore
 * Parameter: ? (something for RTOS)
 */
void vHighscoreScreen(void *pvParameters){
	checkDraw(tumDrawClear(Black), __FUNCTION__);
	int h[10];
	int m[10];
	int s[10];
	char name[10][20];
	int p[10];
	char levels[10][20];
	char walls[10][20];
	while (1) {
	        if (DrawSignal){
	        	if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==pdTRUE) {
	        		checkDraw(tumDrawClear(Black), __FUNCTION__);

	        		update=0;
	        		entered=0;
	        		entered2=0;

	        		static unsigned int mousePosX, mousePosY;
	        		mousePosX = tumEventGetMouseX();
	        		mousePosY = tumEventGetMouseY();
	        		static char str1[100];
        			char str[100];

	        		vHighscoreScreenDrawLogo(mousePosX,mousePosY);

	        		//First Intiailisation Reading File and printf first time on screen
	        		if(readHighscoreFile){
		        		readFile();
	        			for(int i=0;i<10;i++){
	        				sscanf(fgets(str,50,file),"%d:%d:%d %s %d %s %s",&h[i],&m[i],&s[i],&name[i][0],&p[i],&levels[i][0],&walls[i][0]);
	        			}

	        			for(int i=0;i<10;i++){
	        				sprintf(str1,"%s",name[i]);
	        				checkDraw(tumDrawText(str1,75,150+(20*i), White),__FUNCTION__);
	        				sprintf(str1,"%.2d:%.2d:%.2d",h[i],m[i],s[i]);
	        				checkDraw(tumDrawText(str1,225,150+(20*i), White),__FUNCTION__);
	        				sprintf(str1,"%d",p[i]);
	        				checkDraw(tumDrawText(str1,375,150+(20*i), White),__FUNCTION__);
	        				sprintf(str1,"%s %s",levels[i],walls[i]);
	        				checkDraw(tumDrawText(str1,505,150+(20*i), White),__FUNCTION__);
	        			}
	        			readHighscoreFile=0;
	        		}

	        		//Time Sort ascending descending
	        		if(!pointSort){
	        			for(int i=0;i<10;i++){
	        				sprintf(str1,"%s",name[i]);
	        				checkDraw(tumDrawText(str1,75,150+(20*i), White),__FUNCTION__);
	        				sprintf(str1,"%.2d:%.2d:%.2d",h[i],m[i],s[i]);
	        				checkDraw(tumDrawText(str1,225,150+(20*i), White),__FUNCTION__);
	        				sprintf(str1,"%d",p[i]);
	        				checkDraw(tumDrawText(str1,375,150+(20*i), White),__FUNCTION__);
	        				sprintf(str1,"%s %s",levels[i],walls[i]);
	        				checkDraw(tumDrawText(str1,505,150+(20*i), White),__FUNCTION__);
	        			}

	        		}else{
	        			int j=9;
	        			for(int i=9;i>=0;i--){
	        				sprintf(str1,"%s",name[i]);
	        				checkDraw(tumDrawText(str1,75,150+(20*(i-j)), White),__FUNCTION__);
	        				sprintf(str1,"%.2d:%.2d:%.2d",h[i],m[i],s[i]);
	        				checkDraw(tumDrawText(str1,225,150+(20*(i-j)), White),__FUNCTION__);
	        				sprintf(str1,"%d",p[i]);
	        				checkDraw(tumDrawText(str1,375,150+(20*(i-j)), White),__FUNCTION__);
	        				sprintf(str1,"%s %s",levels[i],walls[i]);
	        				checkDraw(tumDrawText(str1,505,150+(20*(i-j)), White),__FUNCTION__);
	        				j-=2;
	        			}
	        		}



	        		sprintf(str1,"Name");
	        		checkDraw(tumDrawFilledBox(20,100,150,50,White),__FUNCTION__);
	        		checkDraw(tumDrawFilledBox(25,105,140,40,Black),__FUNCTION__);
	        		checkDraw(tumDrawText(str1,75,115, White),__FUNCTION__);

	        		sprintf(str1,"Time");
	        		checkDraw(tumDrawFilledBox(170,100,150,50,White),__FUNCTION__);
	        		checkDraw(tumDrawFilledBox(175,105,140,40,Black),__FUNCTION__);
	        		checkDraw(tumDrawText(str1,225,115, White),__FUNCTION__);

	        		sprintf(str1,"Punkte");
	        		if(mousePosX >=320  && mousePosX <=470  && mousePosY >=100  && mousePosY <=150 ){
	        			checkDraw(tumDrawFilledBox(320,100,150,50,Grey),__FUNCTION__);
	        			checkDraw(tumDrawFilledBox(325,105,140,40,Black),__FUNCTION__);
	        			checkDraw(tumDrawText(str1,375,115, Grey),__FUNCTION__);
	        			//Button Point Clicked
	        			if(tumEventGetMouseLeft()){
	        				pointButtonClicked=1;
	        			}
	        			else{
	        				if(pointButtonClicked){
	        					pointSort=(pointSort+1)%2;
	        					pointButtonClicked=0;
	        				}
	        			}
	        		}
	        		else{
	        			checkDraw(tumDrawFilledBox(320,100,150,50,White),__FUNCTION__);
	        			checkDraw(tumDrawFilledBox(325,105,140,40,Black),__FUNCTION__);
	        			checkDraw(tumDrawText(str1,375,115, White),__FUNCTION__);
	        		}

	        		sprintf(str1,"Level");
	        		checkDraw(tumDrawFilledBox(470,100,150,50,White),__FUNCTION__);
	        		checkDraw(tumDrawFilledBox(475,105,140,40,Black),__FUNCTION__);
	        		checkDraw(tumDrawText(str1,525,115, White),__FUNCTION__);



	        		xGetButtonInput();
	        		xSemaphoreGive(ScreenLock);
	        		vCheckStateInput();
	        	}
	        }
	        if(tumEventGetMouseLeft()){
	        	checkDraw(tumDrawCircle(tumEventGetMouseX(),tumEventGetMouseY(), 10, Grey), __FUNCTION__);
	        }
	        else{
	        	checkDraw(tumDrawCircle(tumEventGetMouseX(), tumEventGetMouseY(), 10, White), __FUNCTION__);
	        }
	}
}

//Not Used, Sound for Ball Collision old RTOS Emulator
/*
void playBallSound(void *args)
{
    tumSoundPlaySample(a3);
}
*/


#define PRINT_TASK_ERROR(task) PRINT_ERROR("Failed to print task ##task");

int main(int argc, char *argv[])
{
	char* filename="../files/Highscore.txt";
	file=fopen(filename,"r");

	if(! file){
        char* filename="../files/Highscore.txt";
        file=fopen(filename,"r");
        if(! file){
            printf("Could not open file %s\n",filename);
            return 1;
        }
	}

    //setting Path Folder for Pictures and etc.
	char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);

    //printf("Initializing: ");

    //Initialze Drawings and initialize Programm
    if (tumDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }

    // Initialize Mouse
    if (tumEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    //load images
    logo_image = tumDrawLoadImage(LOGO_FILENAME);
    logo_image2 = tumDrawLoadImage(LOGO_FILENAME2);
    home_image = tumDrawLoadImage(HOME_FILENAME);
    home_image2 = tumDrawLoadImage(HOME_FILENAME2);
    backSpace_image = tumDrawLoadImage(BACKSPACE);
    backSpace_hover_image = tumDrawLoadImage(BACKSPACE_HOVER);
    capsLock_image = tumDrawLoadImage(CAPS_LOCK);
    capsLock_hover_image = tumDrawLoadImage(CAPS_LOCK_HOVER);

    //Return 0 when successful terminated, else return 1
    atexit(aIODeinit);

    //Load a second font for fun and show FPS Rate
    //tumFontLoadFont(FPS_FONT, DEFAULT_FONT_SIZE);

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_buttons_lock;
    }

    DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
    if (!DrawSignal) {
        PRINT_ERROR("Failed to create draw signal");
        goto err_draw_signal;
    }
    ScreenLock = xSemaphoreCreateMutex();
    if (!ScreenLock) {
        PRINT_ERROR("Failed to create screen lock");
        goto err_screen_lock;
    }

    // Message sending
    StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));
    if (!StateQueue) {
        PRINT_ERROR("Could not open state queue");
        goto err_state_queue;
    }

    if (xTaskCreate(basicSequentialStateMachine, "StateMachine",
                    mainGENERIC_STACK_SIZE * 2, NULL,
                    configMAX_PRIORITIES - 1, StateMachine) != pdPASS) {
        PRINT_TASK_ERROR("StateMachine");
        goto err_statemachine;
    }
    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES,
                    BufferSwap) != pdPASS) {
        PRINT_TASK_ERROR("BufferSwapTask");
        goto err_bufferswap;
    }

    /** Demo Tasks */
    if (xTaskCreate(vMainMenu, "MainMenu", mainGENERIC_STACK_SIZE * 2,
                    NULL, mainGENERIC_PRIORITY, &MainMenu) != pdPASS) {
        PRINT_TASK_ERROR("MainMenu");
        goto err_demotask1;
    }
    if (xTaskCreate(vGameScreen, "GameScreen", mainGENERIC_STACK_SIZE * 2,
                    NULL, mainGENERIC_PRIORITY, &GameScreen) != pdPASS) {
        PRINT_TASK_ERROR("GameScreen");
        goto err_demotask3;
    }
    if (xTaskCreate(vHighscoreScreen, "HighscoreScreen", mainGENERIC_STACK_SIZE * 2,
    				NULL, mainGENERIC_PRIORITY, &HighscoreScreen) != pdPASS) {
    	PRINT_TASK_ERROR("HighscoreScreen");
    	goto err_demotask2;
    }


    vTaskSuspend(MainMenu);
    vTaskSuspend(HighscoreScreen);
    vTaskSuspend(GameScreen);

    vTaskStartScheduler();

    return EXIT_SUCCESS;

//Error handler
err_demotask3:
    vTaskDelete(MainMenu);
err_demotask2:
	vTaskDelete(MainMenu);
	vTaskDelete(GameScreen);
err_demotask1:
    vTaskDelete(BufferSwap);
err_bufferswap:
    vTaskDelete(StateMachine);
err_statemachine:
    vQueueDelete(StateQueue);
err_state_queue:
    vSemaphoreDelete(StateQueue);
err_screen_lock:
    vSemaphoreDelete(DrawSignal);
err_draw_signal:
    vSemaphoreDelete(buttons.lock);
err_buttons_lock:
    tumSoundExit();
err_init_events:
    tumDrawExit();
err_init_drawing:
    return EXIT_FAILURE;
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}

