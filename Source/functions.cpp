#include "functions.h"
#include <stdlib.h>
#include "gba.h"
#include "class.h"
#include "font.h"
#include "tiles.h"
#include <math.h>
#include <stdio.h>

////////////////////GENERAL FUNCTIONS/////////////////////////////////////


//  Handles initial setup of palettes and charblocks and DCNT
void LoadTilesAndSetDCNT() {

	//  Set the palettes
	for (int i = 0; i < 256; i++) {	
		SetPaletteBG(i, BGPalette[i]);
	}
	for (int i = 0; i < 3; i++) {
		SetPaletteObj(i, CursorPal[i]);
	}
	//  append dart palette to object palette
	for (int i = 0; i < 3; i++) {
		SetPaletteObj(i+3, DartPal[i]);
	}

	//  Load the Dartboard tiles into CharBlock 0	
	for (int i = 0; i < 214; i++) {
		LoadTile8(0, i+1, &BGTiles[i*64]);
	}
	
	//  Load UI Border Tiles into CharBlock 1
	LoadTile8(1, 1, horiz_border_tile);
	LoadTile8(1, 2, vertical_border_tile);
	LoadTile8(1, 3, corner_border_tile);
	LoadTile8(1, 4, window_background_tile);
	LoadTile8(1, 5, minidart_tile);
	
	//  Load Text Tiles into Charblock 2
	for (int i = 0; i < 128; i++)
	{
		LoadTile8(2, i, font_bold[i]);
	}
	
	//  Load number-text tiles into charblock 0 for use in dartboard background (BG3)
	for (int i = 0; i < 10; i++) {
		LoadTile8(0, i + 215, font_medium[i+48]);
	}
	
	// Load the tiles for the objects into charblock 4.
	// (Charblocks 4 and 5 are for object tiles;
	// 8bpp tiles 0-255 are in CB 4, tiles 256-511 in CB 5.)
	for (int i = 0; i < 64; i++) {
		LoadTile8(4, i, &CursorTiles[i*64]);
	}
	
	for (int i = 0; i < 16; i++) {
		LoadTile8(4, i+64, &DartTiles[i*64]);
	}
	
	// Set display options.
	// DCNT_MODE0 sets mode 0, which provides four tiled backgrounds.
	// DCNT_OBJ enables objects.
	// DCNT_OBJ_1D make object tiles mapped in 1D (which makes life easier).
	REG_DISPCNT = DCNT_MODE0 | DCNT_OBJ | DCNT_OBJ_1D | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3;
	REG_BG0CNT = BG_CBB(2) | BG_SBB(25) | BG_8BPP | BG_REG_32x32 | BG_PRIO(0);  //  UI Text
	REG_BG1CNT = BG_CBB(2) | BG_SBB(26) | BG_8BPP | BG_REG_32x32 | BG_PRIO(0);  //  User Generated Text and tile-based cursors
	REG_BG2CNT = BG_CBB(1) | BG_SBB(27) | BG_8BPP | BG_REG_32x32 | BG_PRIO(1);  //  UI Windows
	REG_BG3CNT = BG_CBB(0) | BG_SBB(28) | BG_8BPP | BG_REG_64x64 | BG_PRIO(3);  //  DartBoard	
	
	//  Set Sound Registers
/*	
	REG_SGCNT1 = SSTAT_ENABLE;
	REG_SGCNT0 = SDMG_BUILD_LR(SDMG_SQR1, 7) | (1<<1);
	REG_SG21 = (1<<16) | SND_RATE(NOTE_A, 0);
*/		


}

