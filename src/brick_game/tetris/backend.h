#ifndef BACKEND_H
#define BACKEND_H
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "game.h"

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define FIGURE_SIZE 4

typedef enum TetrisState_t {
  STATE_START,
  STATE_SPAWN,
  STATE_FALLING,
  STATE_PAUSED,
  STATE_CLEARING,
  STATE_GAME_OVER
} TetrisState_t;

typedef struct Tetromino_t {
  int shape[FIGURE_SIZE][FIGURE_SIZE];
  int x, y;
  int rotation;
  int color;
} Tetromino_t;

typedef struct GameContext_t {
  GameInfo_t info;
  TetrisState_t state;
  Tetromino_t current;

} GameContext_t;

void nextFigureInit(GameContext_t *gc);
GameContext_t *getContext();
void userInputHandler(UserAction_t action);
int trySpawnFigure(GameContext_t *gc);
int checkCollision(GameContext_t *gc);
void drawFigure(GameContext_t *gc);
void clearFigure(GameContext_t *gc);
void nextCurrentInit(GameContext_t *gc);
void dropTetromino(GameContext_t *gc);
void clearLines(GameContext_t *gc);
void rotateTetromino(GameContext_t *gc);
void gameOver(GameContext_t *gc);
void clearLines(GameContext_t *gc);
void fallingHandler(GameContext_t *gc, UserAction_t action);
void autoMoveDown(GameContext_t *gc);
int canFall(GameContext_t *gc);
void saveHighScore(int score);

#endif