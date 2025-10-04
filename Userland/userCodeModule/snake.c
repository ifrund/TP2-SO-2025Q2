#include "include/snake.h"
#include "include/drawings.h"
#include "include/userlib.h"
#include "include/rand.h"

#define APPLE 1
#define SNAKE1  4
#define SNAKE2  8

//================================================================================================================================
// Main Screen Stuff
//================================================================================================================================
//================================================================================================================================
uint8_t snakeScreen(){
    //title
    putSnakeTitle();

    #define PLAYERS_1 0
    #define PLAYERS_2 1

    char keypressed[1]={0};
    uint8_t selected=0, selection=PLAYERS_1;

    while(!selected){
        if(read(keypressed,1)>0){
            switch(keypressed[0]){
                case('\n'):
                    selected=1;
                    break;

                case('w'):
                case('i'):
                    selection=PLAYERS_1;
                    break;

                case('s'):
                case('k'):
                    selection=PLAYERS_2;
                    break;

                default:
                    break;
            }
        }
        //cambiamos  la apariencia segun la selection
        selectHover(selection);
    }

    return selection;
}

//================================================================================================================================
// Board Stuff
//================================================================================================================================
//================================================================================================================================
#define MIN_DIM 5//contempla dim_snake_start=3 con 2 de margen
#define START_MARGIN 2
#define BOARD_UP_MARGIN 3

uint8_t BOARD_H=20; //basandonos en las dimensiones del de maps
uint8_t BOARD_W=20;

uint8_t board[20][20];//valor grande para inicializarla (evitamos tener que usar malloc)
//b7-b4 for direction b3-b0 for value
//we use & 0x0F and 0xF0 multiple times to read the corresponding value

uint8_t fontSize, drawSize;
uint16_t screenHeight, screenWidth;

uint8_t dibSpaceHeight, dibSpaceWidth;//el valor que realmente toma luego del size

uint8_t board_start_x, board_start_y;

uint8_t middleBoard_y;

//================================================================================================================================
//players stuff
uint16_t player1Points=0;
uint16_t player2Points=0;

uint32_t snakecolor1=0x00;
uint32_t snakecolor2=0x00;

//================================================================================================================================
//setup
uint8_t initGame(){
    changeDrawSize(3);
    getScreenData(&screenHeight,&screenWidth,&fontSize,&drawSize);
    dibSpaceHeight=(dibHeight*drawSize);
    dibSpaceWidth=(dibWidth*drawSize);
    BOARD_H= BOARD_H>((screenHeight/dibSpaceHeight)-BOARD_UP_MARGIN)?  ((screenHeight/dibSpaceHeight)-BOARD_UP_MARGIN) : BOARD_H;    //buscamos la menor dimension
    BOARD_W= BOARD_W>(screenWidth/dibSpaceWidth)?  (screenWidth/dibSpaceWidth) : BOARD_W;
    BOARD_H>BOARD_W? (BOARD_H=BOARD_W) : (BOARD_W=BOARD_H);//nos aseguramos de que el tablero sea cuadrado
    if(BOARD_H<MIN_DIM) return 1;
    for(int i=0; i<BOARD_W; i++){
        for(int j=0; j<BOARD_H; j++){
            board[i][j]=0;
        }
    }
    board_start_x=(screenWidth-(BOARD_W*dibSpaceWidth))/2;
    board_start_y=(BOARD_UP_MARGIN*dibSpaceHeight);
    middleBoard_y=BOARD_H/2;

    return 0;
}


