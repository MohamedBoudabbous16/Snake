#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

// Characters visually represent game elements
#define SnakeBody '0'
#define SnakeHead '@'
#define FoodChar '#'
#define EmptyCell ' '

//Global variables
int numbLines = 0;
int numbCols = 0;
int WrapArroundMode = 0;
char **grid = NULL;

struct Cell {
    int line;
    int column;
    struct Cell *next;
};
struct Direction {
    int line;
    int column;
};
struct Snake {
    struct Cell *head;
    struct Cell *tail;
};
typedef struct Snake Snake;
typedef struct Direction Direction;
typedef struct Cell Cell;

//Initialization and Cleanup:
void initGrid() {
    grid = malloc(numbLines * sizeof(char*));
    if (grid == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation failed for grid rows.\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < numbLines; i++) {
        grid[i] = malloc(numbCols * sizeof(char));
        if (grid[i] == NULL) {
            endwin();
            fprintf(stderr, "Memory allocation failed for grid[%d].\n", i);
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < numbCols; j++) {
            grid[i][j] = EmptyCell;
        }
    }
};
Snake createSnake() {
    Snake snake;
    Cell *cell1 = malloc(sizeof(Cell));
    Cell *cell2 = malloc(sizeof(Cell));
    Cell *cell3 = malloc(sizeof(Cell));

    cell1->line = 1;
    cell1->column = 2;
    cell1->next = cell2;

    cell2->line = 1;
    cell2->column = 3;
    cell2->next = cell3;

    cell3->line = 1;
    cell3->column = 4;
    cell3->next = NULL;

    grid[cell1->line][cell1->column] = SnakeBody;
    grid[cell2->line][cell2->column] = SnakeBody;
    grid[cell3->line][cell3->column] = SnakeHead;

    snake.head = cell3;
    snake.tail = cell1;
    return snake;
}


void clean(Snake *snake)
{
//je libere les cellules occupes par le seprent
    Cell *curr = snake->head;
    while (curr != NULL) {
        Cell *next = curr->next;
        free(curr);
        curr = next;
    }
//cette fonction ne s'occupe pas que de la liberation du snake
//je libere la grille
    for (int i = 0; i<numbLines; i++) {
        free(grid[i]);
    }
    free(grid);//libere mem
    grid =NULL;//libere pointeur

};
//Display:
void displayGrid(Snake *snake){
    for (int i = 0; i<numbLines;i++) {
        for (int j = 0 ; j<numbCols; j++) {
            move(i,j);
            addch(grid[i][j]);
        }
    }
    refresh();
};

void printGameOver() {
    int middleY= numbLines/2;
    int middleX= numbCols/2-5;
    mvprintw(middleY,middleX,"You lose 😃");
    mvprintw(middleY + 1, middleX - 2, "Press any key to exit");
    refresh();
    timeout(-1);
    getch();
};


//Game Control:
bool isOutOfBounds(int line, int column) {
    return (line<0 || line>= numbLines || column<0 || column>= numbCols);
};
bool isCellEmpty(int line, int column) {
    return (grid[line][column] == EmptyCell);
};
void wrapCoordinates(int *line, int *column) {
//serpent au dessus de la grille, trsp au bas:
    if (*line < 0) {
        *line = numbLines -1;
    } else if (*line >= numbLines) {
        *line = 0;
    }
    if (*column < 0) {
        *column = numbCols -1;
    }else if (*column >= numbCols) {
        *column = 0;
    }
};//utilisation de pointeurs pour travailler directement dans les copies
//appel juste avant de dplacer la tete

//Food Management:
void generateFood() {
    int line,column;
    do {
        line = rand()%numbLines;
        column = rand()%numbCols;
    }while (!isCellEmpty(line,column));
    grid[line][column] = FoodChar;
};
bool isFoodAt(int line, int column) {
    return (grid[line][column] == FoodChar);
};

//Snake Mechanics:
void addHead(Snake *snake, int line, int column) {
    Cell *head = malloc(sizeof(Cell));
    //mecanisme de renforcement:
    if (head == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation failed for the head\n");
        exit(EXIT_FAILURE);
    }
    head->line = line;
    head->column = column;
    head->next = snake->head;

    grid[snake->head->line][snake->head->column]=SnakeBody;
    grid[line][column] = SnakeHead;
    snake->head = head;
};
void removeTail(Snake *snake) {
    if (snake->tail == NULL || snake->tail == snake->head) return;

    Cell *prev = NULL;
    Cell *current = snake->head;

    while (current != NULL && current->next != snake->tail) {
        current = current->next;
    }

    if (current != NULL) {
        if (snake->tail->line >= 0 && snake->tail->line < numbLines &&
            snake->tail->column >= 0 && snake->tail->column < numbCols) {
            grid[snake->tail->line][snake->tail->column] = EmptyCell;
            }

        free(snake->tail);
        snake->tail = current;
        snake->tail->next = NULL;
    }
}



bool isCollision(Snake *snake, int line, int column) {
    Cell *current = snake->head;
    while (current != NULL) {
        if (current->line == line && current->column == column) {
            return true;
        }
        current = current->next;
    }
    return false;
}
int main(int argc, char *argv[]) {
    initscr();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    timeout(100);

    getmaxyx(stdscr, numbLines, numbCols);
    if (argc > 1) WrapArroundMode = 1;

    initGrid();
    Snake snake = createSnake();
    Direction dir = {0, 1};

    srand(time(NULL));
    generateFood();

    bool gameOver = false;

    while (!gameOver) {
        int ch = getch();

        if (ch == KEY_UP && dir.line != 1) {
            dir.line = -1;
            dir.column = 0;
        } else if (ch == KEY_DOWN && dir.line != -1) {
            dir.line = 1;
            dir.column = 0;
        } else if (ch == KEY_LEFT && dir.column != 1) {
            dir.line = 0;
            dir.column = -1;
        } else if (ch == KEY_RIGHT && dir.column != -1) {
            dir.line = 0;
            dir.column = 1;
        }

        int nextLine = snake.head->line + dir.line;
        int nextCol = snake.head->column + dir.column;

        if (WrapArroundMode) wrapCoordinates(&nextLine, &nextCol);

        if (isOutOfBounds(nextLine, nextCol) || isCollision(&snake, nextLine, nextCol)) {
            gameOver = true;
            break;
        }

        bool ate = isFoodAt(nextLine, nextCol);
        addHead(&snake, nextLine, nextCol);
        if (!ate) removeTail(&snake);
        else generateFood();

        erase();
        displayGrid(&snake);
    }
    printf("Game over reached.\n");

    printGameOver();
    clean(&snake);
    endwin();
    return 0;
}
//pour compiler: cc /Users/bouda/Desktop/Cfiles/Snake.c -o Snake -lncurses


//Utility Functions:
