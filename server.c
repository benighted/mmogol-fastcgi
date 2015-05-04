#include <fcgi_stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

int width = 64, height = 32;
int depth = sizeof(int) * 8;
unsigned int *cells, *livery;


void writeLog(char* message);

void *startGame();
void printGame();
void evalCells();
void applyCellNextStates();

int getState(unsigned int *values, int x, int y);
int getCellState(int x, int y);
int getCellNextState(int x, int y);
void setState(unsigned int *values, int x, int y, int value);
void setCellState(int x, int y, int value);
void setCellNextState(int x, int y, int value);


int main(int argc, char *argv[]) {
    srand(time(NULL));

    // start game
    writeLog("Starting game...");
    pthread_t pth;
    pthread_create(&pth, NULL, startGame, NULL);

    // accept connections
    writeLog("Waiting for connections...");
    while(FCGI_Accept() >= 0) {
        printf("Content-Type: text/plain");
        printf("\r\n\r\n");

        printGame();
    }
    writeLog("Shutting down...");

    return 0;
}

void writeLog(char *message) {
    FILE *file;

    file = fopen("server.log", "a");

    if (file) {
        fputs(message, file);
        fputs("\n", file);
        fclose(file);
    } else {
        perror("Unable to open log file.");
        exit(1);
    }
}

void *startGame() {
    int count = width * height;

    // allocate space for cell state data
    cells = calloc(count, sizeof(unsigned int));
    livery = calloc(count, sizeof(unsigned int));

    int i;
    for (i = 0; i < count; i++) {
        // randomize initial cell states
        cells[i] = 0;//(rand() * 5) % UINT_MAX;
    }

// glider
setCellState(11,28,1);
setCellState(12,29,1);
setCellState(10,30,1);
setCellState(11,30,1);
setCellState(12,30,1);

// blinker
setCellState(-1, 31, 1);
setCellState(0, 31, 1);
setCellState(1, 31, 1);

sleep(2);

    while (1) {
        evalCells();
        applyCellNextStates();
        usleep(1000000);
    }

    free(cells);

    return 0;
}

int getPos(int x, int y) {
    return (x + (y * width)) / depth;
}

int getOffset(int x, int y) {
    return (x + (y * width)) % depth;
}

int getState(unsigned int *values, int x, int y) {
    return (values[getPos(x, y)] & (1 << getOffset(x, y))) ? 1 : 0;
}

int getCellState(int x, int y) {
    // wrap to left if under limit
    while (x < 0) x += width;
    while (y < 0) y += height;

    // wrap to right if over limit
    if (x >= width) x %= width;
    if (y >= height) y %= height;

    return getState(cells, x, y);
}

int getCellNextState(int x, int y) {
    // wrap to left if under limit
    while (x < 0) x += width;
    while (y < 0) y += height;

    // wrap to right if over limit
    if (x >= width) x %= width;
    if (y >= height) y %= height;

    return getState(livery, x, y);
}

void setState(unsigned int *values, int x, int y, int value) {
    values[getPos(x, y)] = value
        ? values[getPos(x, y)] | (1 << getOffset(x, y))
        : values[getPos(x, y)] & ~(1 << getOffset(x, y));
}

void setCellState(int x, int y, int value) {
    // wrap to left if under limit
    while (x < 0) x += width;
    while (y < 0) y += height;

    // wrap to right if over limit
    if (x >= width) x %= width;
    if (y >= height) y %= height;

    setState(cells, x, y, value);
}

void setCellNextState(int x, int y, int value) {
    // wrap to left if under limit
    while (x < 0) x += width;
    while (y < 0) y += height;

    // wrap to right if over limit
    if (x >= width) x %= width;
    if (y >= height) y %= height;

    setState(livery, x, y, value);
}

void applyCellNextStates() {
    int x, y;

    // copy pending cells to board
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            setCellState(x, y, getCellNextState(x, y));
        }
    }
}

void evalCells() {
    int x, x2, y, y2, count;

    for (y = 0; y < height + 2; y++) {
        if (y < height) for (x = 0; x < width; x++) {
            // count adjacent live cells
            count = 0;
            for (x2 = -1; x2 <= 1; x2++) {
                for (y2 = -1; y2 <= 1; y2++) {
                    if (!(x2 || y2)) continue; // skip self
                    if (getCellState(x + x2, y + y2)) count++;
                }
            }

            // evaluate next generation
            if (getCellState(x, y)) {
                setCellNextState(x, y, count == 2 || count == 3);
            } else {
                setCellNextState(x, y, count == 3);
            }
        }
    }
}

void printGame() {
    char buffer[width + 3];
    int x, y;

    // print header row
    buffer[0] = '*';
    buffer[1] = 32;
    for (x = 0; x < width; x++) {
        buffer[x + 2] = (x % 10 ? x % 10 : x / 10) + 48;
    }
    buffer[x + 2] = 0;
    printf("%s\n", buffer);

    // print game board
    for (y = 0; y < height; y++) {
        buffer[0] = (y % 10 ? y % 10 : y / 10) + 48;
        buffer[1] = 32;
        for (x = 0; x < width; x++) {
            buffer[x + 2] = getCellState(x, y) ? 'O' : ' '; 
        }
        buffer[x + 2] = 0;
        printf("%s\n", buffer);
    }
}
