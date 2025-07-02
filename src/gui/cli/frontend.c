#include "frontend.h"
/**
 * @brief Инициализирует режим ncurses.
 *
 * Устанавливает локаль, запускает ncurses, отключает отображение вводимых
 * символов, скрывает курсор, включает поддержку функциональных клавиш и делает
 * ввод неблокирующим.
 */
void initNcurses() {
  setlocale(LC_ALL, "");
  initscr();
  clear();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
}

/**
 * @brief Инициализирует цветовые пары для отображения фигур.
 *
 * Настраивает базовую палитру из восьми цветов, где индексы 1–7 используются
 * для заливки фигур, а пара 8 — для фона и рамок.
 */
void initColors() {
  start_color();
  if (can_change_color() && COLORS >= 256) {
    init_color(COLOR_BISSQUE, 900, 800, 750);
    init_color(COLOR_BABY_BLUE, 600, 800, 950);
    init_color(COLOR_PINK, 950, 700, 800);
    init_color(COLOR_MINT, 600, 950, 850);
    init_color(COLOR_LAVENDER, 800, 750, 950);
    init_color(COLOR_PEACH, 950, 800, 700);
    init_color(COLOR_POWDER, 750, 850, 950);
    init_color(COLOR_GREY, 400, 400, 400);
    init_pair(1, COLOR_BABY_BLUE, COLOR_BABY_BLUE);
    init_pair(2, COLOR_PINK, COLOR_PINK);
    init_pair(3, COLOR_MINT, COLOR_MINT);
    init_pair(4, COLOR_LAVENDER, COLOR_LAVENDER);
    init_pair(5, COLOR_PEACH, COLOR_PEACH);
    init_pair(6, COLOR_POWDER, COLOR_POWDER);
    init_pair(7, COLOR_BISSQUE, COLOR_BISSQUE);
    init_pair(8, COLOR_BLACK, COLOR_GREY);
  } else {
    int cmap[8] = {COLOR_BLACK,   COLOR_CYAN,  COLOR_BLUE, COLOR_YELLOW,
                   COLOR_MAGENTA, COLOR_GREEN, COLOR_RED,  COLOR_WHITE};
    for (int i = 1; i <= 7; ++i) {
      init_pair(i, cmap[i], cmap[i]);
    }
    init_pair(8, COLOR_WHITE, COLOR_BLACK);
  }
}

/**
 * @brief Создаёт и настраивает два окна: игровое поле и боковую панель.
 *
 * @param[out] fwin Указатель на переменную, куда сохраняется окно игрового
 * поля.
 * @param[out] swin Указатель на переменную, куда сохраняется окно боковой
 * панели.
 */
void createWindows(WINDOW **fwin, WINDOW **swin) {
  *fwin = newwin(FIELD_HEIGHT + 2, FIELD_WIDTH * 2 + 2, 0, 0);
  wbkgd(*fwin, COLOR_PAIR(8));
  wattron(*fwin, COLOR_WHITE);
  box(*fwin, 0, 0);
  wattroff(*fwin, COLOR_WHITE);
  wrefresh(*fwin);

  *swin = newwin(FIELD_HEIGHT + 2, 20, 0, FIELD_WIDTH * 2 + 3);
  wbkgd(*swin, COLOR_PAIR(8));
  box(*swin, 0, 0);
}

/**
 * @brief Отрисовывает боковую панель со статистикой и следующей фигурой.
 *
 * @param side_win Окно боковой панели.
 * @param info     Текущая информация об игре.
 */
void DrawSideBar(WINDOW *side_win, GameInfo_t info) {
  werase(side_win);
  box(side_win, 0, 0);
  mvwprintw(side_win, 1, 2, "level:      %4d", info.level);
  mvwprintw(side_win, 3, 2, "score:      %4d", info.score);
  mvwprintw(side_win, 4, 2, "high score: %4d", info.high_score);
  mvwprintw(side_win, 6, 2, "next:");
  for (int i = 0; i < FIGURE_SIZE; i++) {
    for (int j = 0; j < FIGURE_SIZE; j++) {
      int v = info.next[i][j];
      if (v > 0) {
        wattron(side_win, COLOR_PAIR(v));
        mvwprintw(side_win, 8 + i, 2 + j * 2, "  ");
        wattroff(side_win, COLOR_PAIR(v));
      }
    }
  }
  wrefresh(side_win);
}

