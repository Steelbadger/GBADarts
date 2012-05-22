#ifndef FUNCTIONS_H
#define FUNCTIONS_H

class Player;

const int boardpointsextended[22] =  {8, 11, 14, 9, 12, 5, 20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11};
const int boardpoints[20] =  {20, 1, 18, 4, 13, 6, 10, 15, 2, 17, 3, 19, 7, 16, 8, 11, 14, 9, 12, 5};
const int BULLSEYE = 50;
const int BULL = 25;
const float scalingfactor = 256/170;
const float realsizes[6] = {6.35, 16, 99, 107, 162, 170};
const int sq_radii[6] = {23, 145, 5556, 6491, 14878, 16384};
const int radii[6] = {5, 12, 74, 80, 122, 128};
enum notes {NOTE_C, NOTE_CS, NOTE_D, NOTE_DS, NOTE_E, NOTE_F, NOTE_FS, NOTE_G, NOTE_GS, NOTE_A, NOTE_AS, NOTE_B};
const int SOUNDFREQS[12] = {8013, 7566, 7144, 6742, 6362, 6005, 5666, 5346, 5048, 4766, 4499, 4246};
const int DELAY = 100;

#define SDMG_SQR1		0x01
#define SDMG_SQR2		0x02
#define SDMG_WAVE		0x04
#define SDMG_NOISE		0x08
#define SSTAT_ENABLE	(1<<7)
#define SSTAT_DISABLE	0x00

#define SND_RATE(note, octave) (2048-(SOUNDFREQS[note]>>(4+(octave))))
#define SDMG_BUILD(_lmode, _rmode, _lvol, _rvol)    \
    ( ((_lvol)&7) | (((_rvol)&7)<<4) | ((_lmode)<<8) | ((_rmode)<<12) )

#define SDMG_BUILD_LR(_mode, _vol) (SDMG_BUILD(_mode, _mode, _vol, _vol)<<16)


enum buttons{ UP, DOWN, LEFT, RIGHT, A_KEY, B_KEY, START };

static bool oldkey[8] = {true, true, true, true, true, true, true, true};

void LoadTilesAndSetDCNT();
void HandleCursorMovement(int cursoractual[], int cursorpos[], int BGPos[]);
void DrawDartboard();
void StartScreen();
void ClearScreenBlocks(int low, int high);
void DisplayCursorLoc(int cursoractual[], int BGPos[]);
void DisplayPointsValue(int cursoractual[], int BGPos[]);
void DisplayKeyboard(const char title[], char output[]);

int GetTile(int screenblock, int x, int y);

void DrawText(int, int, const char string[], int);
void DrawWindow(int x, int y, int width, int height);
void DrawNumber(int x, int y, int number, int screenblock);
void DrawFloat(int x, int y, float number, int screenblock);
void AnimateCursor(int cursoractual[]);
void SetupMenu(Player players[]);
int MenuSelect(int top, int bottom);
int DifficultyMenu();
bool ColinModeMenu(int cursoractual[], int cursorpos[], Player players[]);
void SetAffine(int matrix, int rot, float sx, float sy);
void DrawAffineDartboard();


void GetAff(int base);

#endif
