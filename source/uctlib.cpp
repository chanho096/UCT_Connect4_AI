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
		< root node �ֽ�ȭ >

		CUCT Ŭ������ �ʱ�ȭ�� �Ҵ�� minf �� hd ������ ������ �ִ�.
		CUCT �ʱ�ȭ ���Ŀ��� �������� ������������ ������ �ϰ�ʹٸ�,
		getinf �Լ��� ���� minf �� �����͸� �޾Ƽ� ���� ���� �� �� �ִ�.
	*/
	hd->setinf(&minf->ginf);

	// < Upper Confidence Bounded Tree Search >
	/*
		hd ������ UCT �� root node �̴�.
		UCT Search �� ginf �������� ���� ������ ã�Ƴ��� �Լ��̴�.
		��, hd �� �ڽĳ�� �� ���� ������ ���������� ��ȯ�Ѵ�.
	*/

	/*
		trg �� ���� �Ӱ谪 �̴�.
		������ ������ ����, Simulation �� ����ؼ� �ݺ��Ѵ�.
	*/
	t1 = clock();
	do {
		t2 = clock();
		Simulation();
	} while ((t2 - t1) < (*trg * CLOCKS_PER_SEC));

	/*
		�ùķ��̼� ���� ��, exploration const �� �Ұ��� UCB1 �� ��ȯ�Ѵ�.
		UCT Search �� ��ȯ����, ���� ������ ���������̴�.
		������ Simulation ���Ŀ��� ���� ���������� ���ٸ�, nullptr �� ��ȯ�Ѵ�.
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

	// �ݺ� Simulation �� ����� �ӽ� ����� ���������� �����Ѵ�.
	tmp = minf->copy();

	/*
		Tree Policy �� �̿��ؼ� Tree �� Ȯ���Ѵ�.
		Tree Policy �� Ȯ�� ���Ŀ� ���ϴ� ��带 ��ȯ�Ѵ�.
		�� �� ���������� ��ȯ�Ǵ� ������ �ֽ�ȭ�Ǿ��ִ� �����̾�� �Ѵ�.
	*/
	lf = TreePolicy(tmp);

	/*
		���ϴ� ��忡�� Default Policy �� �����Ѵ�.
		Default Policy �� ����� z �� ��ȯ�Ѵ�.
	*/
	z = DefaultPolicy(tmp, lf);

	// Backpropagation
	Backup(lf, z);

	// �ӽ� ����� ���������� �����Ѵ�.
	delete tmp;

	return 0;
}

uct::nd* CUCT::TreePolicy(uct::inf* tmpinf)
{
	using namespace uct;
	nd* node; node = hd;

	/*
		UCB1 �˰����� ����ؼ� Ʈ���� ���ϴܺκб��� ��������.
		���� ��ο� ���ܳ�尡 �����Ѵٸ�, ��� �ش� ���ܳ�带 ��ȯ�Ѵ�.
		Tree Policy �� ��ȯ �� ������, �ݵ�� ��ȯ�Ǵ� �������� ���������� �ֽ�ȭ �Ǿ� �־�� �Ѵ�.
	*/
	while (node->ginf.isend != 1) {
		if (!node->isexp) {
			/*
				�ڽĳ�尡 �������� ���� ��� Ȯ���Ѵ�.
				�ش� ��尡 ���ܳ�尡 �ƴ� ���, �ݵ�� �ּ� 1�� �̻��� �ڽ��� ���� �� �� ������ �����Ѵ�.
				Expand �� ������ �ڽ� �ϳ��� ��ȯ�ϱ� ����, ���������� �ݿ��ؾ� �Ѵ�.
			*/
			Expand(tmpinf, node);

			// �ڽĳ���� ������ ���������� �ݿ��Ѵ�.
			node = node->descendant[0];
			tmpinf->apply(node);

			return node;
		}
		// UCB1 �� ����Ͽ�, Ž���� �ڽĳ�带 �����Ѵ�.
		node = UCB1(node, ExplorationConst);

		// �ش� ����� ������ ���������� �ݿ��Ѵ�.
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
			�����Լ��� ���ǵ� ������ ������ �Ѵ�.
			RandomPlay �� ���� ������ ������ �ݺ��ϸ�, ���� �� Player ������ �Ѱ��ش�.
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
		�ش� ����� �ڽĵ� �� �������� ������ �ڽ��� ã�Ƽ� ��ȯ�Ѵ�.
		UCB1 �� Minimiax �������� �����Ͽ� �ۼ��� �˰��� �̴�.
		UCB1 �Լ��� ȣ���ϴ� ���� �ݵ�� �ּ� 1���� �ڽĳ�带 �������־�� �Ѵ�.
	*/
	player = node->ginf.player;
	OptimizedChild = 0; OptimizedValue = player * -1;
	for (i = 0; i < node->cnt; ++i) {
		// �ش� ����� �ڽĵ��� ��ȸ�Ѵ�.
		dsc = node->descendant[i];

		// ���� �湮�� �ڽ��� ��� ��ȯ�Ѵ�.
		if (dsc->n == 0)
			return dsc;
		
		// UCB1
		ExploitationTerm = dsc->q / dsc->n;
		ExplorationTerm = sqrt((2 * log(node->n)) / dsc->n);
		z = ExploitationTerm + player * (ExplorationConst * ExplorationTerm);
		
		// ������ ����
		if (z * player > OptimizedValue * player) {
			OptimizedChild = i;
			OptimizedValue = z;
		}
	}

	return node->descendant[OptimizedChild];
}

int CUCT::Expand(uct::inf* tmpinf, uct::nd* node)
{
	// ���������� �������� �μ��� ���� ����� �ڽĵ��� ����
	return 0;
}

int CUCT::RandomPlay(uct::inf* tmpinf, int* win)
{
	// �ش� ������������ ���� ���� �ǽ�
	// ������ ���� �� ��� ���� ��ȯ�ϸ�, ���� �� ���������� win ������ ���� ��Ÿ����.
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
	// �ڽŰ� ���� Ŭ������ ���������Ͽ� ��ȯ�ϴ� �Լ�
	return nullptr;
}