//  Draw a window starting at (x,y) with width and height
void DrawWindow(int xpos, int ypos, int width, int height) {
	width--;
	height--;
	//  Place Corners
	SetTile(27, xpos, ypos, 3 | SE_VFLIP);
	SetTile(27, xpos+width, ypos, 3 | SE_HFLIP | SE_VFLIP);
	SetTile(27, xpos, ypos+height, 3);
	SetTile(27, xpos+width, ypos+height, 3 | SE_HFLIP);
	
	//  Draw Edges	
	for (int x = 1; x < width; x++) {
		SetTile(27, x+xpos, ypos, 1 | SE_VFLIP);
		SetTile(27, x+xpos, ypos + height, 1);
	}	
	for (int y = 1; y < height; y++) {
		SetTile(27, xpos, y+ypos, 2);
		SetTile(27, xpos+width, y+ypos, 2 | SE_HFLIP);
	}
	
	//  Fill in
	
	for (int y = 1; y < height; y++) {
		for (int x = 1; x < width; x++) {
			SetTile(27, x+xpos, y+ypos, 4);
		}
	}
}

//  Draw string to screenblock at position (x,y)
void DrawText(int x, int y, const char string[], int screenblock) {

	int i = 0;
	
	while (string[i] != '\0') {	
		SetTile(screenblock, x, y, int(string[i]));
		x++;
		i++;
	}
}

//  Draw and integer to screenblock at position (x,y)
void DrawNumber(int x, int y, int number, int screenblock){
	char buf[20];
	snprintf(buf, sizeof buf, "%d", number);
	DrawText(x, y, buf, screenblock);
}

//  Draw any floating point number to screenblock at positon (x,y)
void DrawFloat(int x, int y, float number, int screenblock){
	char buf[20];
	snprintf(buf, sizeof buf, "%f", number);
	DrawText(x, y, buf, screenblock);
}

//  Clear a group of screenblocks, clears all blocks low-high inclusive
void ClearScreenBlocks(int low, int high) {
	for (int base = low; base <= high; base++){
		for (int y = 0; y < 32; ++y)
		{
			for (int x = 0; x < 32; ++x)
			{
				SetTile(base, x, y, 0);
			}
		}
	}

}

//  Reads off the screen to provide the the tile entry address of the tile found at (x,y) in screenblock
//  Result is relative to the charblock base used by the background reading from screenblock
int GetTile(int screenblock, int x, int y) {
	return REG_VIDEO_BASE[(screenblock * 1024) + (y * 32) + x];
}


///////////////////////GAME FUNCTIONS////////////////////////////////////

//  Move cursor around screen, add some wobble and pan around
//  background if cursor moves near edges
void HandleCursorMovement(int cursoractual[], int cursorpos[], int BGPos[]) {


	static int force[2] = {0,0}; // Displacement force for wobble
	static int counter = 0; // Slow down wobble speed


	// Detect user input and limit cursor movement
	if ((REG_P1 & KEY_UP) == 0) {
		cursorpos[1]-=2;
		cursoractual[1]-=2;
	}
	if ((REG_P1 & KEY_DOWN) == 0) {
		cursorpos[1]+=2;
		cursoractual[1]+=2;
	}
	if ((REG_P1 & KEY_LEFT) == 0) {
		cursorpos[0]-=2;
		cursoractual[0]-=2;
	}
	if ((REG_P1 & KEY_RIGHT) == 0) {
		cursorpos[0]+=2;
		cursoractual[0]+=2;
	}

	//  Restrain movement to dartboard area and make Background pan
	if (cursorpos[0] > SCREEN_WIDTH-130) {
		cursoractual[0] += (SCREEN_WIDTH-130) - cursorpos[0];
		cursorpos[0] = SCREEN_WIDTH-130;
		BGPos[0]++;
	} else if (cursorpos[0] < 10) {
		cursoractual[0] += 10 - cursorpos[0];
		cursorpos[0] = 10;
		BGPos[0]--;
	}
	if (cursorpos[1] > SCREEN_HEIGHT-40) {
		cursoractual[1] += (SCREEN_HEIGHT-40) - cursorpos[1];
		cursorpos[1] = SCREEN_HEIGHT-40;
		BGPos[1]++;
	} else if (cursorpos[1] < 10) {
		cursoractual[1] += 10-cursorpos[1];
		cursorpos[1] = 10;
		BGPos[1]--;
	}


	// Do cursor wobble
	int x = (cursorpos[0] - cursoractual[0]);
	int y = (cursorpos[1] - cursoractual[1]);

	counter++;
	if (counter == 5) {
	force[0] += ((x*x-2) * ((x > 0) - (x < 0)));
	force[1] += ((y*y-2) * ((y > 0) - (y < 0)));

	cursoractual[0] = cursoractual[0] + force[0]/50;
	cursoractual[1] = cursoractual[1] + force[1]/50;
	counter = 0;
	}
	

	//  Limit background layer panning
	if (BGPos[0] < 80) {
		BGPos[0] = 80;
	} else if (BGPos[0] > (270)) {
		BGPos[0] = (270);
	}
	
	if (BGPos[1] < 80) {
		BGPos[1] = 80;
	} else if (BGPos[1] > (270)) {
		BGPos[1] = (270);
	}
	
	//  Update background offset for new pan values
	REG_BG3HOFS = BGPos[0];
	REG_BG3VOFS = BGPos[1];			
	
	//  Draw the cursor
	AnimateCursor(cursoractual);
}