//================================================================================================================================
//refresh screen value
void printPoints(uint8_t snake){
    uint64_t startXPlayer1Points=((dibSpaceWidth*8)/2)-(dibSpaceHeight*3)/2;//8 as in strlen(PLAYER N)
    uint64_t startXPlayer2Points=screenWidth-(((dibSpaceWidth*8)/2)+(dibSpaceHeight*3)/2);
    //primero impimimos sobre lo que haya y luego imprimimos los nuevos puntos
    //contemplamos solo hasta 3 cifras los puntos
    if(snake==SNAKE1){
        for(int i=0; i<3; i++){
            draw(square[0], SCREEN_BG_MARGIN, dibHeight, startXPlayer1Points + i*(dibSpaceWidth), dibSpaceHeight);
        }

        draw(points_digits[(player1Points%1000)/100], WHITETEXT, dibHeight, startXPlayer1Points, dibSpaceHeight);
        draw(points_digits[(player1Points%100)/10], WHITETEXT, dibHeight, startXPlayer1Points + dibSpaceWidth, dibSpaceHeight);
        draw(points_digits[player1Points%10], WHITETEXT, dibHeight, startXPlayer1Points + dibSpaceWidth*2, dibSpaceHeight);
    }
    else{
        for(int i=0; i<3; i++){
            draw(square[0], SCREEN_BG_MARGIN, dibHeight, startXPlayer2Points + i*(dibSpaceWidth), dibSpaceHeight);
        }
        
        draw(points_digits[(player2Points%1000)/100], WHITETEXT, dibHeight, startXPlayer2Points, dibSpaceHeight);
        draw(points_digits[(player2Points%100)/10], WHITETEXT, dibHeight, startXPlayer2Points + dibSpaceWidth, dibSpaceHeight);
        draw(points_digits[player2Points%10], WHITETEXT, dibHeight, startXPlayer2Points + dibSpaceWidth*2, dibSpaceHeight);
    }
}

void tablero(){
    
    for(uint8_t i=0; i<BOARD_W; i++){
        for(uint8_t j=0; j<BOARD_H;j++){
            //dibujo la casilla
            uint32_t color= 
                (( ((i%(2)==0) && (j%(2))) ||  ((i%(2)) && (j%(2)==0))))? BOARDCOLOR1:BOARDCOLOR2;
            draw(square[0], color, dibHeight, i*(dibSpaceWidth) + board_start_x, j*(dibSpaceHeight) + board_start_y);

            uint8_t casilla = board[i][j] & 0x0F;
            if(casilla==APPLE) draw_manzana(i*(dibSpaceWidth) + board_start_x ,j*(dibSpaceHeight) + board_start_y);
            else if(casilla==SNAKE1 || casilla==SNAKE2) putSnake(j,i,casilla);
        }
    }
}


//================================================================================================================================
// Logic Stuff
//================================================================================================================================
//================================================================================================================================

//================================================================================================================================
//adders
void addApple(){
    uint8_t row, column;
    do{
        row = (uint8_t) (rand()%BOARD_H);
        column = (uint8_t) (rand()%BOARD_W);
    }
    while(board[column][row]);//espero a una casilla no ocupada
    board[column][row]=1;//guardo manzana
}

void addSnake(uint8_t row, uint8_t column, uint8_t elem, enum Direction dir){
    board[column][row]=elem + dir;
}

//================================================================================================================================
//directions stuff

uint8_t snake1_head_row=0, snake1_head_column=0;
uint8_t snake2_head_row=0, snake2_head_column=0;
uint8_t snake1_tail_row=0, snake1_tail_column=0;
uint8_t snake2_tail_row=0, snake2_tail_column=0;

//================================================================================================================================
//movement

uint8_t slither(enum Direction dir, uint8_t snake){
    uint8_t column=getSnakeHeadCol(snake), row=getSnakeHeadRow(snake);
    uint8_t newHeadCol=column, newHeadRow=row;
    switch(dir){
        case (UP):
            if(row<=0) return 1;//border colision
            newHeadRow--;
            break;

        case (DOWN):
            if(row>=BOARD_H-1) return 1;//border colision
            newHeadRow++;
            break;

        case (RIGHT):
            if(column>=BOARD_W-1) return 1;//border colision
            newHeadCol++;
            break;

        case (LEFT):
            if(column<=0) return 1;//border colision
            newHeadCol--;
            break;

        default:
            break;
    }
    if(!( ((board[newHeadCol][newHeadRow] & 0x0F)==0) || ((board[newHeadCol][newHeadRow] & 0x0F)==APPLE)) ){
        //TODO printear una explosion o algo
        return 2;//snakes colision
    }
    if((board[newHeadCol][newHeadRow] & 0x0F)==APPLE){//apple saves points
        beep(1000, 30);
        (snake==SNAKE1)? player1Points++ : player2Points++;
        board[column][row]=snake + (board[column][row]&0xF0);
        board[newHeadCol][newHeadRow]=snake + dir;
        addApple();
        //imprimimos puntajes
        printPoints(snake);
    }
    else{
        board[newHeadCol][newHeadRow]=snake + dir;
        changePosition(column,row,dir,snake);
    }
    saveHeadPosition(snake,newHeadCol,newHeadRow);
    
    return 0;
}

