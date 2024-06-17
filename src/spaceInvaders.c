#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>

//General 
#define KILLHEIGHT 34
#define GAMETIME 1000
//Player
#define HEAD '|'
#define HEIGHT_SET 40
#define WIDTH_SET 30
#define BOD '#'
#define BULLET '0'
#define BULLET_MAX 5
#define BULLET_SPEED 5
//Alien
#define ALIEN_BULLET '|'
#define ALIEN_BULLET_MAX 5
#define ALIEN 'V'
#define ALIEN_MAX 100

typedef struct{
	char alien;
	int x,y,status;
}Alien;

typedef struct{
	char gun;
	char pers; 
	int x,y;
}character;

typedef struct{
	char bullet;
	int x,y;
	int status;
}Bullet;

WINDOW *win;

struct winsize sz;

Bullet bulletList[BULLET_MAX];
Bullet alienBulletList[BULLET_MAX];
Alien alienList[ALIEN_MAX];

int HEIGHT, WIDTH, alienSpawnSpeed;
int killCount=0;
int score=0;
int gameEnd=0;

void keys( int c , character *charac);
void winUpdate( character *charac );
void bulletSpawn( character *charac );
void alienSpawn( int rand );
void alienUpdate();
void bulletUpdate();
void init();

int randGen(int lower, int upper);

long long current_timestamp();

int main(){
	init();

	character charac;
	charac.gun=HEAD;
	charac.pers=BOD;
	charac.x=WIDTH/2;
	charac.y=HEIGHT-4;
	winUpdate(&charac);

	long long retTimeBul=current_timestamp()+BULLET_SPEED;
	long long retTimeAl=current_timestamp()+alienSpawnSpeed;
	int h;
	while (!gameEnd) {
		keys(getch(), &charac);
		winUpdate(&charac);
		score=killCount*10;
		if (retTimeBul<=current_timestamp()) {
			bulletUpdate();
			retTimeBul=current_timestamp()+BULLET_SPEED;
		}
		if (retTimeAl<=current_timestamp()) {
			alienSpawn(randGen(1, WIDTH-2));
			alienUpdate();
			retTimeAl=current_timestamp()+alienSpawnSpeed;
		}
	}
	werase(win);
	wrefresh(win);
	box(win, 0, 0);
	char score[1024];
	int length=floor(log10(abs(killCount)))+1;
	sprintf(score, "%d", killCount);
	mvwprintw(win, 1, floor((double)WIDTH/2)-5, "Score:");
	mvwprintw(win, 2, floor((double)WIDTH/2)-length, "%s", score);
	nodelay(stdscr, FALSE);
	wrefresh(win);
	getch();
	endwin();
	exit(0);
}

void init(){
	HEIGHT=HEIGHT_SET;
	WIDTH=WIDTH_SET;
	alienSpawnSpeed=1000;
	srand(time(0)); 

	initscr();
	start_color();
	noecho();
	nodelay(stdscr, TRUE);

	ioctl( 0, TIOCGWINSZ, &sz );
	if (WIDTH>sz.ws_col) {
		WIDTH=sz.ws_col;
	}
	if (HEIGHT>sz.ws_row) {
		HEIGHT=sz.ws_row-2;
	}

	win=newwin(HEIGHT+2, WIDTH, 0, 0);
}

void winUpdate( character *charac ){
	mvwaddch(win, charac->y-1, charac->x, charac->gun);
	mvwaddch(win, charac->y+1, charac->x-2, charac->pers);
	mvwaddch(win, charac->y+1, charac->x-1, charac->pers);
	mvwaddch(win, charac->y+1, charac->x, charac->pers);
	mvwaddch(win, charac->y+1, charac->x+1, charac->pers);
	mvwaddch(win, charac->y+1, charac->x+2, charac->pers);
	mvwaddch(win, charac->y, charac->x-1, charac->pers);
	mvwaddch(win, charac->y, charac->x, charac->pers);
	mvwaddch(win, charac->y, charac->x+1, charac->pers);
	move(charac->y, charac->x);
	for (int i=0; i<ALIEN_MAX; i++) {
		if (alienList[i].status==1) {
			mvwaddch(win, alienList[i].y, alienList[i].x, alienList[i].alien);
		}
	}
	move(charac->y+1, charac->x);
	for (int i=0; i<BULLET_MAX; i++) {
		if (bulletList[i].status==1) {
			mvwaddch(win, bulletList[i].y, bulletList[i].x, bulletList[i].bullet);
		}
	}
	mvwhline(win, HEIGHT-2, 1, 0, WIDTH-2);
	mvwprintw(win, HEIGHT-1, 2, "Score");
	mvwprintw(win, HEIGHT, 2, "%d", score);
	init_pair(1, COLOR_WHITE, COLOR_RED);
	init_pair(2, COLOR_WHITE, COLOR_BLACK);
	int i=1;
	while (i!=HEIGHT-4) {
		mvwchgat(win, i, charac->x-1, 1, A_BLINK, 2, NULL);
		mvwchgat(win, i, charac->x+1, 1, A_BLINK, 2, NULL);
		mvwchgat(win, i, charac->x,1, A_BLINK, 1, NULL);
		i++;
	}
	wmove(win, charac->y+1, charac->x);
	wrefresh(win);
	box(win, 0, 0);
}

