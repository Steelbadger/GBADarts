#include <stdlib.h>
#include "gba.h"
#include "class.h"
#include <math.h>
#include "functions.h"




//  Constructor, sets all initial values
Player::Player(int points,const char name[], bool ishuman, int difficulty, bool colinmode):
	p_points(points),
	p_ishuman(ishuman),
	p_colinaccuracy(difficulty),
	p_colinmode(colinmode)
{
	for (int i = 0; i < 4; i++) {
		p_name[i] = name[i];
	}
	for (int i = 0; i < 13; i++) {
		p_legwins[i] = 0;
	}
	p_setwins = 0;
	p_name[4] = '\0';
	p_throws = 0;
	p_turnthrows = 0;
	p_accuracy = difficulty * 16 + 50;
	p_darthitloc[0] = 0;
	p_darthitloc[1] = 0;
	p_colinaccuracy = difficulty;
}

//  Draw the players name to location (x,y)
void Player::DrawName(int x, int y){
	int i = 0;
	
	while (p_name[i] != '\0') {	
		SetTile(25, x, y, int(p_name[i]));
		x++;
		i++;
	}
}

//  Allows renaming of the player
void Player::SetName(const char name[]){
	for (int i = 0; i < 4; i++) {
		p_name[i] = name[i];
	}
	p_name[4] = '\0';
}

//  Allows resetting of current points
void Player::SetPoints(int points) {
	p_points = points;
}

//  Draw current points onto screen
void Player::DrawPoints(int x, int y) {
	// int tile = 0;
	y -= 4;
	y = (y > 12 ? 12 : y);
	if (y < 12) {
		p_pointsdisplay[y] = p_points;
		DrawNumber(x, y+4, p_points, 25);
		if (y > 0) {
			DrawText(x, y+3, "---", 26);
		}
	} else {	
		for (int i = 0; i < 11; i++) {
			p_pointsdisplay[i] = p_pointsdisplay[i+1];
		}
		p_pointsdisplay[11] = p_points;
		for (int i = 0; i < 12; i++) {
			SetTile(25, x, i+4, 0);
			SetTile(25, x+1, i+4, 0);
			SetTile(25, x+2, i+4, 0);
			DrawNumber(x, i+4, p_pointsdisplay[i], 25);
		}
	}
}

//  Reset a player read for the next leg of the game
void Player::ResetForNextLeg() {
	p_points = 501;
	p_darthitloc[0] = 0;
	p_darthitloc[1] = 0;	
	p_throws = 0;
	p_turnthrows = 0;	
}

//  Getter for players current points
int Player::GetPoints() {
	return p_points;
}

//  Take a 'turn' (throw three darts)
void Player::TakeTurn(int cursoractual[], int cursorpos[], int BGPos[], bool playertothrow) {
	int throws = 3;
	int wait = 0;
	p_turnthrows = 0;
	p_turnstartpoints = p_points;
	SetTile(27, 23+5*playertothrow, 1, 5);
	SetTile(27, 23+5*playertothrow-1, 1, 5);
	SetTile(27, 23+5*playertothrow-2, 1, 5);
	
	while ((throws > 0 && p_points > 0) || wait < 40) {
		UpdateObjects();
		
		if (throws > 0 && p_points > 0) {	
			WaitVSync();
			UpdateObjects();
			TakeAShot(cursoractual, cursorpos, BGPos);
			DrawPoints(20+5*playertothrow, 4 + p_throws);
			
			SetTile(27, 23+5*playertothrow-p_turnthrows, 1, 4);
			
			p_throws++;
			p_turnthrows++;
			throws--;
		} else {
			wait++;
		}
		
		for (int i = 0; i < p_turnthrows; i++) {
			darts[i].UpdateDart(BGPos);			
		}
	}
	SetTile(27, 23+5*playertothrow, 1, 4);
	SetTile(27, 23+5*playertothrow-1, 1, 4);
	SetTile(27, 23+5*playertothrow-2, 1, 4);
	ClearObjects();
}

