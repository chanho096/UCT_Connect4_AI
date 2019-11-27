#pragma once
#include "uctlib.h"
#include "windows.h"
#include <string>
#include <iostream>

using namespace uct;
using namespace std;

#define Connect4Width 7
#define Connect4Height 6
#define Connect4Size 42

#define Node_Default -1

class CConnect4;
class CConnect4GameInformation;
class CConnect4UCT;
class CConnect4InputOutputSystem;

typedef CConnect4 cn4;
typedef CConnect4GameInformation cn4inf;
typedef CConnect4UCT cn4uct;
typedef CConnect4InputOutputSystem cn4io;

class CConnect4
{
public:
	CConnect4();
	~CConnect4();

	static int chkend(cn4inf*, int player, int point, int* win);
	static int setpos(cn4inf*, int player, int pos);
	static int setpnt(cn4inf*, int player, int point);

private:
	static int getbff(int[4][9], cn4inf*, int point);
	static int chkcnt(int* bff, int player);

};

class CConnect4GameInformation : public inf
{	
public:
	CConnect4GameInformation(subinf*);
	~CConnect4GameInformation();

	int apply(uct::nd*);
	inf* copy();

	int* data, *top;
private:
	
};

class CConnect4UCT : public CUCT
{
public:
	CConnect4UCT(cn4inf*);
	~CConnect4UCT();

	int Expand(inf*, nd*);
	int RandomPlay(inf*, int*);

};

class CConnect4InputOutputSystem
{
public:
	CConnect4InputOutputSystem();
	~CConnect4InputOutputSystem();

	typedef struct Connect4CommandInformation {
		cn4inf* minf;
		int trg;
		int player;
		int chkend; int chkwin;
	} cmdinf;

	void Run();

private:
	cmdinf cmd;

	void Initialize();
	void Print();
	void Message();
	void Ready();
	void Main();
};