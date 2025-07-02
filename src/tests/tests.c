#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../brick_game/tetris/backend.h"
#include "../brick_game/tetris/game.h"

START_TEST(test_getContext_singleton) {
  GameContext_t *a = getContext();
  GameContext_t *b = getContext();
  ck_assert_ptr_eq(a, b);
  gameOver(getContext());
}
END_TEST

START_TEST(test_nextCurrentInit_copies_next_and_color) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIGURE_SIZE; ++i)
    for (int j = 0; j < FIGURE_SIZE; ++j)
      gc->info.next[i][j] = (i == 2 && j == 1) ? 5 : 0;

  nextCurrentInit(gc);
  ck_assert_int_eq(gc->current.shape[2][1], 5);
  ck_assert_int_eq(gc->current.color, 5);

  gameOver(gc);
}
END_TEST

START_TEST(test_clearLines_increment_score_and_compact) {
  GameContext_t *gc = getContext();
  for (int j = 0; j < FIELD_WIDTH; ++j) gc->info.field[FIELD_HEIGHT - 1][j] = 1;
  gc->info.score = 0;

  clearLines(gc);
  ck_assert_int_eq(gc->info.score, 100);
  for (int j = 0; j < FIELD_WIDTH; ++j)
    ck_assert_int_eq(gc->info.field[0][j], 0);

  gameOver(gc);
  remove("record.txt");
}
END_TEST

START_TEST(test_gameOver_writes_record_and_sets_pause) {
  remove("record.txt");
  GameContext_t *gc = getContext();
  gc->info.score = 555;

  gameOver(gc);

  FILE *f = fopen("record.txt", "r");
  ck_assert_ptr_nonnull(f);
  int val = -1;
  fscanf(f, "%d", &val);
  fclose(f);
  ck_assert_int_eq(val, 555);
  remove("record.txt");
}
END_TEST

START_TEST(test_checkCollision_empty_board_in_bounds) {
  GameContext_t *gc = getContext();

  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][0] = 1;
  gc->current.x = 3;
  gc->current.y = 5;

  for (int i = 0; i < FIELD_HEIGHT; ++i)
    for (int j = 0; j < FIELD_WIDTH; ++j) gc->info.field[i][j] = 0;
  ck_assert_int_eq(checkCollision(gc), 1);
  gameOver(getContext());
}
END_TEST

START_TEST(test_checkCollision_out_of_bounds_right) {
  GameContext_t *gc = getContext();
  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][3] = 1;
  gc->current.x = FIELD_WIDTH - 2;
  gc->current.y = 0;
  ck_assert_int_eq(checkCollision(gc), 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_canFall_true_and_false) {
  GameContext_t *gc = getContext();
  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][0] = 1;
  gc->current.x = 0;
  gc->current.y = 0;

  for (int i = 0; i < FIELD_HEIGHT; ++i)
    for (int j = 0; j < FIELD_WIDTH; ++j) gc->info.field[i][j] = 0;
  ck_assert(canFall(gc));

  gc->info.field[1][0] = 1;
  ck_assert(!canFall(gc));
  gameOver(getContext());
}
END_TEST

START_TEST(test_rotateTetromino_non_square) {
  GameContext_t *gc = getContext();

  int L[4][4] = {{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}};
  memcpy(gc->current.shape, L, sizeof(L));
  rotateTetromino(gc);

  ck_assert_int_eq(gc->current.shape[0][1], 1);
  ck_assert_int_eq(gc->current.shape[0][2], 1);
  ck_assert_int_eq(gc->current.shape[0][3], 1);
  ck_assert_int_eq(gc->current.shape[1][1], 1);
  gameOver(getContext());
}
END_TEST

START_TEST(test_draw_and_clear_figure) {
  GameContext_t *gc = getContext();

  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][0] = gc->current.shape[0][1] = gc->current.shape[1][0] =
      gc->current.shape[1][1] = 1;
  gc->current.color = 7;
  gc->current.x = 2;
  gc->current.y = 3;

  for (int i = 0; i < FIELD_HEIGHT; ++i)
    for (int j = 0; j < FIELD_WIDTH; ++j) gc->info.field[i][j] = 0;
  drawFigure(gc);

  ck_assert_int_eq(gc->info.field[3][2], 7);
  ck_assert_int_eq(gc->info.field[3][3], 7);
  ck_assert_int_eq(gc->info.field[4][2], 7);
  ck_assert_int_eq(gc->info.field[4][3], 7);
  clearFigure(gc);
  ck_assert_int_eq(gc->info.field[3][2], 0);
  ck_assert_int_eq(gc->info.field[4][3], 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_trySpawnFigure_collision_prevents_draw) {
  GameContext_t *gc = getContext();

  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][0] = 1;
  gc->current.color = 4;
  gc->current.x = 0;
  gc->current.y = 0;

  gc->info.field[0][0] = 9;

  ck_assert_int_eq(trySpawnFigure(gc), 0);

  ck_assert_int_eq(gc->info.field[0][0], 9);
  gameOver(getContext());
}
END_TEST