//  Throw a single dart, handles both AI and Player controlled, if a player than enable cursor and wait for input
//  if computer then call target select functions and decide what to do with results by passing to DartLocationDecide()
void Player::TakeAShot(int cursoractual[], int cursorpos[],int BGPos[]){
	bool wait = true;
	oldkey[A_KEY] = true;

	if (p_ishuman == true) {		
		while (wait) {
			WaitVSync();
			UpdateObjects();
			HandleCursorMovement(cursoractual, cursorpos, BGPos);
			
			for (int i = 0; i < p_turnthrows; i++) {
				darts[i].UpdateDart(BGPos);			
			}
			
			if ((REG_P1 & KEY_A) == 0 && oldkey[A_KEY] == false) {
				GetPointsValue(cursoractual, BGPos);
				wait = false;
				oldkey[A_KEY] = true;
			}
			if ((REG_P1 & KEY_A) != 0) {
				oldkey[A_KEY] = false;
			}
		}
		if (p_colinmode) {
			Inaccuracy();
			DartLocationDecide();
			darts[p_turnthrows] = Dart(p_darthitloc[0], p_darthitloc[1], p_turnthrows+1);					
		} else {	
			darts[p_turnthrows] = Dart(cursoractual[0]+BGPos[0], cursoractual[1]+BGPos[1], p_turnthrows+1);	
		}
	} else {
		for (int animate = 0; animate < 10; animate++) {
			WaitVSync();
			UpdateObjects();
			for (int i = 0; i < p_turnthrows; i++) {
				darts[i].UpdateDart(BGPos);			
			}					
		}
		for (int i  = DELAY; i > 0; i--) {
			WaitVSync();
		}		
		TargetSelect(0);
		Inaccuracy();
		DartLocationDecide();
		BGPos[0] = p_darthitloc[0]-80;
		BGPos[1] = p_darthitloc[1]-80;
		
		REG_BG3HOFS = BGPos[0];
		REG_BG3VOFS = BGPos[1];	
		darts[p_turnthrows] = Dart(p_darthitloc[0], p_darthitloc[1], p_turnthrows+1);
	}
	if (CheckLegality(p_hit)) {
		p_points -= p_hit;	
		DrawText(20, 17, "          ", 25);
		if (p_targettype == 0) {
			DrawText(20, 17, "Single", 25);
			DrawNumber(27, 17, (p_hit), 25);
		} else if (p_targettype == 1) {
			DrawText(24, 17, "Bull", 25);
		} else if (p_targettype == 2) {
			DrawText(20, 17, "Double", 25);
			DrawNumber(27, 17, (p_hit/2), 25);
		} else if (p_targettype == 3) {
			DrawText(20, 17, "Treble", 25);
			DrawNumber(27, 17, (p_hit/3), 25);
		} else if (p_targettype == 4) {
			DrawText(21, 17, "BullsEye", 25);
		} else {
			DrawText(24, 17, "Miss", 25);
		}				
	} else {
		DrawText(20, 17, "  Illegal   ", 25);
	}

}

