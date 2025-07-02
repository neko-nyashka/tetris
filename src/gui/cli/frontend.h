#ifndef FRONTEND_H
#define FRONTEND_H
#define COLOR_BISSQUE 8
#define COLOR_BABY_BLUE 9
#define COLOR_PINK 10
#define COLOR_MINT 11
#define COLOR_LAVENDER 12
#define COLOR_PEACH 13
#define COLOR_POWDER 14
#define COLOR_GREY 15

#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#include "../../brick_game/tetris/backend.h"
#include "../../brick_game/tetris/game.h"
#ifdef __linux__
#include <sys/random.h>
#endif

void initNcurses();
void initColors();
void createWindows(WINDOW **fwin, WINDOW **swin);
UserAction_t getButton(int userInput);
void DrawSideBar(WINDOW *side_win, GameInfo_t info);
void DrawGameField(WINDOW *game_win, GameInfo_t info);
void processInput(UserAction_t *action, bool *running);
void applyGravity(int *delay, const GameInfo_t *gi);
void render(WINDOW *field_win, WINDOW *side_win, GameInfo_t *gi);

#endif