START_TEST(test_autoMoveDown_changes_state_on_collision) {
  GameContext_t *gc = getContext();
  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][0] = 1;
  gc->current.color = 3;
  gc->current.x = 0;
  gc->current.y = FIELD_HEIGHT - 2;

  gc->info.field[FIELD_HEIGHT - 1][0] = 5;
  gc->state = STATE_FALLING;
  autoMoveDown(gc);

  ck_assert_int_eq(gc->current.y, FIELD_HEIGHT - 2);
  ck_assert_int_eq(gc->state, STATE_CLEARING);
  gameOver(getContext());
}
END_TEST

START_TEST(test_autoMoveDown_moves_down_if_free) {
  GameContext_t *gc = getContext();
  memset(gc->current.shape, 0, sizeof(gc->current.shape));
  gc->current.shape[0][0] = 1;
  gc->current.color = 2;
  gc->current.x = 1;
  gc->current.y = 1;
  for (int i = 0; i < FIELD_HEIGHT; i++)
    for (int j = 0; j < FIELD_WIDTH; j++) gc->info.field[i][j] = 0;
  gc->state = STATE_FALLING;
  autoMoveDown(gc);
  ck_assert_int_eq(gc->current.y, 2);
  ck_assert_int_eq(gc->state, STATE_FALLING);
  ck_assert_int_eq(gc->info.field[2][1], 2);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_left_moves_left) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][0] = 1;
  gc->current.x = 5;
  gc->current.y = 5;
  gc->state = STATE_FALLING;
  fallingHandler(gc, Left);
  ck_assert_int_eq(gc->current.x, 4);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_left_blocked_at_zero) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][0] = 1;
  gc->current.x = 0;
  gc->current.y = 5;
  gc->state = STATE_FALLING;
  fallingHandler(gc, Left);
  ck_assert_int_eq(gc->current.x, 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_right_moves_right) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][0] = 1;
  gc->current.x = 3;
  gc->current.y = 4;
  gc->state = STATE_FALLING;
  fallingHandler(gc, Right);
  ck_assert_int_eq(gc->current.x, 4);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_right_blocked_at_edge) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][FIGURE_SIZE - 1] = 1;
  gc->current.x = FIELD_WIDTH - FIGURE_SIZE;
  gc->current.y = 0;
  gc->state = STATE_FALLING;
  fallingHandler(gc, Right);
  ck_assert_int_eq(gc->current.x, FIELD_WIDTH - FIGURE_SIZE);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_down_locks_and_clearing) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][0] = 1;
  gc->current.x = 0;
  gc->current.y = FIELD_HEIGHT - 2;
  gc->info.field[FIELD_HEIGHT - 1][0] = 1;
  gc->state = STATE_FALLING;
  fallingHandler(gc, Down);
  ck_assert_int_eq(gc->state, STATE_CLEARING);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_action_rotates_when_no_collision) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][1] = 1;
  gc->current.x = 0;
  gc->current.y = 0;
  gc->state = STATE_FALLING;
  int original[FIGURE_SIZE][FIGURE_SIZE];
  memcpy(original, gc->current.shape, sizeof original);
  fallingHandler(gc, Action);
  ck_assert(memcmp(gc->current.shape, original, sizeof original) != 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_pause_sets_pause) {
  GameContext_t *gc = getContext();
  gc->state = STATE_FALLING;
  gc->info.pause = 0;
  fallingHandler(gc, Pause);
  ck_assert_int_eq(gc->state, STATE_PAUSED);
  ck_assert_int_eq(gc->info.pause, 1);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_terminate_sets_game_over) {
  GameContext_t *gc = getContext();
  gc->state = STATE_FALLING;
  gc->info.pause = 0;
  fallingHandler(gc, Terminate);
  ck_assert_int_eq(gc->state, STATE_GAME_OVER);
  ck_assert_int_eq(gc->info.pause, 2);
  gameOver(getContext());
}
END_TEST

START_TEST(test_fallingHandler_up_calls_autoMoveDown) {
  GameContext_t *gc = getContext();
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  memset(gc->current.shape, 0, sizeof gc->current.shape);
  gc->current.shape[0][0] = 1;
  gc->current.x = 2;
  gc->current.y = 2;
  gc->state = STATE_FALLING;
  fallingHandler(gc, Up);
  ck_assert_int_eq(gc->current.y, 3);
  gameOver(getContext());
}
END_TEST

START_TEST(test_userInputHandler_start_to_spawn) {
  GameContext_t *gc = getContext();
  gc->state = STATE_START;
  gc->info.pause = 1;
  userInputHandler(Start);
  ck_assert_int_eq(gc->state, STATE_SPAWN);
  ck_assert_int_eq(gc->info.pause, 1);
  gameOver(getContext());
}
END_TEST

START_TEST(test_userInputHandler_spawn_success) {
  GameContext_t *gc = getContext();
  gc->state = STATE_SPAWN;
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    memset(gc->info.field[i], 0, FIELD_WIDTH * sizeof(int));
  }
  userInputHandler(Left);
  ck_assert_int_eq(gc->state, STATE_FALLING);
  gameOver(getContext());
}
END_TEST