//  Draws 4 quarter dartboards in screenblocks 18-31 inclusive, flipping as appropriate
//  Then inserts numbers around dartboard circumference
void DrawDartboard(){
	int y = 0;
	int x = 0;
	int lim = 15;
	int count = 0;
	
	int offset = 16;
	
	for (y = 0, count = 1; y <16; y++) {
		if (y == 0) {
			x = 10;	
		} else if (y == 1) {
			x = 8;
		} else if (y == 2) {
			x = 6;
		} else if (y == 3) {
			x = 5;
		} else if (y == 4) {
			x = 4;
		} else if (y == 5) {
			x = 3;
		} else if (y == 6 || y == 7){
			x = 2;
		} else if (y == 8 || y == 9) {
			x = 1;
		} else {
			x = 0;
		}
		
		for (x; x <= lim; x++) {
			SetTile(28, x+offset, y+offset, count);
			SetTile(29, 16-(x+1), y+offset, count | SE_HFLIP);			
			SetTile(30, x+offset, 16-(y+1), count | SE_VFLIP);
			SetTile(31, 16-(x+1), 16-(y+1), count | SE_VFLIP | SE_HFLIP);
			count++;
		}
	}
	const int base = 215;
	
	//  Draw outer numbers by hand for optimum appearance	
	SetTile(28, 31, 14, base+2);
	SetTile(29, 0, 14, base+0);
	SetTile(29, 5, 15, base+1);
	SetTile(29, 9, 17, base+1);
	SetTile(29, 10, 17, base+8);
	SetTile(29, 13, 21, base+4);
	SetTile(29, 16, 26, base+1);
	SetTile(29, 17, 26, base+3);
	SetTile(31, 17, 0, base+6);
	SetTile(31, 16, 5, base+1);
	SetTile(31, 17, 5, base+0);
	SetTile(31, 13, 10, base+1);
	SetTile(31, 14, 10, base+5);
	SetTile(31, 9, 14, base+2);
	SetTile(31, 4, 16, base+1);
	SetTile(31, 5, 16, base+7);
	SetTile(30, 31, 17, base+3);
	SetTile(30, 26, 16, base+1);
	SetTile(30, 27, 16, base+9);
	SetTile(30, 21, 14, base+7);
	SetTile(30, 16, 10, base+1);
	SetTile(30, 17, 10, base+6);
	SetTile(30, 15, 5, base+8);
	SetTile(30, 13, 0, base+1);
	SetTile(30, 14, 0, base+1);
	SetTile(28, 14, 26, base+1);
	SetTile(28, 15, 26, base+4);
	SetTile(28, 17, 21, base+9);
	SetTile(28, 21, 17, base+1);
	SetTile(28, 22, 17, base+2);
	SetTile(28, 26, 15, base+5);
}