/**
 * @brief Отрисовывает игровое поле с текущим расположением блоков.
 *
 * @param game_win Окно игрового поля.
 * @param info     Текущая информация об игре.
 */
void DrawGameField(WINDOW *game_win, GameInfo_t info) {
  werase(game_win);
  box(game_win, 0, 0);
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      int v = info.field[y][x];
      if (v > 0) {
        wattron(game_win, COLOR_PAIR(v));
        mvwprintw(game_win, y + 1, x * 2 + 1, "  ");
        wattroff(game_win, COLOR_PAIR(v));
      }
    }
  }
  wrefresh(game_win);
}

/**
 * @brief Преобразует код клавиши в действие пользователя.
 *
 * @param userInput Код нажатой клавиши (getch()).
 * @return Соответствующее действие пользователя (UserAction_t).
 */
UserAction_t getButton(int userInput) {
  UserAction_t action;
  if (userInput == KEY_ENTER || userInput == ' ' || userInput == '\n') {
    action = Start;
  } else if (userInput == KEY_LEFT) {
    action = Left;
  } else if (userInput == KEY_RIGHT) {
    action = Right;
  } else if (userInput == KEY_DOWN) {
    action = Down;
  } else if (userInput == 'z' || userInput == 'Z') {
    action = Action;
  } else if (userInput == 'p' || userInput == 'P') {
    action = Pause;
  } else if (userInput == 27) {
    action = Terminate;
  } else {
    action = Up;
  }
  return action;
}

/**
 * @brief Обрабатывает ввод пользователя и флаг завершения игры.
 *
 * Считывает последнюю нажатую клавишу, преобразует её в действие,
 * и при необходимости завершает главный цикл.
 *
 * @param[out] action  Переменная для сохранения распознанного действия.
 * @param[out] running Флаг, указывающий, продолжается ли игра.
 */
void processInput(UserAction_t *action, bool *running) {
  int ch = getch();
  *action = getButton(ch);
  if (*action == Terminate) {
    *running = false;
  }
}

/**
 * @brief Реализует «гравитацию»: по таймеру вызывает падение фигуры.
 *
 * @param[in,out] delay Текущий накопленный задержки.
 * @param[in]     gi    Указатель на информацию об игре (speed).
 */
void applyGravity(int *delay, const GameInfo_t *gi) {
  *delay += 50;
  if (*delay >= gi->speed) {
    userInput(Up, false);
    *delay = 0;
  }
}

/**
 * @brief Обновляет состояние экрана: поле, боковую панель и сообщения
 * паузы/конца игры.
 *
 * @param field_win Окно игрового поля.
 * @param side_win  Окно боковой панели.
 * @param gi        Указатель на текущую информацию об игре (изменяется внутри).
 */
void render(WINDOW *field_win, WINDOW *side_win, GameInfo_t *gi) {
  if (gi->pause != 2) {
    *gi = updateCurrentState();
    DrawSideBar(side_win, *gi);
    DrawGameField(field_win, *gi);
  }
  if (gi->pause == 1) {
    mvwprintw(field_win, FIELD_HEIGHT / 2 + 1, (FIELD_WIDTH * 2 - 5) / 2 + 1,
              "pause");
    wrefresh(field_win);
  } else if (gi->pause == 2) {
    mvwprintw(field_win, FIELD_HEIGHT / 2 + 1, (FIELD_WIDTH * 2 - 5) / 2 + 1,
              "game over");
    wrefresh(field_win);
  }
  napms(50);
}

/**
 * @brief Точка входа в программу. Инициализирует окружение и запускает главный
 * цикл игры.
 */
int main() {
  srand((unsigned)time(NULL));

  initNcurses();
  initColors();

  WINDOW *game_win, *side_win;
  createWindows(&game_win, &side_win);

  userInput(Start, false);
  GameInfo_t gi = updateCurrentState();

  int delay = 0;
  bool running = true;
  UserAction_t action;
  while (running) {
    processInput(&action, &running);
    if (running) {
      if (action != Up) {
        userInput(action, false);
      }
      applyGravity(&delay, &gi);
      render(game_win, side_win, &gi);
    }
  }

  endwin();
  return 0;
}
