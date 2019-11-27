#include "Connect4.h"

CConnect4::CConnect4()
{
}

CConnect4::~CConnect4()
{
}

int CConnect4::chkend(cn4inf* minf, int player, int point, int* win)
{
	int bff[4][9];
	int i;

	*win = NullPlayer;
	// �� ���� �ֻ�� Ȯ��
	for (i = 0; i < Connect4Width; ++i) {
		if (minf->top[i] >= 0)
			break;
	}
	if (i == Connect4Width)
		return 1;

	// Buffer ����
	getbff(bff, minf, point);

	// �¸����� �Ǵ�
	for (i = 0; i < 4; ++i)
		if (chkcnt(bff[i], player) == 1) {
			*win = player;
			return 1;
		}

	return 0;
}

int CConnect4::setpos(cn4inf* minf, int player, int pos)
{
	int point;

	if (minf->top[pos] >= 0) {
		point = minf->top[pos]-- * Connect4Width + pos;
		minf->data[point] = player;
		minf->ginf.value = point;
		return point;
	}
	else
		return -1;
}

int CConnect4::setpnt(cn4inf* minf, int player, int point)
{
	int pos;
	pos = point % Connect4Width;
	
	if (minf->top[pos] >= 0) {
		minf->data[point] = player;
		minf->top[pos]--;
		minf->ginf.value = point;
		return point;
	}
	else
		return -1;
}

int CConnect4::getbff(int bff[4][9], cn4inf* minf, int point)
{
	int cnt, i, j, r, c, dr[4], dc[4], rt, ct;
	int Row, Column;
	int Width, Height, Size;

	// ���� �ʱ�ȭ
	Width = Connect4Width;
	Height = Connect4Height;
	Size = Connect4Size;

	dr[0] = 1; dr[1] = 0; dr[2] = 1; dr[3] = 1;
	dc[0] = 0; dc[1] = 1; dc[2] = 1; dc[3] = -1;

	Row = point / Width;
	Column = point % Width;

	for (i = 0; i < 4; ++i) {
		// ���� ������ �Ҵ��Ѵ�
		r = Row; c = Column; cnt = 4;

		rt = -1 * dr[i]; ct = -1 * dc[i];
		while (--cnt > 0) {
			if ((r < 0 || r >= Height) || (c < 0 || c >= Width))
				break;
			if (dr[i] != 0 && ((r == 0 && rt < 0) || ((r == Height - 1) && rt > 0)))
				break;
			if (dc[i] != 0 && ((c == 0 && ct < 0) || ((c == Width - 1) && ct > 0)))
				break;
			r += rt; c += ct;
		}

		// Buffer �ʱ�ȭ
		for (j = 0; j < 9; ++j)
			bff[i][j] = -1;

		// ������������ ���� 9ĭ�� ������ ��´�
		rt = dr[i]; ct = dc[i];
		cnt = 0;
		while (cnt < 9) {
			if ((r < 0 || r >= Height) || (c < 0 || c >= Width))
				break;
			bff[i][cnt++] = r * Width + c;

			if (dr[i] != 0 && ((r == 0 && rt < 0) || ((r == Height - 1) && rt > 0)))
				break;
			if (dc[i] != 0 && ((c == 0 && ct < 0) || ((c == Width - 1) && ct > 0)))
				break;
			r += rt; c += ct;
		}

		// Buffer �� ������ �޾ƿ´�
		for (j = 0; j < 9; ++j) {
			if (bff[i][j] == -1)
				bff[i][j] = 0;
			else
				bff[i][j] = minf->data[bff[i][j]];
		}
	}

	return 0;
}

int CConnect4::chkcnt(int* bff, int player)
{
	int i, cnt;

	cnt = 0;
	for (i = 0; i < 9; ++i) {
		if (bff[i] == player)
			++cnt;
		else
			cnt = 0;

		if (cnt >= 4)
			return 1;
	}

	return 0;
}

CConnect4GameInformation::CConnect4GameInformation(subinf* newinf) : inf(newinf)
{
	int i;

	inf::data = new int[Connect4Size];
	for (i = 0; i < Connect4Size; ++i)
		*((int*)inf::data + i) = NullPlayer;
	/*
		�� cn4inf �� ��������ڸ� ��� �� �� ����.
		cn4inf ���� �� inf �� �����ڰ� ���� ȣ��Ǿ� data �����Ϳ� ���� �Ҵ� �� �� �̴�.
		������, cn4inf ���ο� data �޸� �Ҵ� �Լ��� �����ϹǷ�, ��������� ��� �� ������ �߻� �� �� �̴�.
	*/
	data = (int*)inf::data;

	top = new int[Connect4Width];
	for (i = 0; i < Connect4Width; ++i)
		top[i] = Connect4Height - 1;
}

CConnect4GameInformation::~CConnect4GameInformation()
{
	delete top;
}

