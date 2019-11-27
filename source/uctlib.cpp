#include "uctlib.h"

CUCT::CUCT(uct::inf* minf):minf(minf),hd(new uct::nd(nullptr))
{	
	ExplorationConst = 0.1;
}

CUCT::~CUCT()
{
	delete hd;
}

uct::nd* CUCT::Search(int* trg)
{
	using namespace uct;
	clock_t t1, t2;
	srand((int)time(NULL));

	/*
		< root node 최신화 >

		CUCT 클래스는 초기화시 할당된 minf 와 hd 정보를 가지고 있다.
		CUCT 초기화 이후에도 임의적인 게임정보에서 연산을 하고싶다면,
		getinf 함수를 통해 minf 의 포인터를 받아서 직접 수정 할 수 있다.
	*/
	hd->setinf(&minf->ginf);

	// < Upper Confidence Bounded Tree Search >
	/*
		hd 변수는 UCT 의 root node 이다.
		UCT Search 는 ginf 시점에서 최적 선택을 찾아내는 함수이다.
		곧, hd 의 자식노드 중 최적 선택의 게임정보를 반환한다.
	*/

	/*
		trg 는 연산 임계값 이다.
		연산이 가능한 동안, Simulation 을 계속해서 반복한다.
	*/
	t1 = clock();
	do {
		t2 = clock();
		Simulation();
	} while ((t2 - t1) < (*trg * CLOCKS_PER_SEC));

	/*
		시뮬레이션 종료 후, exploration const 를 소거한 UCB1 을 반환한다.
		UCT Search 의 반환값은, 최적 선택의 게임정보이다.
		하지만 Simulation 이후에도 착수 가능지점이 없다면, nullptr 을 반환한다.
	*/
	if (hd->cnt == 0)
		return nullptr;
	else
		return UCB1(hd, 0);
}

uct::nd* CUCT::gethd()
{
	return hd;
}

uct::inf* CUCT::getinf()
{
	return minf;
}

int CUCT::Simulation()
{
	using namespace uct;
	inf* tmp; nd* lf; double z;

	// 반복 Simulation 에 사용할 임시 저장용 게임정보를 생성한다.
	tmp = minf->copy();

	/*
		Tree Policy 를 이용해서 Tree 를 확장한다.
		Tree Policy 는 확장 이후에 최하단 노드를 반환한다.
		이 때 게임정보는 반환되는 노드까지 최신화되어있는 상태이어야 한다.
	*/
	lf = TreePolicy(tmp);

	/*
		최하단 노드에서 Default Policy 를 실행한다.
		Default Policy 는 결과값 z 를 반환한다.
	*/
	z = DefaultPolicy(tmp, lf);

	// Backpropagation
	Backup(lf, z);

	// 임시 저장용 게임정보를 삭제한다.
	delete tmp;

	return 0;
}

uct::nd* CUCT::TreePolicy(uct::inf* tmpinf)
{
	using namespace uct;
	nd* node; node = hd;

	/*
		UCB1 알고리즘을 사용해서 트리의 최하단부분까지 내려간다.
		만약 경로에 종단노드가 존재한다면, 즉시 해당 종단노드를 반환한다.
		Tree Policy 가 반환 될 때에는, 반드시 반환되는 노드까지의 게임정보가 최신화 되어 있어야 한다.
	*/
	while (node->ginf.isend != 1) {
		if (!node->isexp) {
			/*
				자식노드가 존재하지 않을 경우 확장한다.
				해당 노드가 종단노드가 아닐 경우, 반드시 최소 1개 이상의 자식을 생성 할 수 있음을 전제한다.
				Expand 로 생성된 자식 하나를 반환하기 전에, 게임정보에 반영해야 한다.
			*/
			Expand(tmpinf, node);

			// 자식노드의 착수를 게임정보에 반영한다.
			node = node->descendant[0];
			tmpinf->apply(node);

			return node;
		}
		// UCB1 을 사용하여, 탐색할 자식노드를 선택한다.
		node = UCB1(node, ExplorationConst);

		// 해당 노드의 착수를 게임정보에 반영한다.
		tmpinf->apply(node);
	}
	return node;
}

