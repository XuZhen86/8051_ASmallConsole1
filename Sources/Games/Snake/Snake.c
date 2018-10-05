#include"Sources/Games/Snake/Snake.h"

#include"Sources/I23LC512/I23LC512.h"
#include"Sources/LCD12864/LCD12864.h"
#include"Sources/Universal/SystemClock.h"
#include"Sources/Universal/Universal.h"
#include"Sources/Pushbutton/Pushbutton.h"

#include<stdio.h>
#include<stdlib.h>

enum SNAKE_DIRECTION{
    UP=PUSHBUTTON_DIRECTION_UP,
    DOWN=PUSHBUTTON_DIRECTION_DOWN,
    LEFT=PUSHBUTTON_DIRECTION_LEFT,
    RIGHT=PUSHBUTTON_DIRECTION_RIGHT,
    FORWARD=PUSHBUTTON_DIRECTION_FORWARD,
    BACK=PUSHBUTTON_DIRECTION_BACK,
    INVALID=PUSHBUTTON_DIRECTION_INVALID
};

enum SNAKE_RAM_CONFIG{
    MAP_ADDR=0x0000,
    MAP_SIZE=2048,
    MAP_DIRTY_ADDR=MAP_ADDR+MAP_SIZE,
    MAP_DIRTY_SIZE=128
};

enum SNAKE_MAP{
    MAP_X=32,
    MAP_Y=32,
    PIXEL_SIZE=2
};

enum SNAKE_LEVEL{
    LEVEL_MIN=1,
    LEVEL_MAX=16,
    LEVEL_DELAY=8,
    LEVEL_BASE=10,
    LEVEL_DEFAULT=8
};

enum SNAKE_SPLASH_SCREEN_EXIT_CODE{
    SNAKE_SPLASH_SCREEN_EXIT_CODE_CANCELED,
    SNAKE_SPLASH_SCREEN_EXIT_CODE_ENTER_GAME
};

static unsigned int
    snakeLength,
    snakeTick,
    refreshInterval;

static unsigned char
    snakeHeadX,snakeHeadY,
    snakeTailX,snakeTailY,
    foodX,foodY,
    direction;

unsigned char snake(){
    if(snake_splashScreen()==SNAKE_SPLASH_SCREEN_EXIT_CODE_CANCELED){
        return SNAKE_EXIT_CODE_CANCELED;
    }
    return snake_gamePlay();
}

unsigned char snake_gamePlay(){
    unsigned int data tailVal;
    unsigned char data pressedDirection;
    unsigned char data i,j;
    bit foundTail=0;

    lcd12864_stringSet(6,13,"Length");
    lcd12864_stringSet(7,13,"Ticks");

    while(1){
        systemClock_timerStart(refreshInterval);

        pressedDirection=direction;
        while(!systemClock_timerIsTimeUp()&&delay(0,108,145)){
            if(pushbutton_directionGet()!=INVALID){
                pressedDirection=pushbutton_lastPressedDirectionGet();
            }
        }

        if(pressedDirection!=direction){
            if(((direction==UP||direction==DOWN)&&(pressedDirection==LEFT||pressedDirection==RIGHT))||((direction==LEFT||direction==RIGHT)&&(pressedDirection==UP||pressedDirection==DOWN))){
                direction=pressedDirection;
            }
        }

        switch(direction){
            case UP:
                snakeHeadX--;
                break;
            case DOWN:
                snakeHeadX++;
                break;
            case LEFT:
                snakeHeadY--;
                break;
            case RIGHT:
                snakeHeadY++;
                break;
        }

        if(snake_mapGet(snakeHeadX,snakeHeadY)!=0){
            break;
        }

        snake_mapSet(snakeHeadX,snakeHeadY,++snakeTick);
        if(snakeTick==65534){
            snake_restartTick();
        }

        if(snakeHeadX!=foodX||snakeHeadY!=foodY){
            foundTail=0;
            tailVal=snake_mapGet(snakeTailX,snakeTailY);

            for(i=snakeTailX-1;i<=snakeTailX+1&&!foundTail;i++){
                for(j=snakeTailY-1;j<=snakeTailY+1&&!foundTail;j++){
                    if(0<i&&i<MAP_X&&0<j&&j<MAP_Y&&snake_mapGet(i,j)==tailVal+1){
                        foundTail=1;
                        snake_mapSet(snakeTailX,snakeTailY,0);
                        snakeTailX=i;
                        snakeTailY=j;
                    }
                }
            }

            if(!foundTail){
                return SNAKE_EXIT_CODE_UNEXPECTED_TAIL_DATA;
            }
        }else{
            snakeLength++;
            snake_newFood();
        }

        snake_renewDisplay(0);
    }

    pushbutton_waitDirectionGet();
    pushbutton_waitDirectionRelease();

    return SNAKE_EXIT_CODE_NORMAL;
}