int CConnect4GameInformation::apply(uct::nd* node)
{
	int player, point, tmp;
	subinf* sub; sub = node->getinf();
	tmp = 0;

	/*
		�� cn4inf �� node �� ������ �ִ� subinf ������ �ľ��� ��.
		- player : ���� ���ӻ��¿��� �����ؾ� �� ����� ����Ų��.
		- value (= point) : ���������� ������ ��ġ ���� ����Ѵ�.
		��, subinf �� opp (player) �� value ��ġ�� �����߾��� ���� ����Ѵ�.

		�ش� �Լ��� node �� ���������� �����ϸ鼭, cn4inf ���� ����ȭ�� ���ÿ� �Ѵ�.
	*/
	player = sub->player * -1; point = sub->value;
	point = cn4::setpnt(this, player, point);

	// cn4inf �� ���� �ֽ�ȭ
	ginf.player = sub->player;
	ginf.value = sub->value;

	// ����� ���� �Ҵ� ��, ���ܿ��θ� Ȯ���Ѵ�.
	if (sub->isend == Node_Default) {
		sub->isend = cn4::chkend(this, player, point, &tmp);
		node->setinf(sub);
		ginf.isend = sub->isend;
	}

	return 0;
}

inf* CConnect4GameInformation::copy()
{
	int i;
	cn4inf* newinf; newinf = new cn4inf(&ginf);
	for (i = 0; i < Connect4Size; ++i)
		newinf->data[i] = data[i];
	for (i = 0; i < Connect4Width; ++i)
		newinf->top[i] = top[i];

	return newinf;
}

CConnect4UCT::CConnect4UCT(cn4inf* minf) : CUCT(minf)
{
}

CConnect4UCT::~CConnect4UCT()
{
}

int CConnect4UCT::Expand(inf* minf, nd* node)
{
	int i, cnt;
	subinf tmp, *sub;
	
	sub = node->getinf();
	node->cnt = 0;

	/*
		node �� ���������� �ڽ� ���� Ȯ���Ѵ�.
		����, �ڽ� ��带 ���� �� �� ���� ��츦 �����Ѵٸ� cnt ������ Ȯ���ϸ� �ȴ�.
		������, �ش� �˰��򿡼��� �ݵ�� �ּ� 1�� �̻��� �ڽ��� ���� �� �� ������ �����Ѵ�.
	*/
	for (i = 0; i < Connect4Width; ++i)
		if (static_cast<cn4inf*>(minf)->top[i] >= 0)
			node->cnt++;

	cnt = node->cnt;

	node->isexp = 1;
	node->descendant = new nd*[cnt];

	tmp.isend = Node_Default;
	tmp.player = sub->player * -1;
	tmp.value = -1;

	cnt = 0;
	for (i = 0; i < Connect4Width; ++i)
		if (static_cast<cn4inf*>(minf)->top[i] >= 0) {
			tmp.value = static_cast<cn4inf*>(minf)->top[i] * Connect4Width + i;
			node->descendant[cnt] = new nd(node);
			node->descendant[cnt++]->setinf(&tmp);
		}

	return 0;
}

int CConnect4UCT::RandomPlay(inf *minf, int *win)
{
	int chk, pos, cnt, i, *action;
	action = new int[Connect4Width];

	// �켱, ���� ���°� ����� �������� �Ǵ��Ѵ�.
	chk = cn4::chkend((cn4inf*)minf, minf->ginf.player * -1, minf->ginf.value, win);
	if (chk )
		return 1;

	// �ش� ������������ ���� ���������� �ҷ��´�
	for (cnt = 0, i = 0; i < Connect4Width; ++i)
		if (static_cast<cn4inf*>(minf)->top[i] >= 0)
			action[cnt++] = i;

	// ���� ���� ���� ��, ���� �ϳ��� �����Ѵ�.
	pos = rand() % cnt;
	pos = action[pos];

	// ����� �׼��� �����Ѵ�.
	cn4::setpos((cn4inf*)minf, minf->ginf.player, pos);
	minf->ginf.player *= -1;
	delete action;

	return 0;
}

CConnect4InputOutputSystem::CConnect4InputOutputSystem()
{
	subinf ginf;
	ginf.player = 0; ginf.value = 0; ginf.isend = 0;
	cmd.minf = new cn4inf(&ginf);
}

CConnect4InputOutputSystem::~CConnect4InputOutputSystem()
{
	delete cmd.minf;
}

void CConnect4InputOutputSystem::Run()
{
	Initialize();
	Main();
}