//  Display starting slash screen and wait for user to hit start, seed the RNG with the number of frames passed
void StartScreen() {
	bool pause = true;
	int seed = 0;

	DrawText(4, 10, "Press Start to Begin!", 25);
	
	while (pause) {
		WaitVSync();
		seed++;
		if ((REG_P1 & KEY_START) == 0) {
			pause = false;
		}
	}
	srand(seed);
}

//  Draws the keyboard on screen (qwerty) and move cursor on keyboard according to user input
void DisplayKeyboard(const char title[], char output[]) {
	bool caps = false;
	bool running = true;
	int cursorloc[2] = {10,14};
	DrawWindow(0, 0, 30, 20);
	
	int characternumber = 0;
	
	DrawText(2, 4, title, 25);
	DrawText(5, 7, "Start to Confirm", 25);
	DrawText(12, 9, "____", 25);
	
	//  Sanitize output char array
	for (int i = 0; i < 4; i++) {
		output[i] = ' ';
	}
	output[4] = '\0';

	
	while(running){
		if (caps == false) {
			DrawText(10, 14, "qwertyuiop", 25);
			DrawText(10, 15, "asdfghjkl", 25);
			DrawText(11, 16, "zxcvbnm", 25);
			SetTile(25, 10, 16, 1);  		//capslock button
		} else {
			DrawText(10, 14, "QWERTYUIOP", 25);
			DrawText(10, 15, "ASDFGHJKL", 25);
			DrawText(11, 16, "ZXCVBNM", 25);
			SetTile(25, 10, 16, 2);			//capslock button
		}
		SetTile(25, 19, 16, 4);				//backspace button
		
		DrawText(cursorloc[0], cursorloc[1], " ", 26);  //delete old cursor location
		
		
		//  Get user input for cursor movement, limit to keyboard boundry
		if ((REG_P1 & KEY_UP) == 0 && oldkey[UP] == false) {
			if (cursorloc[1] > 14) {
				cursorloc[1]--;
			}
			oldkey[UP] = true;
		}
		if ((REG_P1 & KEY_UP) != 0) {
			oldkey[UP] = false;
		}
		
		if ((REG_P1 & KEY_DOWN) == 0 && oldkey[DOWN] == false) {
			if (cursorloc[1] < 16) {
				cursorloc[1]++;
			}
			oldkey[DOWN] = true;
		}
		if ((REG_P1 & KEY_DOWN) != 0) {
			oldkey[DOWN] = false;
		}
		
		if ((REG_P1 & KEY_RIGHT) == 0 && cursorloc[0] < 19 && oldkey[RIGHT] == false) {
			cursorloc[0]++;
			oldkey[RIGHT] = true;
		}	
		if ((REG_P1 & KEY_RIGHT) != 0) {
			oldkey[RIGHT] = false;
		}
		if ((REG_P1 & KEY_LEFT) == 0 && cursorloc[0] > 10 && oldkey[LEFT] == false) {
			cursorloc[0]--;
			oldkey[LEFT] = true;
		}
		if ((REG_P1 & KEY_LEFT) != 0) {
			oldkey[LEFT] = false;
		}
		
		if (cursorloc[1] < 14) {
			cursorloc[1] = 14;
		}
		if (cursorloc[1] > 18) {
			cursorloc[1] = 18;
		}
		
		//  Move cursor to new location
		SetTile(26, cursorloc[0], cursorloc[1],5);
		
		//  When player selects a character, read off screen to find which character
		//  Then put that character into the output array and draw it in the entry bar
		//  if the capslock is hit then display caps keyboard
		//  if the backspace is hit then delete last value in output array
		if ((REG_P1 & KEY_A) == 0 && oldkey[A_KEY] == false) {
			if (GetTile(25, cursorloc[0], cursorloc[1]) > 4 && characternumber < 4) {
				output[characternumber] = (char)GetTile(25, cursorloc[0], cursorloc[1]);			
				characternumber++;				
			} else if (GetTile(25, cursorloc[0], cursorloc[1]) == 1) {
				caps = true;
			} else if (GetTile(25, cursorloc[0], cursorloc[1]) == 2) {
				caps = false;
			} else if (GetTile(25, cursorloc[0], cursorloc[1]) == 4 && characternumber >= 0) {
				if (characternumber != 0) {
					characternumber--;
				}
				output[characternumber] = ' ';
			}	
			oldkey[A_KEY] = true;
			DrawText(12,9,output,26);
		}
		
		if ((REG_P1 & KEY_A) != 0) {
			oldkey[A_KEY] = false;
		}
		if ((REG_P1 & KEY_START) == 0 && oldkey[START] == false) {
			running = false;
			oldkey[START] = true;
		} 
		if ((REG_P1 & KEY_START) != 0) {
			oldkey[START] = false;
		}
		WaitVSync();	
	}
}

