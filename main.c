#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "curses/curses.h"
#include <inttypes.h>

//#include <unistd.h>

#define MIN_Y  2
enum {LEFT=1, UP, RIGHT, DOWN, STOP_GAME=KEY_F(10)};
enum {MAX_TAIL_SIZE=100, START_TAIL_SIZE=3, MAX_FOOD_SIZE=20, FOOD_EXPIRE_SECONDS=10};

#define CONTROLS 5 // число символов для управления
// Здесь храним коды управления змейкой
struct control_buttons
{
    int down;
    int up;
    int left;
    int right;
} control_buttons;

struct control_buttons default_controls[CONTROLS] = { {KEY_DOWN, KEY_UP, KEY_LEFT, KEY_RIGHT},
                                                        {0x73, 0x77, 0x61, 0x64}, 
                                                        {0x53, 0x57, 0x41, 0x44},
                                                        {0xFFFFFFEB, 0xFFFFFFE6, 0xFFFFFFE4, 0xFFFFFFA2},
                                                        {0xFFFFFF9B, 0xFFFFFF96, 0xFFFFFF94, 0xFFFFFF82} };

/*
 Голова змейки содержит в себе
 x,y - координаты текущей позиции
 direction - направление движения
 tsize - размер хвоста
 *tail -  ссылка на хвост
 */
typedef struct snake_t
{
    int x;
    int y;
    int direction;
    size_t tsize;
    struct tail_t *tail;
    struct control_buttons * controls;
} snake_t;

/*
 Хвост это массив состоящий из координат x,y
 */
typedef struct tail_t
{
    int x;
    int y;
} tail_t;

void initTail(struct tail_t t[], size_t size)
{
    struct tail_t init_t={0,0};
    for(size_t i=0; i<size; i++)
    {
        t[i]=init_t;
    }
}

void initHead(struct snake_t *head, int x, int y)
{
    head->x = x;
    head->y = y;
    head->direction = RIGHT;
}

void initSnake(snake_t *head, size_t size, int x, int y)
{
    tail_t*  tail  = (tail_t*) malloc(MAX_TAIL_SIZE*sizeof(tail_t));
    initTail(tail, MAX_TAIL_SIZE);
    initHead(head, x, y);
    head->tail = tail; // прикрепляем к голове хвост
    head->tsize = size+1;
    head->controls = default_controls;
}

/*
 Движение головы с учетом текущего направления движения
 */
void go(struct snake_t *head)
{
    char ch = '@';
    int max_x=0, max_y=0;
    getmaxyx(stdscr, max_y, max_x); // macro - размер терминала
    mvprintw(head->y, head->x, " "); // очищаем один символ
    switch (head->direction)
    {
        case LEFT:
            if(head->x <= 0) // Циклическое движение, чтобы не уходить за пределы экрана
                head->x = max_x;
            mvprintw(head->y, --(head->x), "%c", ch);
            break;
        case RIGHT:
            if(head->x >= max_x) // Циклическое движение, чтобы не уходить за пределы экрана
                head->x = 0;
            mvprintw(head->y, ++(head->x), "%c", ch);
            break;
        case UP:
            if(head->y <= MIN_Y) // Циклическое движение, чтобы не уходить за пределы экрана
                head->y = max_y;
            mvprintw(--(head->y), head->x, "%c", ch);
            break;
        case DOWN:
            if(head->y >= max_y) // Циклическое движение, чтобы не уходить за пределы экрана
                head->y = MIN_Y;
            mvprintw(++(head->y), head->x, "%c", ch);
            break;
        default:
            break;
    }
    refresh();
}

/*
 Движение хвоста с учетом движения головы
*/
void goTail(struct snake_t *head)
{
    char ch = '*';
    mvprintw(head->tail[head->tsize-1].y, head->tail[head->tsize-1].x, " ");
    for(size_t i = head->tsize-1; i>0; i--)
    {
        head->tail[i] = head->tail[i-1];
        if( head->tail[i].y || head->tail[i].x)
            mvprintw(head->tail[i].y, head->tail[i].x, "%c", ch);
    }
    head->tail[0].x = head->x;
    head->tail[0].y = head->y;
}

/* 
функция проверки корректности выбранного направляения. 
Служит для запрета двигаться сразу в противоположном направлении 
*/
int checkDirection(snake_t* snake, int32_t key, int32_t keySetting){
    if((snake->direction == DOWN) != (key == snake->controls[keySetting].up))
        return 1;
    else if((snake->direction == UP) != (key == snake->controls[keySetting].down))
        return 1;
    else if((snake->direction == RIGHT) != (key == snake->controls[keySetting].left))
        return 1;
    else if((snake->direction == LEFT) != (key == snake->controls[keySetting].right))
        return 1;

    return 0;
}

/*
Функция для выбора направления движения змейки
*/
void changeDirection(struct snake_t* snake, const int32_t key)
{
    mvprintw(1, 0,"key - %x , direction - %d", key, snake->direction);
    for(int i = 0; i < CONTROLS; i++){
        if (key == snake->controls[i].down){
            if(checkDirection(snake, key, i))
                snake->direction = DOWN;
        }
        else if (key == snake->controls[i].up){
            if(checkDirection(snake, key, i))
                snake->direction = UP;
        }
        else if (key == snake->controls[i].right){
            if(checkDirection(snake, key, i))
                snake->direction = RIGHT;
        }
        else if (key == snake->controls[i].left){
            if(checkDirection(snake, key, i))
                snake->direction = LEFT;
        }
    }
}

int main()
{
    snake_t* snake = (snake_t*)malloc(sizeof(snake_t));
    initSnake(snake,START_TAIL_SIZE,10,10);
    initscr();
    keypad(stdscr, TRUE); // Включаем F1, F2, стрелки и т.д.
    raw();                // Откдючаем line buffering
    noecho();            // Отключаем echo() режим при вызове getch
    curs_set(FALSE);    //Отключаем курсор
    mvprintw(0, 0,"Use arrows for control. Press 'F10' for EXIT");
    timeout(0);    //Отключаем таймаут после нажатия клавиши в цикле
    int key_pressed=0;
    while( key_pressed != STOP_GAME )
    {
        key_pressed = getch(); // Считываем клавишу
        go(snake);
        goTail(snake);
        timeout(100); // Задержка при отрисовке
        changeDirection(snake, key_pressed);
    }
    free(snake->tail);
    free(snake);
    endwin(); // Завершаем режим curses mod
    return 0;
}