void charClear( character *charac ){
	mvwaddch(win, charac->y-1, charac->x, ' ');
	mvwaddch(win, charac->y+1, charac->x-2, ' ');
	mvwaddch(win, charac->y+1, charac->x-1, ' ');
	mvwaddch(win, charac->y+1, charac->x, ' ');
	mvwaddch(win, charac->y+1, charac->x+1, ' ');
	mvwaddch(win, charac->y+1, charac->x+2, ' ');
	mvwaddch(win, charac->y, charac->x-1, ' ');
	mvwaddch(win, charac->y, charac->x, ' ');
	mvwaddch(win, charac->y, charac->x+1, ' ');
}

void keys( int c , character *charac){
	switch (c) {
		case 'a':
			if (getch()==' ') {
				bulletSpawn(charac);
			}
			if ( charac->x-2==-1 ) {
				break;
			}
			charClear(charac);
			charac->x--;
			break;
		case 'd':
			if (getch()==' ') {
				bulletSpawn(charac);
			}
			if ( charac->x+2==WIDTH ) {
				break;
			}
			charClear(charac);
			charac->x++;
			break;
		case ' ':
			bulletSpawn(charac);
			break;
		case 'q':
			endwin();
			exit(1);
			break;
	}
	wmove(win, charac->y+1, charac->x);
}

void bulletSpawn( character *charac ){
	for (int i = 0; i<BULLET_MAX; i++) {
		if (bulletList[i].status == 0) {
			bulletList[i].status = 1;
			bulletList[i].bullet = BULLET;
			bulletList[i].x = charac->x;
			bulletList[i].y = charac->y - 2;
			break;		
		}
	}
}

void bulletUpdate(){
	for (int i=0; i<BULLET_MAX; i++) {
		if(bulletList[i].status==1){
			if (bulletList[i].y==0) {
				bulletList[i].status=0;
			}
			for (int j=0; j<ALIEN_MAX; j++) {
				if (alienList[j].y==bulletList[i].y && alienList[j].x==bulletList[i].x && alienList[j].status!=0) {
					bulletList[i].status=0;
					bulletList[i].bullet=' ';
					alienList[j].status=0;
					alienList[j].alien=' ';
					killCount++;
					if ((alienSpawnSpeed-=floor(killCount*2.5))<400) {
						alienSpawnSpeed=400;
					}
				}
			}
			mvwaddch(win, bulletList[i].y, bulletList[i].x, ' ');
			bulletList[i].y--;
		}
	}
}

//Alien spawn
int h;
void alienSpawn( int rand ){
	for (int i = 0; i<ALIEN_MAX; i++) {
		if (alienList[i].status == 0) {
			alienList[i].status = 1;
			if ((alienList[i].x=rand)==h) {
				alienList[i].x=randGen(1, WIDTH-2);
			}
			alienList[i].y =1;
			alienList[i].alien=ALIEN;
			break;		
		}
	}
	h=rand;
}

void alienUpdate(){
	for (int i=0; i<ALIEN_MAX; i++) {
		if(alienList[i].status==1){
			mvwaddch(win, alienList[i].y, alienList[i].x, ' ');
			alienList[i].y++;
			if (alienList[i].y>KILLHEIGHT){
				gameEnd=1;
			}
		}
	}
}

long long current_timestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return (long long)(tp.tv_sec * 1000 + tp.tv_nsec / 1000000);
}

int randGen(int lower, int upper) { 
	return (rand() % (upper - lower + 1)) + lower; 
} 
