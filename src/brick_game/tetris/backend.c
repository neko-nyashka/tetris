#include "backend.h"

/**
 * @brief Копирует форму и цвет из следующей фигуры в текущую.
 *
 * @param gc Указатель на контекст игры.
 */
void nextCurrentInit(GameContext_t *gc) {
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      gc->current.shape[i][j] = gc->info.next[i][j];
      if (gc->info.next[i][j]) {
        gc->current.color = gc->info.next[i][j];
      }
    }
  }
}

/**
 * @brief Сохраняет рекордный счёт в файл record.txt.
 *
 * @param score Новый счёт для сохранения.
 */
void saveHighScore(int score) {
  FILE *file = fopen("record.txt", "w");
  if (file) {
    fprintf(file, "%d", score);
    fclose(file);
  }
}

/**
 * @brief Проверяет, не выходит ли текущая фигура за границы поля и не
 * пересекается ли с уже занятыми ячейками.
 *
 * @param gc Указатель на контекст игры.
 * @return 1, если перемещение/вращение возможно; 0 — в противном случае.
 */
int checkCollision(GameContext_t *gc) {
  int is_possible = 1;
  for (int i = 0; i < FIGURE_SIZE && is_possible; i++) {
    for (int j = 0; j < FIGURE_SIZE && is_possible; j++) {
      if (gc->current.shape[i][j]) {
        int abs_x = gc->current.x + j;
        int abs_y = gc->current.y + i;
        if (abs_x < 0 || abs_x >= FIELD_WIDTH || abs_y < 0 ||
            abs_y >= FIELD_HEIGHT) {
          is_possible = 0;
        }
        if (is_possible && abs_y >= 0 && gc->info.field[abs_y][abs_x] != 0) {
          is_possible = 0;
        }
      }
    }
  }
  return is_possible;
}

/**
 * @brief Проверяет, можно ли опустить фигуру на одну строку вниз.
 *
 * @param gc Указатель на контекст игры.
 * @return 1, если падение возможно; 0 — иначе.
 */
int canFall(GameContext_t *gc) {
  gc->current.y += 1;
  int is_possible = checkCollision(gc);
  gc->current.y -= 1;
  return is_possible;
}

/**
 * @brief Поворачивает текущую фигуру на 90° по часовой стрелке.
 *
 * @param gc Указатель на контекст игры.
 */
void rotateTetromino(GameContext_t *gc) {
  int temp_shape[FIGURE_SIZE][FIGURE_SIZE];
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      temp_shape[i][j] = gc->current.shape[i][j];
    }
  }
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      gc->current.shape[j][FIGURE_SIZE - 1 - i] = temp_shape[i][j];
    }
  }
}

/**
 * @brief Отрисовывает текущую фигуру на поле, записывая её цвет в ячейки.
 *
 * @param gc Указатель на контекст игры.
 */
void drawFigure(GameContext_t *gc) {
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      if (gc->current.shape[i][j]) {
        int abs_x = gc->current.x + j;
        int abs_y = gc->current.y + i;
        if (abs_y >= 0 && abs_x >= 0 && abs_x < FIELD_WIDTH &&
            abs_y < FIELD_HEIGHT) {
          gc->info.field[abs_y][abs_x] = gc->current.color;
        }
      }
    }
  }
}

/**
 * @brief Очищает ячейки, занятые текущей фигурой, устанавливая их в 0.
 *
 * @param gc Указатель на контекст игры.
 */
void clearFigure(GameContext_t *gc) {
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      if (gc->current.shape[i][j]) {
        int abs_x = gc->current.x + j;
        int abs_y = gc->current.y + i;
        if (abs_y >= 0 && abs_y < FIELD_HEIGHT && abs_x >= 0 &&
            abs_x < FIELD_WIDTH) {
          gc->info.field[abs_y][abs_x] = 0;
        }
      }
    }
  }
}

/**
 * @brief Пытается заспавнить текущую фигуру: проверяет коллизию и отрисовывает,
 * если возможно.
 *
 * @param gc Указатель на контекст игры.
 * @return 1, если спавн возможен; 0 — иначе.
 */