double CUCT::DefaultPolicy(uct::inf* tmpinf, uct::nd* leaf)
{
	using namespace uct;
	double z; int chk, win;

	chk = 0; win = 0;
	while (1) {
		/*
			가상함수로 정의된 무작위 실행을 한다.
			RandomPlay 는 임의 실행을 무한히 반복하며, 종료 시 Player 정보를 넘겨준다.
		*/
		chk = RandomPlay(tmpinf, &win);
		if (chk) {
			switch (win) {
			case MaximizePlayer :
				z = 1;
				break;
			case MinimizePlayer :
				z = 0;
				break;
			default :
				z = 0.5;
			}
			break;
		}
	}

	return z;
}

uct::nd* CUCT::UCB1(uct::nd* node, double c)
{
	using namespace uct; nd* dsc;
	double ExploitationTerm, ExplorationTerm;
	int player, i; double z;
	int OptimizedChild; double OptimizedValue;

	/*
		해당 노드의 자식들 중 최적값을 가지는 자식을 찾아서 반환한다.
		UCB1 을 Minimiax 형식으로 변형하여 작성한 알고리즘 이다.
		UCB1 함수를 호출하는 노드는 반드시 최소 1개의 자식노드를 가지고있어야 한다.
	*/
	player = node->ginf.player;
	OptimizedChild = 0; OptimizedValue = player * -1;
	for (i = 0; i < node->cnt; ++i) {
		// 해당 노드의 자식들을 순회한다.
		dsc = node->descendant[i];

		// 최초 방문한 자식은 즉시 반환한다.
		if (dsc->n == 0)
			return dsc;
		
		// UCB1
		ExploitationTerm = dsc->q / dsc->n;
		ExplorationTerm = sqrt((2 * log(node->n)) / dsc->n);
		z = ExploitationTerm + player * (ExplorationConst * ExplorationTerm);
		
		// 최적값 갱신
		if (z * player > OptimizedValue * player) {
			OptimizedChild = i;
			OptimizedValue = z;
		}
	}

	return node->descendant[OptimizedChild];
}

int CUCT::Expand(uct::inf* tmpinf, uct::nd* node)
{
	// 게임정보를 바탕으로 인수로 받은 노드의 자식들을 생성
	return 0;
}

int CUCT::RandomPlay(uct::inf* tmpinf, int* win)
{
	// 해당 게임정보에서 임의 실행 실시
	// 게임이 종료 될 경우 값을 반환하며, 종료 시 승자정보를 win 변수를 통해 나타낸다.
	return 0;
}

int CUCT::Backup(uct::nd* leaf, double z)
{
	using namespace uct;
	nd* tmp; tmp = leaf;

	while (tmp != NULL) {
		tmp->n += 1;
		tmp->q += z;
		tmp = tmp->ancestor;
	}

	return 0;
}

CUCTNode::CUCTNode(uct::nd* node):ancestor(node)
{
	ginf.player = 0; ginf.value = 0; ginf.isend = 0;
	n = 0; q = 0; cnt = 0; isexp = 0;
	descendant = nullptr;
}

CUCTNode::~CUCTNode()
{
	int i;

	if (isexp)
		for (i = 0; i < cnt; ++i)
			delete descendant[i];
	delete descendant;
}

int CUCTNode::setinf(uct::subinf* newinf)
{
	ginf.isend = newinf->isend;
	ginf.player = newinf->player;
	ginf.value = newinf->value;

	return 0;
}

uct::subinf* CUCTNode::getinf()
{
	return &ginf;
}

CUCTGameInformation::CUCTGameInformation()
{
	ginf.player = 0;
	ginf.value = 0;
	ginf.isend = 0;
	data = nullptr;
}

CUCTGameInformation::CUCTGameInformation(uct::subinf* newinf)
{	
	ginf.player = newinf->player;
	ginf.value = newinf->value;
	ginf.isend = newinf->isend;
	data = nullptr;
}


CUCTGameInformation::~CUCTGameInformation()
{
	delete data;
}

int CUCTGameInformation::apply(uct::nd* node)
{
	return 0;
}

uct::inf* CUCTGameInformation::copy()
{
	// 자신과 같은 클래스를 깊은복사하여 반환하는 함수
	return nullptr;
}