//  Find the points value of the point the cursor is over, uses radius and angle to calculate
void Player::GetPointsValue(int cursoractual[], int BGPos[]){
	
	int x = (cursoractual[0] + BGPos[0])-30*8;
	int y = (cursoractual[1] + BGPos[1])-30*8;
	int h = x*x+y*y;
	float angle = atan2(y,x) * 180 / M_PI;
	int basepoints = 0;
	int type = 0;
	p_targettype = 0;
	p_innersingle = false;
	
	int index = (angle+9)/18+11;
	
	if (h < sq_radii[0]) {
		basepoints = 50;
		type = 5;
		p_targettype = 4;
	} else if (h < sq_radii[1]) {
		basepoints = 25;
		type = 4;
		p_targettype = 1;
	} else if (h < sq_radii[2]) {
		type = 1;
		p_innersingle = true;
		p_targettype = 0;
	} else if (h < sq_radii[3]) {
		type = 3;
		p_targettype = 3;
	} else if (h < sq_radii[4]) {
		type = 1;
		p_targettype = 0;
	} else if (h < sq_radii[5]) {
		type = 2;
		p_targettype = 2;
	} else {
		basepoints = 0;
		type = 0;
		p_targettype = 5;
	}
	p_hit = basepoints;
	DrawText(20, 17, "          ", 25);
	if (type == 1 || type == 2 || type == 3) {
		basepoints = boardpointsextended[index];
		p_hit = basepoints * type;
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
	
	if (p_colinmode) {
		p_target = p_hit;
	}
}

//  AI Choose a target, choose highest legal points option.  If below 180 then check to see if current points are ODD, (ODD numbers canot be finished on)
//  Then recalculate target to aim for a treble or single odd, select highest possible
void Player::TargetSelect(int push){
	p_target = 0;
	p_targetindex = 0;
	p_targettype = 0;
	

	for (int i = 0; i < 20; i++) {
		if (boardpoints[i]+push < p_points && boardpoints[i] > p_target) {
			p_target = boardpoints[i];
			p_targetindex = i;
			p_targettype = 0;
			
		}
	}

	if (BULL+2 < p_points && BULL > p_target) {

		p_target = BULL;
		p_targettype = 1;
	}

	for (int i = 0; i < 20; i++) {
		if ((boardpoints[i]*2) == p_points && p_target != p_points) {
			p_target = boardpoints[i]*2;
			p_targetindex = i;
			p_targettype = 2;
		} else if ((boardpoints[i]*2)+push < p_points && ((boardpoints[i])*2) > p_target) {
			p_target = boardpoints[i]*2;
			p_targetindex = i;
			p_targettype = 2;
		}
	}

	for (int i = 0; i < 20; i++) {

		if ((boardpoints[i]*3)+push < p_points && ((boardpoints[i])*3) > p_target) {
			p_target = boardpoints[i]*3;
			p_targetindex = i;
			p_targettype = 3;
		}
	}

	if (BULLSEYE == p_points && p_target != p_points) {
		p_target = BULLSEYE;
		p_targettype = 4;
	} else if (BULLSEYE+push < p_points && BULLSEYE > p_target) {
		p_target = BULLSEYE;
		p_targettype = 4;
	}
	
	
	
	if (p_points < 180 && p_points % 2 == 1 && push == 0) {
		p_target = 0;
		
		for (int i = 0; i < 20; i++) {
			if ((boardpoints[i]*3) < p_points && ((boardpoints[i])*3) > p_target && (boardpoints[i]%2) == 1) {
				p_target = boardpoints[i]*3;
				p_targetindex = i;
				p_targettype = 3;
			}
		}
		
		for (int i = 0; i < 20; i++) {
			if (boardpoints[i] < p_points && boardpoints[i] > p_target && (boardpoints[i]%2) == 1) {
				p_target = boardpoints[i];
				p_targetindex = i;
				p_targettype = 0;
			}
		}			
		
	
	}

	if (!CheckLegality(p_target)){
		TargetSelect(push + 1);
	}
}

//  Choose applicable inaccuracy functions for current game mode and target type
void Player::Inaccuracy(){
	int i = 0;
	if (p_colinmode) {
		switch (p_targettype) {
			case 0: ColinSingle(); break;
			case 1: ColinSingle(); break;
			case 2: ColinDouble(); break;
			case 3: ColinTreble(); break;
			case 4: ColinBull(); break;		
		}
		if (p_targettype == 2 || p_targettype == 3) {
			for (i = 0; boardpoints[i] != p_hit/p_targettype; i++){	
			}
			p_targetindex = i;
		} else if (p_targettype == 0) {
			for (i = 0; boardpoints[i] != p_hit; i++) {		
			}
			p_targetindex = i;
		}
	} else {
		switch (p_targettype) {
			case 0: Single(); break;
			case 1: Bull(); break;
			case 2: Double(); break;
			case 3: Treble(); break;
			case 4: BullsEye(); break;
		}
	}
}

//  Works out AI misses for darts aimed at bullseye (50) when in Adam mode
void Player::BullsEye(){
	if (rand()/(RAND_MAX/100) > p_accuracy) {
		if (rand()/(RAND_MAX/100) > 20) {
			p_hit = BULL;
			p_targettype = 1;
		} else {
			p_hit = rand()/(RAND_MAX/20);
			p_targettype = 0;
		}
	} else {
		p_hit = p_target;
	}
}

//  Works out AI misses for darts aimed at the outer bull (25 ring) when in Adam Mode
void Player::Bull(){
	if (rand()/(RAND_MAX/100) < ((100-p_accuracy)*0.19)) {
		if (rand()/(RAND_MAX/100) > 50) {
			p_hit = BULLSEYE;
			p_targettype = 4;
		} else {
			p_hit = rand()/(RAND_MAX/20);
			p_targettype = 0;
		}
	} else {
		p_hit = p_target;
	}
}

//  Works out AI misses for darts aimed at a single when in Adam Mode
void Player::Single(){
	if (rand()/(RAND_MAX/100) < ((100-p_accuracy)*0.055)) {
		if (rand()/(RAND_MAX/100) > 50) {
			if (rand()/(RAND_MAX/100) > 50) {
				p_hit = p_target*3;
				p_targettype = 3;
			} else {
				p_hit = p_target*2;
				p_targettype = 2;
			}
		} else {
			if (rand()/(RAND_MAX/100) > 50) {
				if (p_targetindex == 19) {
					p_targetindex = 0;
				} else {
					p_targetindex++;
				}				
				p_hit = boardpoints[p_targetindex];
			} else {
				if (p_targetindex == 0) {
					p_targetindex = 19;
				} else {
					p_targetindex--;
				}				
				p_hit = boardpoints[p_targetindex];
			}
		}
	} else {
		p_hit = p_target;
	}	
}

//  Works out AI misses for darts aimed at a double when in Adam Mode
void Player::Double(){
	if (rand()/(RAND_MAX/100) < ((100-p_accuracy)*0.3)) {
		if (rand()/(RAND_MAX/100) > 20) {
			if (rand()/(RAND_MAX/100) > 50) {
				p_hit = 0;
				p_targettype = 5;
			} else {
				p_hit = p_target/2;
				p_targettype = 0;
			}
		} else {
			if (rand()/(RAND_MAX/100) > 50) {
				if (p_targetindex == 19) {
					p_targetindex = 0;
				} else {
					p_targetindex++;
				}		
				p_hit = boardpoints[p_targetindex]*2;
			} else {
				if (p_targetindex == 0) {
					p_targetindex = 19;
				} else {
					p_targetindex--;
				}				
				p_hit = boardpoints[p_targetindex]*2;
			}
		}
	} else {
		p_hit = p_target;
	}	
}

//  Works out AI misses for darts aimed at a treble when in Adam Mode
void Player::Treble(){
	if (rand()/(RAND_MAX/100) < ((100-p_accuracy)*0.5)) {
		if (rand()/(RAND_MAX/100) > 30) {
			p_hit = p_target/3;
			p_targettype = 0;
		} else {
			if (rand()/(RAND_MAX/100) > 50) {
				if (p_targetindex == 19) {
					p_targetindex = 0;
				} else {
					p_targetindex++;
				}			
				p_hit = boardpoints[p_targetindex]*3;
			} else {
				if (p_targetindex == 0) {
					p_targetindex = 19;
				} else {
					p_targetindex--;
				}				
				p_hit = boardpoints[p_targetindex]*3;
			}
		}
	} else {
		p_hit = p_target;
	}	
}

//  Returns true if points would be legal when hit is deducted from points
bool Player::CheckLegality(int hit){
	
	bool legality = false;
	
	if ((p_points - hit == 0 && (p_targettype == 2 || p_targettype == 4))|| p_points - hit > 1) {
		legality = true;
	} else {
		legality = false;
	}
	
	return legality;	
}

//  Places AI thrown darts on the dartboard for visual feedback of ai player.
//  Simply randomly places them in the sector for that points value
void Player::DartLocationDecide() {
	
	int radius = 0;
	int angle = 0;
	
	switch(p_targettype) {
		case 0: radius = rand()%(radii[4]-radii[3])+radii[3]; break;			//Outer Single Ring
		case 1: radius = rand()%(radii[1]-radii[0])+radii[0]; break;			//25 ring
		case 2: radius = rand()%(radii[5]-radii[4])+radii[4]; break;			//double ring
		case 3: radius = rand()%(radii[3]-radii[2])+radii[2]; break;			//treble ring
		case 4: radius = rand()%radii[0]; break;								//Bullseye
		default: radius = rand()%30 + radii[5];									//misses
	}
	
	angle = ((p_targetindex)*18)-90;
	angle += (rand()%19)-10;

	p_darthitloc[0] = radius * cos(angle*M_PI/180)+(30*8);
	p_darthitloc[1] = radius * sin(angle*M_PI/180)+(30*8);
	
}


///////////////////////////////////////////////////////////////////////////////
////////////////////////  Colin Mode Functions ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void Player::ColinBull() {
	
	//  Throw for the bull with accuracy p%  (20<p<85)
	
	int r = rand()%100;
	
	if(r<(p_colinaccuracy-20)) {
		p_hit = 50;
		p_targettype = 4;
	} else if(r<85)	{
		p_hit = 25;
		p_targettype = 1;
	} else {
		p_hit = 1+rand()%20;
		p_targettype = 0;
	}
}


