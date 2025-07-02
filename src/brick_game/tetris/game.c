#include "game.h"

#include "backend.h"

/**
 * @brief Обрабатывает ввод пользователя и передаёт его в обработчик игровой
 * логики.
 *
 * @param action Тип действия пользователя (движение, вращение, пауза и т.д.).
 * @param hold   Флаг удержания кнопки (в текущей реализации не используется).
 */
void userInput(UserAction_t action, bool hold) {
  (void)hold;
  userInputHandler(action);
}

/**
 * @brief Возвращает актуальную информацию об игровом состоянии.
 *
 * Эта функция запрашивает контекст игры и возвращает структуру GameInfo_t,
 * содержащую текущий счёт, уровень, скорость, состояние паузы, поле и прочие
 * параметры.
 *
 * @return Текущая информация об игре (GameInfo_t).
 */
GameInfo_t updateCurrentState() {
  const GameContext_t *gc = getContext();
  return gc->info;
}