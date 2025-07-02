CC := gcc
FLAGS = -Wall -Werror -Wextra -std=c11 
back = brick_game/tetris/backend.c
game = brick_game/tetris/game.c
front = gui/cli/frontend.c
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)

	PREFIX = $(HOME)/tetris
    OPEN_CMD = open
	CHECK_INCLUDE_PATH = $(shell brew --prefix check 2>/dev/null)/include
 	CHECK_LIB_PATH = $(shell brew --prefix check 2>/dev/null)/lib
	CHECK_LIB = -lcheck
else

	PREFIX = /usr/local/tetris
    OPEN_CMD = xdg-open
	CHECK_INCLUDE_PATH = /usr/include
 	CHECK_LIB_PATH = /usr/lib
	CHECK_LIB = -lcheck -pthread -lrt -lm -lsubunit
endif


INSTALLBINDIR = $(PREFIX)/game
INSTALLLIBDIR = $(PREFIX)/lib

TEST_SRC = tests/tests.c
TEST_EXE = test_tetris

DOC_DIR = docs
REPORT_DIR = report
COVDIR = cov_info



MAIN_FLAGS = $(FLAGS)
TEST_FLAGS = $(FLAGS) -fprofile-arcs -ftest-coverage


all: tetris

tetris.a: backend.o game.o
	ar rcs tetris.a backend.o game.o

tetris: tetris.a frontend.o 
	$(CC) $(MAIN_FLAGS) -lncurses -o tetris frontend.o tetris.a

install: all 
	mkdir -p "$(INSTALLBINDIR)"
	install -m 755 tetris "$(INSTALLBINDIR)/tetris"
	mkdir -p "$(INSTALLLIBDIR)"
	install -m 644 tetris.a "$(INSTALLLIBDIR)/tetris.a"

uninstall:
	rm -rf "$(PREFIX)"

clean:
	rm -rf *.o tetris $(TEST_EXE) $(DOC_DIR) $(REPORT_DIR) $(COVDIR) $(PREFIX) *.gcda *.gcno *.info *.a tetris.tar.gz *.txt

backend.o: $(back)
	$(CC) $(MAIN_FLAGS) -c $(back) -o $@

frontend.o: $(front)
	$(CC) $(MAIN_FLAGS) -c $(front) -o $@

game.o: $(game)
	$(CC) $(MAIN_FLAGS) -c $(game) -o $@

test: 
	$(CC) $(TEST_FLAGS) -I$(CHECK_INCLUDE_PATH) $(CHECK_CFLAGS) -c $(back) -o backend_test.o
	$(CC) $(TEST_FLAGS) -I$(CHECK_INCLUDE_PATH) -c $(game) -o game_test.o
	$(CC) $(TEST_FLAGS) -I$(CHECK_INCLUDE_PATH) -o $(TEST_EXE) $(TEST_SRC) backend_test.o game_test.o -L$(CHECK_LIB_PATH) $(CHECK_LIB)
	./$(TEST_EXE)


gcov_report:
	@$(MAKE) clean
	@$(MAKE) test
	@mkdir -p $(COVDIR)
	@lcov --directory . --capture --output-file $(COVDIR)/coverage.info
	@genhtml --output-directory $(REPORT_DIR) --legend $(COVDIR)/coverage.info
	-@$(OPEN_CMD) $(REPORT_DIR)/index.html || true

dvi:
	doxygen Doxyfile
ifeq ($(UNAME_S),Darwin)
	@$(OPEN_CMD) $(DOC_DIR)/html/index.html
else
	@echo "HTML-документация доступна в $(DOC_DIR)/html/index.html"
endif

dist: clean
	mkdir -p tetris_dist/src
	cp -r brick_game gui Makefile tetris_dist/src
	tar -czvf tetris.tar.gz tetris_dist
	rm -rf tetris_dist