void Player::ColinTreble() {

	//  return result of throwing for treble d with accuracy p%  (o<90)
	
	// Board neighbours ignoring slot zero
	int bd[2][21]={{0,20,15,17,18,12,13,19,16,14,6,8,9,4,11,10,7,2,1,3,5},
		       {0,18,17,19,13,20,10,16,11,12,15,14,5,6,9,2,8,3,4,7,1}};
	
	int r = rand()%100;
	
	if(r<p_colinaccuracy) {
		p_hit = p_target;
		p_targettype = 3;
	} else if(r<90) {
		p_hit = p_target/3;
		p_targettype = 0;
	} else if(r<93) {
		p_hit = 3*bd[0][p_target/3];
		p_targettype = 3;
		if (p_targetindex == 0) {
			p_targetindex = 19;
		} else {
			p_targetindex--;
		}	
	} else if (r<96) {
		p_hit = 3*bd[1][p_target/3];
		p_targettype = 3;
		if (p_targetindex == 19) {
			p_targetindex = 0;
		} else {
			p_targetindex++;
		}				
	} else if(r<98) {
		p_hit = bd[0][p_target/3];
		p_targettype = 0;
		if (p_targetindex == 0) {
			p_targetindex = 19;
		} else {
			p_targetindex--;
		}			
		
	} else {
		p_hit = bd[1][p_target/3];
		p_targettype = 0;
		if (p_targetindex == 19) {
			p_targetindex = 0;
		} else {
			p_targetindex++;
		}			
	}
}


