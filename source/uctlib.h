#pragma once
#include <cmath>
#include <cstdlib>
#include <ctime>

class CUCT;
class CUCTNode;
class CUCTGameInformation;

struct CUCTGameSubInformation {
	int player, value, isend;
};

namespace uct {
	typedef class CUCTNode nd;
	typedef class CUCTGameInformation inf;

	typedef struct CUCTGameSubInformation subinf;

	const int MaximizePlayer = 1;
	const int NullPlayer = 0;
	const int MinimizePlayer = -1;
}

class CUCT
{
public:
	CUCT(uct::inf*);
	~CUCT();

	uct::nd* Search(int* trg);
	uct::nd* gethd();
	uct::inf* getinf();

protected:
	uct::inf* const minf;
	uct::nd* hd;

	int Simulation();
	uct::nd* TreePolicy(uct::inf*);
	double DefaultPolicy(uct::inf*, uct::nd*);
	uct::nd* UCB1(uct::nd*, double c);
	virtual int Expand(uct::inf*, uct::nd*);
	virtual int RandomPlay(uct::inf*, int*);
	int Backup(uct::nd*, double z);

private:
	double ExplorationConst;
};

class CUCTNode
{
public:
	friend CUCT;

	CUCTNode(uct::nd*);
	~CUCTNode();

	int setinf(uct::subinf*);
	uct::subinf* getinf();

	int n, cnt, isexp; double q;
	uct::nd** descendant;

protected:
	uct::subinf ginf;
	uct::nd* const ancestor;
};

class CUCTGameInformation {
public:
	CUCTGameInformation();
	CUCTGameInformation(uct::subinf*);
	~CUCTGameInformation();
	virtual int apply(uct::nd*);
	virtual uct::inf* copy();

	uct::subinf ginf;

protected:
	void* data;
};