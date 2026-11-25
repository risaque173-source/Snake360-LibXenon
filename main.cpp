#include <SDL/SDL.h>
#include <cstdlib>
#include <ctime>
#include <cstdio>

static const int SCREEN_W = 640, SCREEN_H = 480;
static const int CELL = 16;
static const int GRID_W = 36, GRID_H = 25;
static const int MAX_SNAKE = GRID_W * GRID_H;

struct Point { int x, y; };
enum Scene { TITLE, PLAYING, GAME_OVER, HOW_TO };

static SDL_Surface *screen = 0;
static Point snake[MAX_SNAKE];
static int snakeLen = 4;
static Point food;
static int dx = 1, dy = 0, nextDx = 1, nextDy = 0;
static int score = 0, best = 0;
static int tickMs = 150;
static Uint32 lastStep = 0;
static Scene scene = TITLE;
static bool quitGame = false;

static Uint32 rgb(Uint8 r, Uint8 g, Uint8 b) {
    return SDL_MapRGB(screen->format, r, g, b);
}
static void rect(int x, int y, int w, int h, Uint32 c) {
    SDL_Rect r; r.x=x; r.y=y; r.w=w; r.h=h;
    SDL_FillRect(screen, &r, c);
}
static void text(const char *s, int x, int y, Uint32 color) {
    // Tiny built-in 5x7 pixel font for the letters used by the menus.
    // Each glyph is encoded as seven 5-bit rows.
    struct Glyph { char c; unsigned char row[7]; };
    static const Glyph font[] = {
      {'A',{14,17,17,31,17,17,17}}, {'B',{30,17,17,30,17,17,30}},
      {'C',{14,17,16,16,16,17,14}}, {'D',{30,17,17,17,17,17,30}},
      {'E',{31,16,16,30,16,16,31}}, {'F',{31,16,16,30,16,16,16}},
      {'G',{14,17,16,23,17,17,15}}, {'H',{17,17,17,31,17,17,17}},
      {'I',{31,4,4,4,4,4,31}}, {'J',{7,2,2,2,18,18,12}},
      {'K',{17,18,20,24,20,18,17}}, {'L',{16,16,16,16,16,16,31}},
      {'M',{17,27,21,21,17,17,17}}, {'N',{17,25,21,19,17,17,17}},
      {'O',{14,17,17,17,17,17,14}}, {'P',{30,17,17,30,16,16,16}},
      {'Q',{14,17,17,17,21,18,13}}, {'R',{30,17,17,30,20,18,17}},
      {'S',{15,16,16,14,1,1,30}}, {'T',{31,4,4,4,4,4,4}},
      {'U',{17,17,17,17,17,17,14}}, {'V',{17,17,17,17,17,10,4}},
      {'W',{17,17,17,21,21,21,10}}, {'X',{17,17,10,4,10,17,17}},
      {'Y',{17,17,10,4,4,4,4}}, {'Z',{31,1,2,4,8,16,31}},
      {'0',{14,17,19,21,25,17,14}}, {'1',{4,12,4,4,4,4,14}},
      {'2',{14,17,1,2,4,8,31}}, {'3',{30,1,1,14,1,1,30}},
      {'4',{2,6,10,18,31,2,2}}, {'5',{31,16,16,30,1,1,30}},
      {'6',{14,16,16,30,17,17,14}}, {'7',{31,1,2,4,8,8,8}},
      {'8',{14,17,17,14,17,17,14}}, {'9',{14,17,17,15,1,1,14}},
      {':',{0,4,4,0,4,4,0}}, {'-',{0,0,0,31,0,0,0}},
      {'/',{1,2,2,4,8,8,16}}, {' ',{0,0,0,0,0,0,0}}
    };
    for (int n=0; s[n]; ++n) {
        char ch=s[n]; if(ch>='a' && ch<='z') ch-=32;
        const Glyph *g=0;
        for(unsigned i=0;i<sizeof(font)/sizeof(font[0]);++i)
            if(font[i].c==ch) { g=&font[i]; break; }
        if(g) for(int yy=0;yy<7;++yy) for(int xx=0;xx<5;++xx)
            if(g->row[yy] & (1 << (4-xx))) rect(x+n*12+xx*2,y+yy*2,2,2,color);
    }
}
static void centerText(const char *s, int y, Uint32 c) {
    int len=0; while(s[len]) ++len;
    text(s, (SCREEN_W-len*12)/2, y, c);
}
static bool occupied(int x,int y) {
    for(int i=0;i<snakeLen;++i) if(snake[i].x==x && snake[i].y==y) return true;
    return false;
}
static void spawnFood() {
    do { food.x=rand()%GRID_W; food.y=rand()%GRID_H; }
    while(occupied(food.x,food.y));
}
static void resetGame() {
    snakeLen=4; snake[0].x=18; snake[0].y=12;
    for(int i=1;i<4;++i) { snake[i].x=18-i; snake[i].y=12; }
    dx=1; dy=0; nextDx=1; nextDy=0; score=0; tickMs=150;
    spawnFood(); lastStep=SDL_GetTicks(); scene=PLAYING;
}
static void drawBoard() {
    rect(0,0,SCREEN_W,SCREEN_H,rgb(8,18,10));
    rect(24,42,592,400,rgb(18,38,20));
    // Nokia-like grid
    for(int x=24;x<=600;x+=CELL) rect(x,42,1,400,rgb(24,48,26));
    for(int y=42;y<=440;y+=CELL) rect(24,y,592,1,rgb(24,48,26));
    for(int i=snakeLen-1;i>=0;--i) {
        Uint32 c = (i==0)?rgb(190,255,100):rgb(105,205,65);
        rect(25+snake[i].x*CELL,43+snake[i].y*CELL,CELL-1,CELL-1,c);
    }
    rect(25+food.x*CELL,43+food.y*CELL,CELL-1,CELL-1,rgb(255,220,70));
    text("SCORE",28,12,rgb(180,240,150));
    char buf[32]; std::sprintf(buf,"%d",score); text(buf,112,12,rgb(255,255,255));
}
static void stepGame() {
    dx=nextDx; dy=nextDy;
    Point h={snake[0].x+dx,snake[0].y+dy};
    if(h.x<0 || h.y<0 || h.x>=GRID_W || h.y>=GRID_H) { scene=GAME_OVER; return; }
    bool eating=(h.x==food.x && h.y==food.y);
    int limit=snakeLen-(eating?0:1);
    for(int i=0;i<limit;++i) if(snake[i].x==h.x && snake[i].y==h.y) { scene=GAME_OVER; return; }
    if(eating && snakeLen<MAX_SNAKE) ++snakeLen;
    for(int i=snakeLen-1;i>0;--i) snake[i]=snake[i-1];
    snake[0]=h;
    if(eating) { score+=10; if(score>best) best=score; if(tickMs>65) tickMs-=5; spawnFood(); }
}
static void drawTitle() {
    rect(0,0,SCREEN_W,SCREEN_H,rgb(8,18,10));
    centerText("SNAKE 360",100,rgb(190,255,100));
    centerText("CLASSIC NOKIA STYLE",145,rgb(130,210,100));
    centerText("A - PLAY",235,rgb(255,255,255));
    centerText("X - HOW TO PLAY",270,rgb(255,255,255));
    centerText("BEST SCORE",335,rgb(150,220,120));
    char b[24]; std::sprintf(b,"%d",best); centerText(b,360,rgb(255,255,255));
}
static void drawHowTo() {
    rect(0,0,SCREEN_W,SCREEN_H,rgb(8,18,10));
    centerText("HOW TO PLAY",85,rgb(190,255,100));
    centerText("D PAD - MOVE",160,rgb(255,255,255));
    centerText("EAT THE FOOD",195,rgb(255,255,255));
    centerText("DO NOT HIT WALLS",230,rgb(255,255,255));
    centerText("OR YOUR OWN TAIL",265,rgb(255,255,255));
    centerText("A - BACK",350,rgb(190,255,100));
}
static void drawGameOver() {
    drawBoard();
    rect(145,170,350,130,rgb(5,15,7));
    centerText("GAME OVER",190,rgb(255,100,80));
    char b[32]; std::sprintf(b,"SCORE %d",score); centerText(b,225,rgb(255,255,255));
    centerText("A - RETRY   B - MENU",265,rgb(190,255,100));
}
static void handleInput() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type==SDL_QUIT) quitGame=true;
        if(e.type==SDL_KEYDOWN) {
            SDLKey k=e.key.keysym.sym;
            if(k==SDLK_ESCAPE) quitGame=true;
            if(k==SDLK_UP && dy!=1) { nextDx=0; nextDy=-1; }
            if(k==SDLK_DOWN && dy!=-1) { nextDx=0; nextDy=1; }
            if(k==SDLK_LEFT && dx!=1) { nextDx=-1; nextDy=0; }
            if(k==SDLK_RIGHT && dx!=-1) { nextDx=1; nextDy=0; }
            if(k==SDLK_RETURN || k==SDLK_SPACE) {
                if(scene==TITLE || scene==GAME_OVER) resetGame();
                else if(scene==HOW_TO) scene=TITLE;
            }
            if(k==SDLK_x && scene==TITLE) scene=HOW_TO;
        }
        if(e.type==SDL_JOYAXISMOTION && e.jaxis.axis<2) {
            if(e.jaxis.axis==0) {
                if(e.jaxis.value < -12000 && dx!=1) { nextDx=-1; nextDy=0; }
                if(e.jaxis.value > 12000 && dx!=-1) { nextDx=1; nextDy=0; }
            } else {
                if(e.jaxis.value < -12000 && dy!=1) { nextDx=0; nextDy=-1; }
                if(e.jaxis.value > 12000 && dy!=-1) { nextDx=0; nextDy=1; }
            }
        }
        if(e.type==SDL_JOYBUTTONDOWN) {
            if(e.jbutton.button==0) {
                if(scene==TITLE || scene==GAME_OVER) resetGame();
                else if(scene==HOW_TO) scene=TITLE;
            }
            if(e.jbutton.button==2 && scene==TITLE) scene=HOW_TO;
            if(e.jbutton.button==1 && scene==GAME_OVER) scene=TITLE;
        }
    }
}
int main(int argc,char **argv) {
    (void)argc; (void)argv;
    std::srand((unsigned)std::time(0));
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_TIMER)<0) return 1;
    screen=SDL_SetVideoMode(SCREEN_W,SCREEN_H,32,SDL_SWSURFACE);
    if(!screen) { SDL_Quit(); return 2; }
    SDL_ShowCursor(SDL_DISABLE);
    if(SDL_NumJoysticks()>0) { SDL_JoystickOpen(0); SDL_JoystickEventState(SDL_ENABLE); }
    while(!quitGame) {
        handleInput();
        if(scene==PLAYING && SDL_GetTicks()-lastStep >= (Uint32)tickMs) {
            stepGame(); lastStep=SDL_GetTicks();
        }
        if(scene==TITLE) drawTitle();
        else if(scene==HOW_TO) drawHowTo();
        else if(scene==PLAYING) drawBoard();
        else drawGameOver();
        SDL_Flip(screen);
        SDL_Delay(8);
    }
    SDL_Quit();
    return 0;
}
