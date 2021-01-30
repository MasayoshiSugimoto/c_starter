#include <stdbool.h>

/********************************************************************************
* Player
********************************************************************************/

enum Player {
	PLAYER_NONE,
	PLAYER_1,
	PLAYER_2,
};

/********************************************************************************
* Game
********************************************************************************/

typedef enum Player Board[3][3];

struct Game {
	Board board;
	enum Player turn;
};

struct BoardCell {
	int x;
	int y;
};


void gamePlay(struct Game* game, struct BoardCell* cell);
void boardCellInit(struct BoardCell* cell);
void gameBoardAsString(char* buf, struct Game* game);
struct BoardCell gameGetBoardCell(char c);
bool boardCellValid(struct BoardCell* cell);
bool gameIsEmptyCell(struct Game* game, struct BoardCell* cell);
bool gameIsOver(struct Game* game);
void gameInit(struct Game* game);
