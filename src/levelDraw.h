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

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Font.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"

#include "AsyncIO.h"

/**
 * Method: Checking if any Drawing Type is Null or empty, then throw [ERROR]
 * Parameter: status is if error is there, msg what error was it
 */
void checkDraw2(unsigned char status, const char *msg)
{
    if (status) {
        if (msg){
            fprintf(stderr, "[ERROR] %s, %s\n", msg,tumGetErrorMessage());
        }
        else {
            fprintf(stderr, "[ERROR] %s\n", tumGetErrorMessage());
        }
    }

}

/**
 * Method: Initialise Level, as long as intial not changed
 * Parameter: Which level, set Wall true or false
 */

int a=0,b=0,c=0,d=0;
int e=0,f=0,g=0,h=0;

/**
 * Method: Reset Integer a,b,c,d,e,f,g,h for animated drawing
 */
void parameterReset(){
	a=0;
	b=0;
	c=0;
	d=0;
	e=0;
	f=0;
	g=0;
	h=0;
}

int level6field[50][2];

void addInitfield(int x, int y, int i){
	level6field[i][0]=x;
	level6field[i][1]=y;
}
/**
 * Method: Initialise Level
 * Parameter: which level and if wall is acitve or not
 */
void initLevel(int level, int wall, int fieldArray[39][29]){
	if(level==1 && wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
				c+=10;
				}
				else{
					if(d!=310){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}
	}
	if(level==1 && !wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
				c+=5;
				}
				else{
					if(d!=305){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
	}
	if(level==2 && wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
				c+=10;
				}
				else{
					if(d!=310){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}
		if(e!=155){
			checkDraw2(tumDrawFilledBox(100+e,200,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=155){
				checkDraw2(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=155){
					checkDraw2(tumDrawFilledBox(100+g,300,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=155){
						checkDraw2(tumDrawFilledBox(350+h,300,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==2 && !wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
				c+=5;
				}
				else{
					if(d!=305){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(e!=155){
			checkDraw2(tumDrawFilledBox(100+e,200,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=155){
				checkDraw2(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=155){
					checkDraw2(tumDrawFilledBox(100+g,300,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=155){
						checkDraw2(tumDrawFilledBox(350+h,300,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==3 && !wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
				c+=5;
				}
				else{
					if(d!=305){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(e!=270){
			checkDraw2(tumDrawFilledBox(165+e,200,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(165+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		if(d==305 && f!=65){
			checkDraw2(tumDrawFilledBox(100+f,100,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,105,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,110,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,115,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,120,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,125,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,130,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,135,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,140,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,145,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,150,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,155,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,160,5,5,White), __FUNCTION__);

			checkDraw2(tumDrawFilledBox(100+f,400,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,395,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,390,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,385,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,380,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,375,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,370,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,365,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,360,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,355,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,350,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,345,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,340,5,5,White), __FUNCTION__);
			f+=5;
		}
		if(d==305 && g!=65){
			checkDraw2(tumDrawFilledBox(440+g,100,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,105,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,110,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,115,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,120,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,125,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,130,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,135,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,140,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,145,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,150,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,155,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,160,5,5,White), __FUNCTION__);

			checkDraw2(tumDrawFilledBox(440+g,400,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,395,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,390,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,385,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,380,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,375,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,370,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,365,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,360,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,355,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,350,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,345,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,340,5,5,White), __FUNCTION__);
			g+=5;
		}
	}
	if(level==3 && wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
				c+=10;
				}
				else{
					if(d!=310){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}
		if(e!=270){
			checkDraw2(tumDrawFilledBox(165+e,200,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(165+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		if(d==310 && f!=65){
			checkDraw2(tumDrawFilledBox(100+f,100,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,105,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,110,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,115,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,120,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,125,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,130,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,135,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,140,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,145,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,150,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,155,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,160,5,5,White), __FUNCTION__);

			checkDraw2(tumDrawFilledBox(100+f,400,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,395,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,390,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,385,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,380,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,375,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,370,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,365,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,360,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,355,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,350,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,345,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(100+f,340,5,5,White), __FUNCTION__);
			f+=5;
		}
		if(d==310 && g!=65){
			checkDraw2(tumDrawFilledBox(440+g,100,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,105,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,110,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,115,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,120,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,125,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,130,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,135,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,140,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,145,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,150,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,155,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,160,5,5,White), __FUNCTION__);

			checkDraw2(tumDrawFilledBox(440+g,400,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,395,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,390,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,385,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,380,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,375,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,370,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,365,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,360,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,355,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,350,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,345,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(440+g,340,5,5,White), __FUNCTION__);
			g+=5;
		}
	}
	if(level==4 && !wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
					c+=5;
				}
				else{
					if(d!=305){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}

		if(e!=110){
			checkDraw2(tumDrawFilledBox(145+e,200,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(145+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=105){
				checkDraw2(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				checkDraw2(tumDrawFilledBox(350+f,300,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=50){
					checkDraw2(tumDrawFilledBox(250,150+g,5,5,White), __FUNCTION__);
					checkDraw2(tumDrawFilledBox(350,150+g,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=55){
						checkDraw2(tumDrawFilledBox(250,300+h,5,5,White), __FUNCTION__);
						checkDraw2(tumDrawFilledBox(350,300+h,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==4 && wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
					c+=10;
				}
				else{
					if(d!=310){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}

		if(e!=110){
			checkDraw2(tumDrawFilledBox(145+e,200,5,5,White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(145+e,300,5,5,White), __FUNCTION__);
			e+=5;
		}
		else{
			if(f!=105){
				checkDraw2(tumDrawFilledBox(350+f,200,5,5,White), __FUNCTION__);
				checkDraw2(tumDrawFilledBox(350+f,300,5,5,White), __FUNCTION__);
				f+=5;
			}
			else{
				if(g!=50){
					checkDraw2(tumDrawFilledBox(250,150+g,5,5,White), __FUNCTION__);
					checkDraw2(tumDrawFilledBox(350,150+g,5,5,White), __FUNCTION__);
					g+=5;
				}
				else{
					if(h!=55){
						checkDraw2(tumDrawFilledBox(250,300+h,5,5,White), __FUNCTION__);
						checkDraw2(tumDrawFilledBox(350,300+h,5,5,White), __FUNCTION__);
						h+=5;
					}
				}
			}
		}
	}

	if(level==5 && wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
					c+=10;
				}
				else{
					if(d!=310){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
			if(d==310 && e!=305){
				checkDraw2(tumDrawFilledBox(100, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw2(tumDrawFilledBox(200, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw2(tumDrawFilledBox(300, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw2(tumDrawFilledBox(400, 100+e, 5, 5, White), __FUNCTION__);
				checkDraw2(tumDrawFilledBox(500, 100+e, 5, 5, White), __FUNCTION__);
				e+=5;
			}
			if(e==305 && f!=50){
				if(f==0){
					checkDraw2(tumDrawFilledBox(150, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==5){
					checkDraw2(tumDrawFilledBox(250, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==10){
					checkDraw2(tumDrawFilledBox(350, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==15){
					checkDraw2(tumDrawFilledBox(450, 170, 5, 5, Purple), __FUNCTION__);
				}
				if(f==30){
					checkDraw2(tumDrawFilledBox(450, 330, 5, 5, Purple), __FUNCTION__);
				}
				if(f==35){
					checkDraw2(tumDrawFilledBox(350, 330, 5, 5, Purple), __FUNCTION__);
				}
				if(f==40){
					checkDraw2(tumDrawFilledBox(250, 330, 5, 5, Purple), __FUNCTION__);
				}
				if(f==45){
					checkDraw2(tumDrawFilledBox(150, 330, 5, 5, Purple), __FUNCTION__);
				}
				f++;
			}
		}

	}
	if(level==5 && !wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
					c+=5;
				}
				else{
					if(d!=305){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(d==305 && e!=305){
			checkDraw2(tumDrawFilledBox(100, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(200, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(300, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(400, 100+e, 5, 5, White), __FUNCTION__);
			checkDraw2(tumDrawFilledBox(500, 100+e, 5, 5, White), __FUNCTION__);
			e+=5;
		}
		if(e==305 && f!=50){
			if(f==0){
				checkDraw2(tumDrawFilledBox(150, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==5){
				checkDraw2(tumDrawFilledBox(250, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==10){
				checkDraw2(tumDrawFilledBox(350, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==15){
				checkDraw2(tumDrawFilledBox(450, 170, 5, 5, Purple), __FUNCTION__);
			}
			if(f==30){
				checkDraw2(tumDrawFilledBox(450, 330, 5, 5, Purple), __FUNCTION__);
			}
			if(f==35){
				checkDraw2(tumDrawFilledBox(350, 330, 5, 5, Purple), __FUNCTION__);
			}
			if(f==40){
				checkDraw2(tumDrawFilledBox(250, 330, 5, 5, Purple), __FUNCTION__);
			}
			if(f==45){
				checkDraw2(tumDrawFilledBox(150, 330, 5, 5, Purple), __FUNCTION__);
			}
			f++;
		}
	}

	if(level==6 && wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,Grey), __FUNCTION__);
			a+=10;
		}
		else{
			if(b!=310){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,Grey), __FUNCTION__);
				b+=10;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,Grey), __FUNCTION__);
					c+=10;
				}
				else{
					if(d!=310){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,Grey), __FUNCTION__);
						d+=10;
					}
				}
			}
		}

		if(d==310 && e!=50){
			checkDraw2(tumDrawFilledBox((level6field[e][0]+11)*10,(level6field[e][1]+11)*10,5,5,White), __FUNCTION__);
			e++;
		}
	}
	if(level==6 && !wall){
		if(a!=400){
			checkDraw2(tumDrawFilledBox(100+a,100,5,5,White), __FUNCTION__);
			a+=5;
		}
		else{
			if(b!=300){
				checkDraw2(tumDrawFilledBox(500,100+b,5,5,White), __FUNCTION__);
				b+=5;
			}
			else{
				if(c!=400){
					checkDraw2(tumDrawFilledBox(500-c,400,5,5,White), __FUNCTION__);
					c+=5;
				}
				else{
					if(d!=305){
						checkDraw2(tumDrawFilledBox(100,400-d,5,5,White), __FUNCTION__);
						d+=5;
					}
				}
			}
		}
		if(d==305 && e!=50){
					checkDraw2(tumDrawFilledBox((level6field[e][0]+11)*10,(level6field[e][1]+11)*10,5,5,White), __FUNCTION__);
					e++;
				}
	}
}

/**
 * Method: Drawing Level1
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level1(int x1, int y1, int x2, int y2, int wall){
	int color=White;
	int distance=5;
	if(wall){
		color=Grey;
		distance=10;
	}
	//top wall side
	for(int x=x1; x<=x2;x+=distance)
	{
		checkDraw2(tumDrawFilledBox(x,y1,5,5,color), __FUNCTION__);
	}
	//bottom wall side
	for(int x=x1; x<=x2;x+=distance)
	{
		checkDraw2(tumDrawFilledBox(x,y2,5,5,color), __FUNCTION__);
	}
	//left wall side
	for(int y=y1; y<=y2;y+=distance)
	{
		checkDraw2(tumDrawFilledBox(x1,y,5,5,color), __FUNCTION__);
	}
	//right wall side
	for(int y=y1; y<=y2;y+=distance)
	{
		checkDraw2(tumDrawFilledBox(x2,y,5,5,color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level2
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level2(int x1, int y1, int x2, int y2, int wall){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;
	//Left Top to Center
	//Left Bottom to Center
	for(int x=x1; x<=250;x+=5)
	{
		checkDraw2(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	//Right Top to Center
	//Right Bottom to Center
	for(int x=350; x<=x2;x+=5)
	{
		checkDraw2(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level3
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level3(int x1, int y1, int x2, int y2, int wall){
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;
	//Middle Lines
	for(int x=170; x<=430;x+=5)
	{
		//Top
		checkDraw2(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		//Bottom
		checkDraw2(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	//Left Top Corner
	for(int y=100;y<=160;y+=5){
		for(int x=100;x<=160;x+=5){
				checkDraw2(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}

	}
	//Right Top Corner
	for(int y=100;y<=160;y+=5){
		for(int x=440;x<=500;x+=5){
				checkDraw2(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}
	}
	//Left Bottom Corner
	for(int y=340;y<=400;y+=5){
		for(int x=100;x<=160;x+=5){
				checkDraw2(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}

	}
	//Right Bottom Corner
	for(int y=340;y<=400;y+=5){
		for(int x=440;x<=500;x+=5){
				checkDraw2(tumDrawFilledBox(x,y,5,5,color), __FUNCTION__);
		}
	}
}

/**
 * Method: Drawing Level4
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level4(int x1, int y1, int x2, int y2, int wall){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;
	// Horizon Lines
	for(int x=x1+50; x<=250;x+=5)
	{
		checkDraw2(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	for(int x=350; x<=x2-50;x+=5)
	{
		checkDraw2(tumDrawFilledBox(x,200,5,5,color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(x,300,5,5,color), __FUNCTION__);
	}
	// Vertical Lines
	for(int y=y1+50; y<=200;y+=5)
	{
		checkDraw2(tumDrawFilledBox(250,y,5,5,color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(350,y,5,5,color), __FUNCTION__);
	}
	for(int y=300; y<=y2-50;y+=5)
	{
		checkDraw2(tumDrawFilledBox(250,y,5,5,color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(350,y,5,5,color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level5
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level5(int x1, int y1, int x2, int y2, int wall){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;

	//Teleport Items
	checkDraw2(tumDrawFilledBox(150, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(150, 330, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(250, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(250, 330, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(350, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(350, 330, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(450, 170, 5, 5, Purple), __FUNCTION__);
	checkDraw2(tumDrawFilledBox(450, 330, 5, 5, Purple), __FUNCTION__);

	//Wall
	for(int i=0;i<305;i+=5){
		checkDraw2(tumDrawFilledBox(100, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(200, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(300, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(400, 100+i, 5, 5, color), __FUNCTION__);
		checkDraw2(tumDrawFilledBox(500, 100+i, 5, 5, color), __FUNCTION__);
	}
}

/**
 * Method: Drawing Level6
 * Parameter: x1 top left corner x, y1 top left corner y
 * x2 bottom right corner x, y2 bottom right corner y, wall true or false
 */
void level6(int x1, int y1, int x2, int y2, int wall,int fieldArray[39][29]){
	//Outside wall White or Grey
	level1(x1,y1,x2,y2,wall);
	//Wall color
	int color=White;

	for(int i = 0; i<39; i++) {
		for (int j = 0; j<29; j++) {
			if(fieldArray[i][j] == 3) {
				checkDraw2(tumDrawFilledBox((i+11)*10, (j+11)*10, 5, 5, color), __FUNCTION__);
			}
		}
	}

}

/**
 * Method: Drawing GameOver if game is over
 */
void gameOverScreen(){
	int letter[500][400];
	//G
	letter[230][200]=1;
	letter[220][200]=1;
	letter[210][200]=1;
	letter[200][200]=1;
	letter[190][200]=1;
	letter[190][210]=1;
	letter[190][220]=1;
	letter[190][230]=1;
	letter[190][240]=1;
	letter[200][240]=1;
	letter[210][240]=1;
	letter[220][240]=1;
	letter[230][240]=1;
	letter[230][230]=1;
	letter[230][220]=1;
	letter[220][220]=1;
	letter[210][220]=1;
	//A
	letter[250][240]=1;
	letter[250][230]=1;
	letter[250][220]=1;
	letter[250][210]=1;
	letter[250][200]=1;
	letter[260][200]=1;
	letter[270][200]=1;
	letter[280][200]=1;
	letter[290][200]=1;
	letter[290][240]=1;
	letter[290][220]=1;
	letter[290][210]=1;
	letter[290][230]=1;
	letter[260][220]=1;
	letter[270][220]=1;
	letter[280][220]=1;
	//M
	letter[310][240]=1;
	letter[310][230]=1;
	letter[310][220]=1;
	letter[310][210]=1;
	letter[310][200]=1;
	letter[320][200]=1;
	letter[330][200]=1;
	letter[330][210]=1;
	letter[330][220]=1;
	letter[330][230]=1;
	letter[340][200]=1;
	letter[350][200]=1;
	letter[350][210]=1;
	letter[350][220]=1;
	letter[350][230]=1;
	letter[350][240]=1;
	//E
	letter[410][200]=1;
	letter[400][200]=1;
	letter[390][200]=1;
	letter[380][200]=1;
	letter[370][200]=1;
	letter[370][210]=1;
	letter[370][220]=1;
	letter[370][230]=1;
	letter[370][240]=1;
	letter[380][240]=1;
	letter[390][240]=1;
	letter[400][240]=1;
	letter[410][240]=1;
	letter[380][220]=1;
	letter[390][220]=1;
	letter[400][220]=1;
	//O
	letter[190][260]=1;
	letter[200][260]=1;
	letter[210][260]=1;
	letter[220][260]=1;
	letter[230][260]=1;
	letter[230][270]=1;
	letter[230][280]=1;
	letter[230][290]=1;
	letter[230][300]=1;
	letter[220][300]=1;
	letter[210][300]=1;
	letter[200][300]=1;
	letter[190][300]=1;
	letter[190][290]=1;
	letter[190][280]=1;
	letter[190][270]=1;
	letter[190][260]=1;
	//V
	letter[250][260]=1;
	letter[250][270]=1;
	letter[250][280]=1;
	letter[260][280]=1;
	letter[260][290]=1;
	letter[270][290]=1;
	letter[280][290]=1;
	letter[270][300]=1;
	letter[280][280]=1;
	letter[290][280]=1;
	letter[290][270]=1;
	letter[290][260]=1;
	//E
	letter[350][260]=1;
	letter[340][260]=1;
	letter[330][260]=1;
	letter[320][260]=1;
	letter[310][260]=1;
	letter[310][270]=1;
	letter[310][280]=1;
	letter[310][290]=1;
	letter[310][300]=1;
	letter[320][300]=1;
	letter[330][300]=1;
	letter[340][300]=1;
	letter[350][300]=1;
	letter[320][280]=1;
	letter[330][280]=1;
	letter[340][280]=1;
	//R
	letter[380][280]=1;
	letter[390][280]=1;
	letter[400][280]=1;
	letter[410][280]=1;
	letter[410][270]=1;
	letter[410][260]=1;
	letter[400][260]=1;
	letter[390][260]=1;
	letter[380][260]=1;
	letter[370][260]=1;
	letter[370][270]=1;
	letter[370][280]=1;
	letter[370][290]=1;
	letter[370][300]=1;
	letter[390][290]=1;
	letter[400][290]=1;
	letter[400][300]=1;
	letter[410][300]=1;


	if(a!=410){
		while(b<150){
			if(letter[100+a][180+b]){
				checkDraw2(tumDrawFilledBox(100+a,180+b,25,25,Black), __FUNCTION__);
			}
			else{
				checkDraw2(tumDrawFilledBox(100+a,180+b,5,5,White), __FUNCTION__);
			}
			b+=10;
		}
		b=0;
		a+=10;
	}
}