//  draws cursor on screen, displays a 4 frame animation.
//  animation reverses direction at ends
void AnimateCursor(int cursoractual[]) {
	static int framecount = 0;
	static int frame = 0;
	static bool reverse = false;
	
	SetObject(0,
		  ATTR0_SHAPE(0) | ATTR0_8BPP | ATTR0_REG | ATTR0_Y(cursoractual[1]),
		  ATTR1_SIZE(2) | ATTR1_X(cursoractual[0]),
		  ATTR2_ID8(frame*16));
	 
	if (framecount == 4) {
		if (reverse) {
			frame--;
		} else {
			frame++;
		}
		framecount = 0;
		if (frame == 3 || frame == 0) {
			reverse = !reverse;
		}
	}
	framecount++;
}

//  Display all the set up menus for start-up
void SetupMenu(Player players[]) {
	int gametype = 0;
	bool humanfirst = true;
	char nameholder[5];
	int difficulty = 0;
	
	DrawWindow(0,0,30,20);
	DrawText(6,5,"Select Game Type:", 25);
	DrawText(6,7,"Human vs Computer", 25);
	DrawText(6,8,"Human vs Human", 25);
	DrawText(6,9,"Computer vs Computer", 25);

	gametype = MenuSelect(7,9);
	
	ClearScreenBlocks(25,26);
	
	if (gametype == 0){
		DrawText(6,5,"Who Goes First?", 25);
		DrawText(6,7,"Human", 25);
		DrawText(6,8,"Computer", 25);
		
		if (MenuSelect(7,8) == 1) {
			humanfirst = false;
		}
	}	
	ClearScreenBlocks(25,26);
	
	if (humanfirst == true && gametype == 0) {
		DisplayKeyboard("Enter Human Player Name:", nameholder);
		ClearScreenBlocks(25,31);
		players[0] = Player(501, nameholder, true, 0);
		for (int i = 0; i < 4; i++) {
			nameholder[i] = ' ';
		}
		
		DisplayKeyboard("Enter Computer Player Name:", nameholder);
		
		ClearScreenBlocks(25,26);	
		
		difficulty = DifficultyMenu();
			
		players[1] = Player(501, nameholder, false, difficulty);
		
	} else if (gametype == 0) {
		DisplayKeyboard("Enter Computer Player Name:", nameholder);
		ClearScreenBlocks(25,26);		
		difficulty = DifficultyMenu();	
		players[0] = Player(501, nameholder, false, difficulty);	
		
		for (int i = 0; i < 4; i++) {
			nameholder[i] = ' ';
		}
		
		ClearScreenBlocks(25,26);
		DisplayKeyboard("Enter Human Player Name:", nameholder);
		players[1] = Player(501, nameholder, true, 0);
	} else {
		DisplayKeyboard("Enter Player 1 Name:", nameholder);
		ClearScreenBlocks(25,26);
		if (gametype == 2) {	
			difficulty = DifficultyMenu();
			players[0] = Player(501, nameholder, false, difficulty);
		} else {
			players[0] = Player(501, nameholder, true, 0);
		}
		
		for (int i = 0; i < 4; i++) {
			nameholder[i] = ' ';
		}
		ClearScreenBlocks(25,26);
		DisplayKeyboard("Enter Player 2 Name:", nameholder);
		ClearScreenBlocks(25,26);
		if (gametype == 2) {						
			difficulty = DifficultyMenu();
			players[1] = Player(501, nameholder, false, difficulty);
		} else {
			players[1] = Player(501, nameholder, true, 0);
		}
		
	}
}