int trySpawnFigure(GameContext_t *gc) {
  int is_possible = checkCollision(gc);
  if (is_possible) {
    drawFigure(gc);
  }
  return is_possible;
}

/**
 * @brief Автоматически перемещает фигуру вниз на одну позицию.
 *        Если касается препятствия — фиксирует фигуру и переключает состояние
 * на очистку линий.
 *
 * @param gc Указатель на контекст игры.
 */
void autoMoveDown(GameContext_t *gc) {
  if (gc->state == STATE_FALLING) {
    clearFigure(gc);
    gc->current.y += 1;
    if (!checkCollision(gc)) {
      gc->current.y -= 1;
      drawFigure(gc);
      gc->state = STATE_CLEARING;
    } else {
      drawFigure(gc);
    }
  }
}

/**
 * @brief Очищает заполненные линии, сдвигает всё сверху вниз, обновляет счёт,
 * уровень и скорость. Сохраняет новый рекорд, если он побит.
 *
 * @param gc Указатель на контекст игры.
 */
void clearLines(GameContext_t *gc) {
  int counter = 0;
  for (int i = FIELD_HEIGHT - 1; i >= 0; --i) {
    int full_line = 1;
    for (int j = 0; j < FIELD_WIDTH && full_line; ++j) {
      if (gc->info.field[i][j] == 0) {
        full_line = 0;
      }
    }
    if (full_line) {
      counter++;
      for (int k = i; k > 0; k--) {
        for (int j = 0; j < FIELD_WIDTH; j++) {
          gc->info.field[k][j] = gc->info.field[k - 1][j];
        }
      }
      for (int j = 0; j < FIELD_WIDTH; j++) {
        gc->info.field[0][j] = 0;
      }
      i++;
    }
  }
  switch (counter) {
    case 1:
      gc->info.score += 100;
      break;
    case 2:
      gc->info.score += 300;
      break;
    case 3:
      gc->info.score += 700;
      break;
    case 4:
      gc->info.score += 1500;
      break;
  }
  gc->info.level = 1 + ((gc->info.score / 600)) % 10;
  gc->info.speed = 1000 - (gc->info.level - 1) * 100;
  if (gc->info.score > gc->info.high_score) {
    gc->info.high_score = gc->info.score;
    saveHighScore(gc->info.score);
  }
}

/**
 * @brief Молниеносно опускает фигуру до последней доступной позиции и фиксирует
 * её на поле.
 *
 * @param gc Указатель на контекст игры.
 */
void dropTetromino(GameContext_t *gc) {
  int original_y = gc->current.y;
  int max_drop = 0;
  int collision = 0;
  while (!collision) {
    max_drop++;
    gc->current.y = original_y + max_drop;
    if (!checkCollision(gc)) {
      collision = 1;
      max_drop--;
    }
  }
  gc->current.y = original_y + max_drop;
  drawFigure(gc);
}

/**
 * @brief Инициализирует следующий случайный тетромино.
 *
 * @param gc Указатель на контекст игры.
 */