START_TEST(test_userInputHandler_pause_unpauses) {
  GameContext_t *gc = getContext();
  gc->state = STATE_PAUSED;
  gc->info.pause = 1;
  userInputHandler(Pause);
  ck_assert_int_eq(gc->state, STATE_FALLING);
  ck_assert_int_eq(gc->info.pause, 0);
  gameOver(getContext());
}
END_TEST
START_TEST(test_updateCurrentState_defaults) {
  GameContext_t *gc = getContext();
  GameInfo_t info = updateCurrentState();
  ck_assert_int_eq(info.score, gc->info.score);
  ck_assert_int_eq(info.high_score, 555);
  ck_assert_int_eq(info.level, 1);
  ck_assert_int_eq(info.speed, 1000);
  ck_assert_int_eq(info.pause, 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_updateCurrentState_snapshot) {
  GameContext_t *gc = getContext();
  gc->info.score = 123;
  GameInfo_t b = updateCurrentState();
  ck_assert_int_eq(b.score, 123);
  gameOver(getContext());
  remove("record.txt");
}
END_TEST

START_TEST(test_userInput_start_transition) {
  GameContext_t *gc = getContext();
  gc->state = STATE_START;
  gc->info.pause = 0;
  userInput(Start, false);
  ck_assert_int_eq(gc->state, STATE_SPAWN);
  ck_assert_int_eq(gc->info.pause, 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_userInput_hold_ignored) {
  GameContext_t *gc = getContext();
  gc->state = STATE_START;
  gc->info.pause = 0;
  userInput(Start, true);
  ck_assert_int_eq(gc->state, STATE_SPAWN);
  ck_assert_int_eq(gc->info.pause, 0);
  gameOver(getContext());
}
END_TEST

START_TEST(test_userInputHandler_clearing_to_spawn) {
  GameContext_t *gc = getContext();
  gc->state = STATE_CLEARING;
  for (int j = 0; j < FIELD_WIDTH; ++j) gc->info.field[FIELD_HEIGHT - 1][j] = 1;
  gc->info.score = 0;
  userInputHandler(Left);
  ck_assert_int_eq(gc->state, STATE_SPAWN);
  ck_assert_int_eq(gc->info.score, 100);
  gameOver(getContext());
  remove("record.txt");
}
END_TEST

Suite *backend_suite() {
  Suite *s = suite_create("TetrisBackend");
  TCase *tc = tcase_create("Core");
  tcase_add_test(tc, test_getContext_singleton);
  tcase_add_test(tc, test_nextCurrentInit_copies_next_and_color);
  tcase_add_test(tc, test_clearLines_increment_score_and_compact);
  tcase_add_test(tc, test_gameOver_writes_record_and_sets_pause);
  tcase_add_test(tc, test_checkCollision_empty_board_in_bounds);
  tcase_add_test(tc, test_checkCollision_out_of_bounds_right);
  tcase_add_test(tc, test_canFall_true_and_false);
  tcase_add_test(tc, test_rotateTetromino_non_square);
  tcase_add_test(tc, test_draw_and_clear_figure);
  tcase_add_test(tc, test_trySpawnFigure_collision_prevents_draw);
  tcase_add_test(tc, test_autoMoveDown_changes_state_on_collision);
  tcase_add_test(tc, test_autoMoveDown_moves_down_if_free);
  tcase_add_test(tc, test_fallingHandler_left_moves_left);
  tcase_add_test(tc, test_fallingHandler_left_blocked_at_zero);
  tcase_add_test(tc, test_fallingHandler_right_moves_right);
  tcase_add_test(tc, test_fallingHandler_right_blocked_at_edge);
  tcase_add_test(tc, test_fallingHandler_down_locks_and_clearing);
  tcase_add_test(tc, test_fallingHandler_action_rotates_when_no_collision);
  tcase_add_test(tc, test_fallingHandler_pause_sets_pause);
  tcase_add_test(tc, test_userInputHandler_start_to_spawn);
  tcase_add_test(tc, test_fallingHandler_terminate_sets_game_over);
  tcase_add_test(tc, test_fallingHandler_up_calls_autoMoveDown);
  tcase_add_test(tc, test_userInputHandler_spawn_success);
  tcase_add_test(tc, test_userInputHandler_pause_unpauses);
  tcase_add_test(tc, test_userInputHandler_clearing_to_spawn);
  tcase_add_test(tc, test_updateCurrentState_defaults);
  tcase_add_test(tc, test_updateCurrentState_snapshot);
  tcase_add_test(tc, test_userInput_start_transition);
  tcase_add_test(tc, test_userInput_hold_ignored);
  suite_add_tcase(s, tc);
  return s;
}

int main() {
  Suite *s = backend_suite();
  SRunner *sr = srunner_create(s);
  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_VERBOSE);
  int nfail = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (nfail == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}