//  Set the affine transformation matrix using rotation, x and y scales
void SetAffine(int matrix, int rot, float sx, float sy) {
	int pa;
	int pb;
	int pc;
	int pd;
	
	//  convert initial inputs into entries in affine transformation matrix
	//  Affine transformation matrix saved in memory as an 8.8 fixed point number
	//  (hence the 256 multiple)
	pa = ((256*cos(rot*M_PI/180))/sx);
	pb = ((256*-sin(rot*M_PI/180))/sx);
	pc = ((256*sin(rot*M_PI/180))/sy);
	pd = ((256*cos(rot*M_PI/180))/sy);
	
	AffineTransform(matrix, pa, pb, pc, pd);
}

//  As the difficulty menu can be drawn in many conditions it has been pulled into a function
//  Draws difficulty options and waits for player to select one
int DifficultyMenu() {

	int difficulty = 0;
	DrawText(6,5,"Select Difficulty:", 25);
	DrawText(6,7,"Easy", 25);
	DrawText(6,8,"Medium", 25);
	DrawText(6,9,"Hard", 25);
	DrawText(6,10,"Jolly Difficult", 25);	

	difficulty = MenuSelect(7,10);
	
	return difficulty;
}

//  Prompts player for colin-mode or adam-mode.
//  If colin-mode is selected all other setup is skipped and prescribed values are selected
bool ColinModeMenu(int cursoractual[], int cursorpos[], Player players[]) {
	
	bool colinmode = false;
	
	DrawWindow(0,0,30,20);
	DrawText(6,5,"What Mode to start in?", 25);
	DrawText(6,7,"Colin Mode", 25);
	DrawText(6,8,"Adam Mode", 25);

	colinmode = ((MenuSelect(7,8) == 0)? true : false);
	
	if (colinmode) {
		cursoractual[0] = cursorpos[0];
		cursoractual[1] = cursorpos[1];
		ClearScreenBlocks(25,26);
		

		DrawText(6,5,"Who Goes First?", 25);
		DrawText(6,7,"Human (Joe)", 25);
		DrawText(6,8,"Computer (Sid)", 25);
		
		if (MenuSelect(7,8) == 1) {
			players[0] = Player(501, "Sid", false, 72, true);
			players[1] = Player(501, "Joe", true, 70, true);
		} else {
			players[0] = Player(501, "Joe", true, 70, true);
			players[1] = Player(501, "Sid", false, 72, true);
	}	
	ClearScreenBlocks(25,26);	

	}
	return colinmode;
}



/////////////////////////DEBUG FUNCTIONS////////////////////////////////////

//  This will display a few useful (x,y) coords to the scoring window for debug purposes
void DisplayCursorLoc(int cursoractual[], int BGPos[]){
	
	int tilex = (cursoractual[0] + BGPos[0])/8;
	int tiley = (cursoractual[1] + BGPos[1])/8;
	int screenblock = 26;
	
	if (tilex > 30 && tiley < 31) {
		tilex -= 32;
		screenblock = 27;
	} else if (tiley > 30 && tilex < 31) {
		tiley -= 32;
		screenblock = 28;
	} else if (tiley > 30 && tilex > 31) {
		tilex -= 32;
		tiley -= 32;
		screenblock = 29;
	}

	DrawText(20, 5, "CursorPos", 25);
	DrawText(22, 6, "X:    ", 25);
	DrawText(22, 7, "Y:    ", 25);
	DrawNumber(24, 6, cursoractual[0]+BGPos[0], 25);
	DrawNumber(24, 7, cursoractual[1]+BGPos[1], 25);
	
	DrawText(22, 9, "BGPos", 25);
	DrawText(22, 10, "X:    ", 25);
	DrawText(22, 11, "Y:    ", 25);
	DrawNumber(24, 10, (BGPos[0])-30*8, 25);
	DrawNumber(24, 11, (BGPos[1])-30*8, 25);

	DrawText(22, 13, "SB:", 25);
	DrawNumber(25, 13, screenblock, 25);
	DrawText(22, 14, "X:    ", 25);
	DrawText(22, 15, "Y:    ", 25);	
	DrawNumber(24, 14, tilex, 25);
	DrawNumber(24, 15, tiley, 25);
}