void nextFigureInit(GameContext_t *gc) {
  const int tetromino_shapes[7][4][4] = {
      {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 2, 2, 0}, {0, 2, 2, 0}, {0, 0, 0, 0}},
      {{0, 3, 0, 0}, {3, 3, 3, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{4, 0, 0, 0}, {4, 4, 4, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 5, 0}, {5, 5, 5, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 6, 6, 0}, {6, 6, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{7, 7, 0, 0}, {0, 7, 7, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};
  int next_shape = rand() % 7;
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      gc->info.next[i][j] = tetromino_shapes[next_shape][i][j];
    }
  }
}

/**
 * @brief Возвращает статический контекст игры, создавая и инициализируя его при
 * первом вызове.
 *
 * @return Указатель на одно-единственный экземпляр GameContext_t.
 */
GameContext_t *getContext() {
  static GameContext_t gc;
  static int is_init = 0;
  if (!is_init) {
    is_init = 1;
    memset(&gc, 0, sizeof(gc));
    gc.info.level = 1;
    gc.info.score = 0;
    gc.info.speed = 1000;
    gc.info.pause = 0;
    gc.state = STATE_START;
    gc.info.field = malloc(FIELD_HEIGHT * sizeof(int *));
    for (int i = 0; i < FIELD_HEIGHT; ++i) {
      gc.info.field[i] = calloc(FIELD_WIDTH, sizeof(int));
    }
    gc.info.next = malloc(FIGURE_SIZE * sizeof(int *));
    for (int i = 0; i < FIGURE_SIZE; i++) {
      gc.info.next[i] = calloc(FIGURE_SIZE, sizeof(int));
    }
    FILE *f = fopen("record.txt", "r");
    if (f) {
      fscanf(f, "%d", &gc.info.high_score);
      fclose(f);
    }
    nextFigureInit(&gc);
    nextCurrentInit(&gc);
    nextFigureInit(&gc);
    gc.current.rotation = 0;
    gc.current.y = 0;
    gc.current.x = FIELD_WIDTH / 2 - 2;
  }
  return &gc;
}

/**
 * @brief Обрабатывает логику падающей фигуры в зависимости от действия
 * пользователя.
 *
 * @param gc     Указатель на контекст игры.
 * @param action Действие пользователя (Left, Right, Down и т.д.).
 */
void fallingHandler(GameContext_t *gc, UserAction_t action) {
  if (action == Left) {
    clearFigure(gc);
    gc->current.x -= 1;
    if (!checkCollision(gc)) gc->current.x += 1;
    drawFigure(gc);
  } else if (action == Right) {
    clearFigure(gc);
    gc->current.x += 1;
    if (!checkCollision(gc)) gc->current.x -= 1;
    drawFigure(gc);
  } else if (action == Down) {
    clearFigure(gc);
    dropTetromino(gc);
    gc->state = STATE_CLEARING;
  } else if (action == Action) {
    clearFigure(gc);
    int original_shape[FIGURE_SIZE][FIGURE_SIZE];
    memcpy(original_shape, gc->current.shape, sizeof(original_shape));
    rotateTetromino(gc);
    if (!checkCollision(gc))
      memcpy(gc->current.shape, original_shape, sizeof(original_shape));
    drawFigure(gc);
  } else if (action == Pause) {
    gc->state = STATE_PAUSED;
    gc->info.pause = 1;
  } else if (action == Terminate) {
    gc->state = STATE_GAME_OVER;
    gc->info.pause = 2;
  } else if (action == Up) {
    autoMoveDown(gc);
  }
}

/**
 * @brief Обрабатывает пользовательский ввод, переключая состояние игры.
 *
 * @param action Действие пользователя (Start, Left, Right и т.д.).
 */
void userInputHandler(UserAction_t action) {
  GameContext_t *gc = getContext();
  if (gc->state == STATE_START) {
    if (action == Start) {
      gc->state = STATE_SPAWN;
    }
  } else if (gc->state == STATE_SPAWN) {
    nextCurrentInit(gc);
    nextFigureInit(gc);
    gc->current.y = 0;
    gc->current.x = FIELD_WIDTH / 2 - 2;
    if (trySpawnFigure(gc)) {
      gc->state = STATE_FALLING;
    } else {
      gc->state = STATE_GAME_OVER;
      gc->info.pause = 2;
    }
  } else if (gc->state == STATE_FALLING) {
    fallingHandler(gc, action);
  } else if (gc->state == STATE_PAUSED) {
    if (action == Pause) {
      gc->state = STATE_FALLING;
      gc->info.pause = 0;
    }
  } else if (gc->state == STATE_CLEARING) {
    clearLines(gc);
    gc->state = STATE_SPAWN;
  } else if (gc->state == STATE_GAME_OVER) {
    gameOver(gc);
  }
}

/**
 * @brief Обрабатывает окончание игры: сбрасывает поле, очищает буфер следующей
 * фигуры и сохраняет рекорд.
 *
 * @param gc Указатель на контекст игры.
 */
void gameOver(GameContext_t *gc) {
  if (gc) {
    if (gc->info.score > gc->info.high_score) {
      saveHighScore(gc->info.score);
      gc->info.high_score = gc->info.score;
    }
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
    }
    for (int i = 0; i < FIGURE_SIZE; i++) {
      memset(gc->info.next[i], 0, FIGURE_SIZE * sizeof(int));
    }
  }
}