void Player::ColinDouble() {
	
	//  return result of throwing for double d with accuracy 80%
	
	// Board neighbours ignoring slot zero
	int bd[2][21]={{0,20,15,17,18,12,13,19,16,14,6,8,9,4,11,10,7,2,1,3,5},
		       {0,18,17,19,13,20,10,16,11,12,15,14,5,6,9,2,8,3,4,7,1}};
	int r = rand()%100;
	
	if(r<80) {
		p_hit = p_target;
		p_targettype = 2;
	} else if(r<85) {
		p_hit = 0;
		p_targettype = 5;
	} else if(r<90) {
		p_hit = p_target/2;
		p_targettype = 0;		
	} else if (r<93) {
		p_hit = 2*bd[0][p_target/2];
		p_targettype = 2;
		if (p_targetindex == 0) {
			p_targetindex = 19;
		} else {
			p_targetindex--;
		}	
	} else if(r<96) {
		p_hit = 2*bd[1][p_target/2];
		p_targettype = 2;
		if (p_targetindex == 19) {
			p_targetindex = 0;
		} else {
			p_targetindex++;
		}			
	} else if (r<98) {
		p_hit = bd[0][p_target/2];
		if (p_targetindex == 0) {
			p_targetindex = 19;
		} else {
			p_targetindex--;
		}			
	} else {
		p_hit = bd[1][p_target/2];
		if (p_targetindex == 19) {
			p_targetindex = 0;
		} else {
			p_targetindex++;
		}			
	}
}


void Player::ColinSingle() {
	
	//  return result of throwing for single d with accuracy 88% (or 80% for the outer)
	
	// Board neighbours ignoring slot zero
	int bd[2][21]={{0,20,15,17,18,12,13,19,16,14,6,8,9,4,11,10,7,2,1,3,5},
		       {0,18,17,19,13,20,10,16,11,12,15,14,5,6,9,2,8,3,4,7,1}};
	int r = rand()%100;
	
	if(p_target==25){		// outer  80%
		if(r<80) {
			p_hit = 25;
			p_targettype = 1;
		} else if (r<90) {
			p_hit = 50;
			p_targettype = 4;
		} else {
			p_hit = 1+rand()%20;
		}
	}
	else			// 1 to 20 single
		if(r<88) {
			p_hit = p_target;
			p_targettype = 0;
		} else if(r<92) {
			p_hit = bd[0][p_target];
			p_targettype = 0;
			if (p_targetindex == 0) {
				p_targetindex = 19;
			} else {
				p_targetindex--;
			}			
		} else if(r<96) {
			p_hit = bd[1][p_target];
			p_targettype = 0;
			if (p_targetindex == 19) {
				p_targetindex = 0;
			} else {
				p_targetindex++;
			}				
		} else if (r<98) {
			p_hit = 3*p_target;
			p_targettype = 3;
		} else {
			p_hit = 2*p_target;
			p_targettype = 2;
		}
}