uint8_t changePosition(uint8_t column, uint8_t row, enum Direction dir, uint8_t snake){
    if(column==getSnakeTailCol(snake) && row==getSnakeTailRow(snake)){//caso base
        board[column][row]=0;
        return 1;//aviso que debe establecerse nuevas posiciones de tail
    }
    uint8_t prevCol=column, prevRow=row;
    enum Direction currentDir = board[column][row] & 0xF0;
    switch(currentDir){
        case (UP):
            prevRow++;
            break;

        case (DOWN):
            prevRow--;
            break;

        case (RIGHT):
            prevCol--;
            break;

        case (LEFT):
            prevCol++;
            break;

        default:
            break;
    }
    if(changePosition(prevCol,prevRow,currentDir,snake)){
        saveTailPosition(snake,column,row);
        board[column][row]=snake+dir;
    }
    return 0;
}

void putSnake(uint8_t row, uint8_t column, uint8_t snake){
    uint8_t caso = checkRight(row,column,snake);
    caso += checkLeft(row,column,snake)<<1;
    caso += checkDown(row,column,snake)<<2;
    caso += checkUp(row,column,snake)<<3;
    enum Direction mainDir=getDirection(row,column);
    if((row==getSnakeHeadRow(snake)) && (column==getSnakeHeadCol(snake))){
        switch(mainDir){
            case(LEFT):
                draw_snakehead_left(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            case(RIGHT):
                draw_snakehead_right(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            case(UP):
                draw_snakehead_up(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            case(DOWN):
                draw_snakehead_down(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            default:
                return;
        }
    }
    if((row==getSnakeTailRow(snake)) && (column==getSnakeTailCol(snake))){
        switch(mainDir){
            case(RIGHT):
                draw_snaketail_left(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            case(LEFT):
                draw_snaketail_right(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            case(DOWN):
                draw_snaketail_up(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            case(UP):
                draw_snaketail_down(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                return;
            default:
                return;
        }
    }
    
    switch(mainDir){
        case (UP):
            if(((caso&0xC)==0xC) && getDirection(row-1,column)==UP){//theres snake up and down
                draw_snakebody_vertical(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
            }
            else{
                if((caso & 0x02) && getDirection(row,column-1)==LEFT){//theres snake left with correct direction
                    draw_snakecurve_downright(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                }
                else{
                    if((caso & 0x01) && getDirection(row,column+1)==RIGHT){//theres snake right
                        draw_snakecurve_downleft(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                    }
                }
            }
            break;

        case (DOWN):
            if(((caso&0xC)==0xC) && getDirection(row+1,column)==DOWN){//theres snake down and up
                draw_snakebody_vertical(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
            }
            else{
                if((caso & 0x02) && getDirection(row,column-1)==LEFT){//theres snake left with correct direction
                    draw_snakecurve_upright(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                }
                else{
                    if((caso & 0x01) && getDirection(row,column+1)==RIGHT){//theres snake right
                        draw_snakecurve_upleft(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                    }
                }
            }
            break;

        case (LEFT):
            if(((caso&0x3)==0x3) && getDirection(row,column-1)==LEFT){//theres snake left and right
                //if(column%2==0) draw_snakebody_horizontal2(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                //else 
                draw_snakebody_horizontal1(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
            }
            else{
                if((caso & 0x08) && getDirection(row-1,column)==UP){//theres snake up with correct direction
                    draw_snakecurve_upleft(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                }
                else{
                    if((caso & 0x04) && getDirection(row+1,column)==DOWN){//theres snake down
                        draw_snakecurve_downleft(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                    }
                }
            }
            break;

        case (RIGHT):
            if(((caso&0x3)==0x3) && getDirection(row,column+1)==RIGHT){//theres snake left and right
                //if(column%2==0) draw_snakebody_horizontal2(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                //else 
                draw_snakebody_horizontal1(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
            }
            else{
                if((caso & 0x08) && getDirection(row-1,column)==UP){//theres snake up with correct direction
                    draw_snakecurve_upright(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                }
                else{
                    if((caso & 0x04) && getDirection(row+1,column)==DOWN){//theres snake down
                        draw_snakecurve_downright(column*(dibSpaceWidth) + board_start_x ,row*(dibSpaceHeight) + board_start_y, getSnakeColor(snake));
                    }
                }
            }
            break;
        
        default:
            break;
    }
}

//================================================================================================================================
//checkers
uint8_t checkUp(uint8_t row, uint8_t column, uint8_t value){
    return (row==0)? 0 : ((board[column][row-1] & 0x0F)==value);
}

uint8_t checkDown(uint8_t row, uint8_t column, uint8_t value){
    return (row==BOARD_H-1)? 0 : ((board[column][row+1] & 0x0F)==value);
}

uint8_t checkLeft(uint8_t row, uint8_t column, uint8_t value){
    return (column==0)? 0 : ((board[column-1][row] & 0x0F)==value);
}

uint8_t checkRight(uint8_t row, uint8_t column, uint8_t value){
    return (column==BOARD_W-1)? 0 : ((board[column+1][row] & 0x0F)==value);
}

//================================================================================================================================
// Main Stuff
//================================================================================================================================
//================================================================================================================================
#define EXIT_KEY 'q'

void Snake(){
    //imprimimos menu principal y se selecciona cant de jugadores.
    uint8_t players=snakeScreen();
    
    //rand init
    srand(time());

    uint8_t exit=0,error=0;//only changes when exit key is pressed to exit the game or a colision happens

    player1Points=0;
    player2Points=0;

    snakecolor1=PLAYER1_DEFAULT_COLOR;
    snakecolor2=PLAYER2_DEFAULT_COLOR;

    char keypressed[1]={0};

    enum Direction lastdir1=NONE, lastdir2=NONE;

    if(initGame()){
        print("size to small!");
        return;
    }

    //bg margins
    for(int i=0; i<(screenWidth/dibSpaceWidth)+1; i++){
        for(int j=0; j<(screenHeight/dibSpaceHeight)+1; j++){
            draw(square[0], SCREEN_BG_MARGIN, dibHeight, i*dibSpaceWidth, j*dibSpaceHeight);
        }
    }

    //imprimimos puntajes y seteamos snakes
    for(int i=0; i<6; i++){
        draw(select_player[i], WHITETEXT, dibHeight, i*dibSpaceWidth, 0);

        if(players) draw(select_player[i], WHITETEXT, dibHeight, screenWidth-(dibSpaceWidth*(8-i)), 0);
    }
    draw(points_digits[1], WHITETEXT, dibHeight, dibSpaceWidth*7, 0);
    
    snakeSetup(SNAKE1);
    printPoints(SNAKE1);

    if(players){
        draw(points_digits[2], WHITETEXT, dibHeight, screenWidth-(dibSpaceWidth), 0);

        snakeSetup(SNAKE2);
        printPoints(SNAKE2);
    }

    addApple();//colocamos la primer manzana


    while(!exit && !error){
        if(readLast(keypressed, 1)>0){
            switch(keypressed[0]){
                case ('q'):
                    exit=1;
                    keypressed[0]=0;
                    break;
                
                case ('a'):
                    if(lastdir1!=RIGHT){//illegal dir
                        lastdir1= (lastdir1==NONE)? RIGHT : LEFT ;
                    }
                    break;

                case ('s'):
                    if(lastdir1!=UP){//illegal dir
                        lastdir1=DOWN;
                    }
                    break;

                case ('d'):
                    if(lastdir1!=LEFT){//illegal dir
                        lastdir1=RIGHT;
                    }
                    break;

                case ('w'):
                    if(lastdir1!=DOWN){// illegal dir
                        lastdir1=UP;
                    }
                    break;

                case ('j'):
                    if(players == 2 && lastdir2!=RIGHT){//illegal dir
                        lastdir2=LEFT ;
                    }
                    break;

                case ('k'):
                    if(players == 2 && lastdir2!=UP){//illegal dir
                        lastdir2=DOWN;
                    }
                    break;

                case ('l'):
                    if(players == 2 && lastdir2!=LEFT){//illegal dir
                        lastdir2= (lastdir2==NONE)? LEFT : RIGHT ;
                    }
                    break;

                case ('i'):
                    if(players == 2 && lastdir2!=DOWN){// illegal dir
                        lastdir2=UP;
                    }
                    break;

                default:
                    break;
            }
        }
        if(lastdir1!=NONE) error += slither(lastdir1,SNAKE1);
        tablero();
        if(players == 2 && lastdir2!=NONE && !error) error += slither(lastdir2,SNAKE2);
        tablero();
        sleep_once();
    }

    //GAME OVER message
    for(int i=0; i<(screenWidth/dibSpaceWidth)+1; i++){
        for(int j=0; j<(screenHeight/dibSpaceHeight)+1; j++){
            draw(square[0], SCREEN_BG_MARGIN, dibHeight, i*dibSpaceWidth, j*dibSpaceHeight);
        }
    }


    for(int i=0; i<9; i++){
        draw(gameover_text[i], WHITETEXT, dibHeight, (screenWidth/2)-((dibSpaceWidth*9)/2)+(i*dibSpaceWidth), (screenHeight/2)-(dibSpaceHeight/2));
    }

    for(int i=0; i<6; i++){
        draw(select_player[i], WHITETEXT, dibHeight, i*dibSpaceWidth, 0);

        if(players) draw(select_player[i], WHITETEXT, dibHeight, screenWidth-(dibSpaceWidth*(8-i)), 0);
    }

    draw(points_digits[1], WHITETEXT, dibHeight, dibSpaceWidth*7, 0);
    

    printPoints(SNAKE1);
    if(players){
        draw(points_digits[2], WHITETEXT, dibHeight, screenWidth-(dibSpaceWidth), 0);
        printPoints(SNAKE2);
    }
    
    beep(1000, 100);
    sleep(100, 1);
    beep(800, 100);
    sleep(100, 1);
    beep(600, 100);
    sleep(3, 0);


    return;
}

//================================================================================================================================
//setup

void snakeSetup(uint8_t snake){

    if(snake==SNAKE1){
        addSnake(middleBoard_y,2,snake,RIGHT);
        addSnake(middleBoard_y,3,snake,RIGHT);
        addSnake(middleBoard_y,4,snake,RIGHT);

        saveHeadPosition(snake, 4, middleBoard_y);
        saveTailPosition(snake, 2, middleBoard_y);
    }
    else{
        addSnake(middleBoard_y,BOARD_W-4,snake,LEFT);
        addSnake(middleBoard_y,BOARD_W-3,snake,LEFT);
        addSnake(middleBoard_y,BOARD_W-2,snake,LEFT);
        
        saveHeadPosition(snake, BOARD_W-4, middleBoard_y);
        saveTailPosition(snake, BOARD_W-2, middleBoard_y);
    }


    tablero();//print the starting board
}

//================================================================================================================================
//setters

void saveHeadPosition(uint8_t snake, uint8_t column, uint8_t row){
    if(snake==SNAKE1){
        snake1_head_column=column;
        snake1_head_row=row;
    }
    else{
        snake2_head_column=column;
        snake2_head_row=row;
    }
}
void saveTailPosition(uint8_t snake, uint8_t column, uint8_t row){
    if(snake==SNAKE1){
        snake1_tail_column=column;
        snake1_tail_row=row;
    }
    else{
        snake2_tail_column=column;
        snake2_tail_row=row;
    }
}

//================================================================================================================================
//getters

enum Direction getDirection(int row, int column){
    if(row<0 || row>=BOARD_H || column<0 || column>=BOARD_W) return NONE;//out of bounds
    uint8_t caso = board[column][row] & 0xF0;//me quedo con los bits superiores, la direccion
    switch(caso){
        case (UP):
            return UP;

        case (DOWN):
            return DOWN;

        case (RIGHT):
            return RIGHT;

        case (LEFT):
            return LEFT;

        default:
            return NONE;
    }
}

uint8_t getSnakeTailCol(uint8_t snake){
    return (snake==SNAKE1)? snake1_tail_column:snake2_tail_column;
}

uint8_t getSnakeTailRow(uint8_t snake){
    return (snake==SNAKE1)? snake1_tail_row:snake2_tail_row;
}

uint8_t getSnakeHeadCol(uint8_t snake){
    return (snake==SNAKE1)? snake1_head_column:snake2_head_column;
}

uint8_t getSnakeHeadRow(uint8_t snake){
    return (snake==SNAKE1)? snake1_head_row:snake2_head_row;
}

uint32_t getSnakeColor(uint8_t snake){
    return (snake==SNAKE1)? snakecolor1:snakecolor2;
}
