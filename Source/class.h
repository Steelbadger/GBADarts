#ifndef CLASS_H
#define CLASS_H



class Dart {

public:
	Dart(int x = 0, int y = 0, int dartnumber = 1);	
	void UpdateDart(int BGPos[]);	
private:
	int loc[2];
	int dartnumber;
	int age;
	int rot;
	
	void Animate();
};



class Player {

public:
	Player(int points = 501,const char name[] = "DFLT", bool ishuman = false, int difficulty = 0, bool colinmode = false);

	void DrawName(int x, int y);
	void SetName(const char name[]);	
	void SetPoints(int points);
	void DrawPoints(int x, int y);
	int GetPoints();
	void TakeTurn(int playeractual[], int cursorpos[],int BGPos[], bool playertothrow);
	void ResetForNextLeg();
	int p_legwins[13];
	int p_setwins;	
private:
	int p_points;
	int p_hit;
	char p_name[5];
	int p_target;
	int p_accuracy;
	int p_targetindex;
	int p_targettype;
	int p_throws;
	int p_turnthrows;
	int p_pointsdisplay[12];
	bool p_ishuman;
	bool p_innersingle;
	int p_darthitloc[2];
	int p_turnstartpoints;
	Dart darts[3];
	
	void TakeAShot(int playeractual[], int cursorpos[],int BGPos[]);
	
	void TargetSelect(int push);
	void GetPointsValue(int playeractual[], int BGPos[]);
	
	void Inaccuracy();
	void BullsEye();
	void Bull();
	void Single();
	void Double();
	void Treble();
	void DartLocationDecide();
	bool CheckLegality(int);
	
//  Colin-Mode Alternate functions /////

	int p_colinaccuracy;
	bool p_colinmode;

	void ColinBull();
	void ColinTreble();
	void ColinDouble();
	void ColinSingle();
};

class Leg {
public:
	Leg(bool playerthrow = 1, int set = 0);
	void Play(Player players[], int BGPos[], int cursorpos[], int cursoractual[]);
	int setnumber;
	bool playertothrow;
	
};

class Set {
public:
	Set(bool playerstart = 1, int set = 0);
	void Play(Player players[], int BGPos[], int cursorpos[], int cursoractual[]);
	void PlayTieBreaker(Player players[], int BGPos[], int cursorpos[], int cursoractual[]);
	int setnumber;
private:
	Leg legs[5];
	Leg tiebreaker[11];
	bool playertostart;
};

class Match {
public:
	Match();
	void Play(Player players[], int BGPos[], int cursorpos[], int cursoractual[]);
private:
	Set sets[13];
	bool playertostart;
};

#endif
