#include <stdint.h>
#include <stdlib.h>
#include "gba.h"
#include "font.h"
#include "class.h"
#include "functions.h"
#include "tiles.h"
#include <time.h>

// g:\RunMake "$(CURRENT_DIRECTORY)" run

uint8_t cursor[64][64];
int cursoractual[2] = {110,68}; // Actual location of cursor including wobble
int BGPos[2] = {128,128}; // Location of screen on BG layer
int cursorpos[2] = {116,76}; // General location of the cursor
Match match;

Player players[2];

int main()
{
	SetAffine(0,30,1,1);
	SetAffine(1,50,2,2);
	LoadTilesAndSetDCNT();
	REG_BG3HOFS = BGPos[0];
	REG_BG3VOFS = BGPos[1];		

	ClearObjects();
	
	ClearScreenBlocks(25,31);
	StartScreen();
	ClearScreenBlocks(25,31);

	
	if (!ColinModeMenu(cursoractual, cursorpos, players)) {
		ClearScreenBlocks(25,31);
		SetupMenu(players);	
	}
	ClearScreenBlocks(25,31);
	
	match.Play(players, BGPos, cursorpos, cursoractual);
	
	
	return 0;
}