//  A Dart is a graphical representation of the imaginary darts help within the player class
//  This is only used for displaying darts on screen, has no other computational purpose
Dart::Dart(int x, int y, int dartnumber):
	dartnumber(dartnumber)
{
	loc[0] = x;
	loc[1] = y;
	age = 1;
	rot = rand()%90;
}

//  Keep the dart stationary relative to the dartboard (move with the dartboard), also allows dart to 
//  scroll off the screen, hides any darts that are entirely off the screen
void Dart::UpdateDart(int BGPos[]) {
	
	int actualloc[2];
	bool wrapping[2] = {false, false};
	actualloc[0] = loc[0] - BGPos[0]-2*8;
	actualloc[1] = loc[1] - BGPos[1]-2*8;

	if (actualloc[0] < 0) {
		actualloc[0] += 512;
		wrapping[0] = true;
	} 
	if (actualloc[1] < 0) {
		actualloc[1] += 256;
		wrapping[1] = true;
	}
	
	Animate();

	if ((wrapping[0] == true && actualloc[0]+8*8 < 512) || (wrapping[1] == true && actualloc[1]+8*8 < 256) ||
		(wrapping[0] == false && actualloc[0] > SCREEN_WIDTH) || (wrapping[1] == false && actualloc[1] > SCREEN_HEIGHT)) {
		SetObject(dartnumber,
		  ATTR0_SHAPE(0) | ATTR0_8BPP | ATTR0_REG | ATTR0_Y(0) | ATTR0_HIDE,
		  ATTR1_SIZE(2) | ATTR1_X(0),
		  ATTR2_ID8(64) | ATTR2_PRIO(2));				
	} else {		
		SetObject(dartnumber,
		  ATTR0_SHAPE(0) | ATTR0_8BPP | ATTR0_REG | ATTR0_Y(actualloc[1]) | ATTR0_AFF | ATTR0_AFF_DBL,
		  ATTR1_SIZE(2) | ATTR1_X(actualloc[0]) | ATTR1_AFF(dartnumber-1),
		  ATTR2_ID8(64) | ATTR2_PRIO(2));	  
	}
	
}

//  Alters dart affine matrix for purposes of the 'throwing' animation.
void Dart::Animate() {
	
	SetAffine(dartnumber-1, rot, 2.6-1.6*(age/10.0), 2.6-1.6*(age/10.0));

	if (age < 10) {
		age++;
	}
}


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////  Leg Set Match ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


Leg::Leg(bool playerthrow, int set):
	playertothrow(playerthrow),
	setnumber(set)
{}


// Play a single leg, resetting all necessary numbers before doing so.
void Leg::Play(Player players[], int BGPos[], int cursorpos[], int cursoractual[]) {
	
	players[0].ResetForNextLeg();
	players[1].ResetForNextLeg();
	ClearScreenBlocks(25,31);
	DrawDartboard();
	DrawWindow(19, 0, 11, 20);
	players[0].DrawName(20,2);
	players[1].DrawName(25,2);
	players[0].DrawPoints(20,3);
	players[1].DrawPoints(25,3);

	bool running = true;
	int pointsdisplay = 1;		//  remember number of lines of points displayed so far

	//  Game loop
	//  flip-flop between players 1 and 2, after each turn check for a winner
	while (running)
	{
		UpdateObjects();
		
		playertothrow = !playertothrow;
		players[playertothrow].TakeTurn(cursoractual, cursorpos, BGPos, playertothrow);
		
		if (playertothrow == 1) {	
			pointsdisplay++;
		}
		if (players[playertothrow].GetPoints() == 0) {
			running = false;
		}
	}
	
	//  Once someone wins clear screen and display winner
	ClearScreenBlocks(25,31);
	DrawWindow(0, 0, 30, 20);
	players[playertothrow].DrawName(13, 8);
	players[playertothrow].p_legwins[setnumber]++;
	DrawText(13, 10, "Wins the Leg!", 25);	
	for (int i = 0; i < DELAY; i++) {
		WaitVSync();
	}	
	
}