unsigned char snake_splashScreen(){
    unsigned char data level=LEVEL_DEFAULT;
    unsigned char buffer[4];
    bit enterGame=0;

    lcd12864_clear();
    lcd12864_stringSet(2,10,"Snake");
    lcd12864_stringSet(5,8,"Level:");
    lcd12864_flush(0);

    snake_mapInitialize();

    while(!enterGame){
        refreshInterval=LEVEL_DELAY*(LEVEL_MAX-level)+LEVEL_BASE;
        sprintf(buffer,"%2bu",level);
        lcd12864_stringSet(5,14,buffer);
        lcd12864_flush(0);

        switch(pushbutton_waitDirectionGet()){
            case UP:
                if(LEVEL_MAX<(++level)){
                    level=LEVEL_MAX;
                }
                break;
            case DOWN:
                if((--level)<LEVEL_MIN){
                    level=LEVEL_MIN;
                }
                break;
            case FORWARD:
                enterGame=1;
                break;
            case BACK:
                return SNAKE_SPLASH_SCREEN_EXIT_CODE_CANCELED;
        }

        pushbutton_waitDirectionRelease();
    }

    srand(systemClock_get());
    snake_newFood();

    lcd12864_clear();
    snake_renewDisplay(1);

    return SNAKE_SPLASH_SCREEN_EXIT_CODE_ENTER_GAME;
}

void snake_restartTick(){
    unsigned int data deltaTick=snake_mapGet(snakeTailX,snakeTailY);
    unsigned char data i,j;
    unsigned char buffer[60];

    for(i=1;i<31;i++){
        i23lc512_uCharSeqRead(buffer,MAP_ADDR+i*64+2,60);
        for(j=1;j<31;j++){
            if(((unsigned int*)buffer)[j]){
                ((unsigned int*)buffer)[j]-=deltaTick;
            }
        }
        i23lc512_uCharSeqWrite(buffer,MAP_ADDR+i*64+2,60);
    }

    snakeTick-=deltaTick;
}

void snake_mapInitialize(){
    unsigned char data i;

    // Clear map
    i23lc512_memset(MAP_ADDR,0,MAP_SIZE);
    // Clear map delta
    i23lc512_memset(MAP_DIRTY_ADDR,0xff,MAP_DIRTY_SIZE);

    // Setup walls
    for(i=0;i<32;i++){
        snake_mapSet(0,i,0xffff);
        snake_mapSet(31,i,0xffff);
        snake_mapSet(i,0,0xffff);
        snake_mapSet(i,31,0xffff);
    }

    snakeHeadX=15;
    snakeHeadY=15;
    snakeTailX=15;
    snakeTailY=19;
    direction=LEFT;
    snakeLength=0;
    snakeTick=0;

    // Put baby snake in
    for(i=snakeTailY;i>=snakeHeadY;i--){
        snake_mapSet(snakeHeadX,i,++snakeTick);
    }
    snakeLength=snakeTick;
}

unsigned int snake_mapGet(unsigned char x,unsigned char y){
    x%=32;
    y%=32;
    return i23lc512_uIntRead(MAP_ADDR+x*64+y*2);
}

unsigned int snake_mapSet(unsigned char x,unsigned char y,unsigned int val){
    x%=32;
    y%=32;
    i23lc512_uCharWrite(MAP_DIRTY_ADDR+x*4+y/8,i23lc512_uCharRead(MAP_DIRTY_ADDR+x*4+y/8)|(1<<(y%8)));
    return i23lc512_uIntWrite(MAP_ADDR+x*64+y*2,val);
}

void snake_renewDisplay(bit forceFlush){
    unsigned char data i,j;
    unsigned char buffer[8];
    bit lightUp;

    // display score
    sprintf(buffer,"%5u",snakeLength);
    lcd12864_stringSet(6,20,buffer);
    sprintf(buffer,"%6u",snakeTick);
    lcd12864_stringSet(7,19,buffer);

    // draw map
    for(i=0;i<64;i+=2){
        i23lc512_uCharSeqRead(buffer,MAP_DIRTY_ADDR+i*2,4);
        for(j=0;j<64;j+=2){
            if(buffer[j/16]&(1<<(j/2%8))){
                lightUp=(snake_mapGet(i/2,j/2)!=0);
                lcd12864_pixelSet(i,j,lightUp);
                lcd12864_pixelSet(i+1,j,lightUp);
                lcd12864_pixelSet(i,j+1,lightUp);
                lcd12864_pixelSet(i+1,j+1,lightUp);
            }
        }
    }
    i23lc512_memset(MAP_DIRTY_ADDR,0,MAP_DIRTY_SIZE);

    // draw food
    lcd12864_pixelSet(foodX*2,foodY*2,1);
    lcd12864_pixelSet(foodX*2+1,foodY*2,1);
    lcd12864_pixelSet(foodX*2,foodY*2+1,1);
    lcd12864_pixelSet(foodX*2+1,foodY*2+1,1);

    lcd12864_flush(forceFlush);
}

void snake_newFood(){
    do{
        foodX=(unsigned int)rand()%30+1;
        foodY=(unsigned int)rand()%30+1;
    }while(snake_mapGet(foodX,foodY)!=0);
}

// void _snake_printMap(){
//     unsigned char i,j;
//     for(i=0;i<MAP_X;i++){
//         for(j=0;j<MAP_Y;j++){
//             printf("%04x ",snake_mapGet(i,j));
//         }
//         printf("\n");
//     }
//     printf("\n");
// }