//  This function displays the points value of an area, can be used for debug to check
//  that board is being correctly described.
//  IS USED in game to display points hit after a throw is made
void DisplayPointsValue(int cursoractual[], int BGPos[]){
	int x = (cursoractual[0] + BGPos[0])-30*8;
	int y = (cursoractual[1] + BGPos[1])-30*8;
	int h = x*x+y*y;
	float angle = atan2(y,x) * 180 / M_PI;
	int basepoints = 0;
	int type = 0;
	
	int index = (angle+9)/18+11;
	
	if (h < 23) {
		basepoints = 50;
		type = 5;
	} else if (h < 143.3) {
		basepoints = 25;
		type = 4;
	} else if (h < 5556.4) {
		type = 1;
	} else if (h < 6490.7) {
		type = 3;
	} else if (h < 14878) {
		type = 1;
	} else if (h < 16384) {
		type = 2;
	} else {
		basepoints = 0;
		type = 0;
	}
	DrawText(20, 17, "          ", 25);
	if (type == 1 || type == 2 || type == 3) {
		basepoints = boardpointsextended[index];
		if (type == 1) {
			DrawText(20, 17, "Single", 25);
		} else if (type == 2) {
			DrawText(20, 17, "Double", 25);
		} else {
			DrawText(20, 17, "Treble", 25);
		}
		DrawNumber(27, 17, basepoints, 25);	
	} else if (type == 0) {
		DrawText(23, 17, "Miss", 25);
	} else if (type == 4) {
		DrawText(23, 17, "Bull", 25);
	} else {
		DrawText(20, 17, "BullsEye", 25);
	}

}

//  draws a tile-based cursor for use in the menu system
//  takes coords for the starting and ending lines of the menu
//  returns the number option selected
int MenuSelect(int top, int bottom) {
	int setupcursor = top;


	SetTile(26, 4, setupcursor, 3);	
	bool wait = true;
	
	while(wait) {	
		WaitVSync();
		DrawText(4,setupcursor," ", 26);
		
		if ((REG_P1 & KEY_UP) == 0 && setupcursor > top && oldkey[UP] == false) {
			setupcursor--;
			oldkey[UP] = true;
		}
		if ((REG_P1 & KEY_UP) != 0) {
			oldkey[UP] = false;
		}
		if ((REG_P1 & KEY_DOWN) == 0 && setupcursor < bottom && oldkey[DOWN] == false) {
			setupcursor++;
			oldkey[DOWN] = true;
		}
		if ((REG_P1 & KEY_DOWN) != 0) {
			oldkey[DOWN] = false;
		}
		if ((REG_P1 & KEY_A) == 0 && oldkey[A_KEY] == false) {
			wait = false;
			oldkey[A_KEY] = true;
		}
		if ((REG_P1 & KEY_A) != 0) {
			oldkey[A_KEY] = false;
		}
		
		SetTile(26, 4, setupcursor, 3);	
	}

	return setupcursor-top;

}

//  DEBUG function for drawing a selected affine transformation matrix to the screen
void GetAff(int base)
{
	ObjAttr& obj(ObjBuffer[base]);
	DrawNumber (20, 12, obj.pad, 26);
	
	ObjAttr& obj2(ObjBuffer[base+1]);
	DrawNumber (20, 13, obj2.pad, 26);
	
	ObjAttr& obj3(ObjBuffer[base+2]);
	DrawNumber (20, 14, obj3.pad, 26);
		
	ObjAttr& obj4(ObjBuffer[base+3]);
	DrawNumber (20, 15, obj4.pad, 26);
}