void CConnect4InputOutputSystem::Initialize()
{
	char d_char; int chk;

	string init1 = ("**********************  Initialize  **********************\n\n");
	string init2 = (" �� ���α׷� ���� �ʱ�ȭ �۾��� �����մϴ�. \n\n");
	string init3 = ("\n**********************************************************\n");

	system("cls");
	cout << init1 << init2 << " �� Computer ���� �ð� ����\n\n �� a. 1 sec\n\n �� b. 3 sec\n\n �� c. 10 sec" << endl << init3;;

	chk = 1;
	while (chk) {
		printf(" : ");
		cin >> d_char;
		getchar();

		if (d_char >= 'a' && d_char <= 'c') {
			switch (d_char) {
			case 'a':
				cmd.trg = 1;
				break;
			case 'c':
				cmd.trg = 10;
				break;
			default:
				cmd.trg = 3;
			}
			chk = 0;
		}
	}

	system("cls");
	cout << init1 << init2 << " �� Player ����\n\n �� a. First Player\n\n �� b. Second Player" << endl << init3;

	chk = 1;
	while (chk) {
		printf(" : ");
		cin >> d_char;
		getchar();

		if (d_char >= 'a' && d_char <= 'b') {
			switch (d_char) {
			case 'a':
				cmd.player = MaximizePlayer;
				break;
			default:
				cmd.player = MinimizePlayer;
			}
			chk = 0;
		}
	}

	cmd.minf->ginf.player = MaximizePlayer;
	cmd.minf->ginf.value = -1;
	cmd.minf->ginf.isend = Node_Default;
	cmd.chkend = 0; cmd.chkwin = 0;
}

void CConnect4InputOutputSystem::Print()
{
	int i, j, num;
	cout << "******************** Game Information ********************" << endl;
	for (i = 0; i < Connect4Height; ++i) {
		printf(" %2d", Connect4Height - i);
		for (j = 0; j < Connect4Width; ++j) {
			num = i * Connect4Width + j;
			switch (cmd.minf->data[num]) {
			case MaximizePlayer:
				printf("��");
				break;
			case MinimizePlayer:
				printf("��");
				break;
			default:
				printf("  ");
			}
		}
		printf("\n");
	}

	printf("   ");
	for (i = 0; i < Connect4Width; ++i) {
		printf("%2c", 'A' + i);
	}
	cout << "\n**********************************************************" << endl;
}

void CConnect4InputOutputSystem::Message()
{
	system("cls");
	cout << "2018. 11. 29. Connect4 AI" << endl;;
	cout << "- Using Monte-Carlo Tree Search Algorithm\n" << endl;;

}

void CConnect4InputOutputSystem::Ready()
{
	char d_char;
	system("pause");

	while (1) {
		system("cls");
		cin >> d_char;
	}
}

void CConnect4InputOutputSystem::Main()
{
	char d_char;
	int pos, point, last, time; double rate;
	cn4uct* cuct; cn4inf* uctinf; nd* tmp;
	//cuct = new cn4uct(cn4cmd->minf);
	//uctinf = static_cast<cn4inf*>(cuct->getinf());

	last = -1;
	while (1) {
		Message();
		Print();

		if (last != -1) {
			if (cmd.player == MaximizePlayer)
				rate = 1 - rate;
			rate *= 100;

			// ��ǻ�� ���� ��ġ ǥ��.
			cout.setf(ios::fixed);
			cout.precision(2);
			cout << " �� Computer Set " << char('A' + (point % Connect4Width)) << Connect4Height - point / Connect4Width << endl;
			cout << " �� " << time << " Times / " << rate << "%";
			if (rate >= 90)
				cout << " :)" << endl;
			else if (rate <= 10)
				cout << " :(" << endl;
			else
				cout << endl;
		}

		if (cmd.chkend == 1) {
			if (cmd.chkwin == cmd.player)
				cout << endl << "�ڡڡڡڡڡڡڡڡڡ� Player Win ! �ڡڡڡڡڡڡڡڡڡ�" << endl;
			else if (cmd.chkwin == NullPlayer)
				cout << endl << "�ڡڡڡڡڡڡڡڡڡ� Draw �ڡڡڡڡڡڡڡڡڡ�\n" << endl;
			else
				cout << endl << "�ڡڡڡڡڡڡڡڡڡ� Computer Win ! �ڡڡڡڡڡڡڡڡڡ�\n" << endl;

			Ready();
		}

		if (cmd.minf->ginf.player == cmd.player) {
			// Player Turn
			cout << " �� Player Turn (Press a ~ " << char('a' + Connect4Width - 1) << ")" << endl;
			printf(" : ");
			cin >> d_char;
			getchar();

			pos = d_char - 'a';
			uctinf = cmd.minf;
			if (pos >= 0 && pos < Connect4Width && uctinf->top[pos] >= 0) {
				// �������� ����
				point = cn4::setpos(uctinf, uctinf->ginf.player, pos);
				cmd.chkend = cn4::chkend(uctinf, uctinf->ginf.player, point, &cmd.chkwin);
				uctinf->ginf.player *= -1;
			}
			last = -1; time = 0; rate = 0;
		}
		else {
			// Computer Turn
			cuct = new cn4uct(cmd.minf); //^
			uctinf = static_cast<cn4inf*>(cuct->getinf()); //^

			cout << " �� Computer Turn (" << cmd.trg << " Sec)" << endl;
			tmp = cuct->Search(&cmd.trg); point = tmp->getinf()->value;
			cn4::setpnt(uctinf, uctinf->ginf.player, point);
			cmd.chkend = cn4::chkend(uctinf, uctinf->ginf.player, point, &cmd.chkwin);
			uctinf->ginf.player *= -1;

			last = point; time = cuct->gethd()->n; rate = tmp->q / tmp->n;

			delete cuct; //^
			cuct = nullptr; //^
		}
	}

	//delete cuct;
}