Set::Set(bool playerstart, int set):
	playertostart(playerstart),
	setnumber(set)
{
	bool temp = playertostart;
	
	for (int i = 0; i < 5; i++) {
		legs[i] = Leg(playertostart, setnumber);
		playertostart = !playertostart;
	}
	playertostart = temp;
	for (int i = 0; i < 11; i++) {
		tiebreaker[i] = Leg(playertostart, setnumber);
		playertostart = !playertostart;
	}
	playertostart = legs[4].playertothrow;
}

//  Play a single Set and record winner then display a screen showing who won
void Set::Play(Player players[], int BGPos[], int cursorpos[], int cursoractual[]) {
	bool running = true;
	int leg = 0;
	bool lastthrow = 0;
	
	while (running) {
		legs[leg].Play(players, BGPos, cursorpos, cursoractual);
		leg++;
		if (players[0].p_legwins[setnumber] == 3 || players[1].p_legwins[setnumber] == 3) {
			running = false;
		}
	
	}
	
	if (players[0].p_legwins[setnumber] == 3) {
		lastthrow = 0;
	} else {
		lastthrow = 1;
	}
 	ClearScreenBlocks(25,31);
	DrawWindow(0, 0, 30, 20);
	players[lastthrow].DrawName(13, 8);
	DrawText(13, 10, "Wins the Set!", 25);
	players[lastthrow].p_setwins++;
	
	for (int i = 0; i < DELAY; i++) {
		WaitVSync();
	}
}

//  Plays a special set for tiebreaker situations, requires a clear lead of 2 sets to win, or plays first to 6, unlike normal sets which
//  play best of 5 legs
void Set::PlayTieBreaker(Player players[], int BGPos[], int cursorpos[], int cursoractual[]) {
	bool running = true;
	int leg = 0;
	bool lastthrow = 0;
	
	while (running) {
		tiebreaker[leg].Play(players, BGPos, cursorpos, cursoractual);
		leg++;
		
		
		if (players[0].p_legwins[setnumber] == 6) {
			running = false;
			lastthrow = 0;
		}
		if (players[1].p_legwins[setnumber] == 6) {
			running = false;
			lastthrow = 1;
		}
		if (players[0].p_legwins[setnumber] - players[1].p_legwins[setnumber] >= 2 && players[0].p_legwins[setnumber] > 2) {
			running = false;
			lastthrow = 0;
		}
		if (players[1].p_legwins[setnumber] - players[0].p_legwins[setnumber] >= 2 && players[1].p_legwins[setnumber] > 2) {
			running = false;
			lastthrow = 1;
		}
	
	}
	
 	ClearScreenBlocks(25,31);
	DrawWindow(0, 0, 30, 20);
	players[lastthrow].DrawName(13, 8);
	DrawText(13, 10, "Wins the Set!", 25);
	players[lastthrow].p_setwins++;	
	
	for (int i = 0; i < DELAY; i++) {
		WaitVSync();
	}

}

Match::Match()
{
	playertostart = true;
	for (int i = 0; i < 13; i++) {
		sets[i] = Set(playertostart, i);
		playertostart = !playertostart;
	}
}

//  Play a full championship match (13 set) and display winner and table of results at end
void Match::Play(Player players[], int BGPos[], int cursorpos[], int cursoractual[]) {
	bool running = true;
	int set = 0;
	bool winner;
	
	while(running) {
	
		if (set < 12) {
			sets[set].Play(players, BGPos, cursorpos, cursoractual);
		} else {
			sets[set].PlayTieBreaker(players, BGPos, cursorpos, cursoractual);
		}
		set++;
		
		if (players[0].p_setwins == 7) {
			running = false;
			winner = 0;
			
		}
		if (players[1].p_setwins == 7) {
			running = false;
			winner = 1;
		}
	
	}
 	ClearScreenBlocks(25,31);
	DrawWindow(0, 0, 30, 20);
	players[0].DrawName(9, 1);
	players[1].DrawName(14, 1);
	for (int i = 0; i < set; i++) {
		DrawNumber(11, i+2, players[0].p_legwins[i], 25);
		DrawNumber(14, i+2, players[1].p_legwins[i], 25);
	}
	
	players[winner].DrawName(8, 18);
	DrawText(13,18, "Wins the Match!", 25);
	
	while(true) {}
	
};

