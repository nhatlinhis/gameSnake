#define _WIN32_WINNT 0x0500
#include<conio.h>
#include<Windows.h>
#include<iostream>
#include<wincon.h>
#include<string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
using namespace std;

#define MAX_SIZE_SNAKE 30
#define MAX_SIZE_FOOD 4
#define MAX_SPEED 3

POINT operator + (POINT const& a, POINT const& b) {
	return { a.x + b.x,a.y + b.y };
}

bool operator == (POINT const& a, POINT const& b) {
	return (a.x == b.x) && (a.y == b.y);
}

class SnakeGame{
public:

    string SNAKE_NAME, ROCK;
    POINT snake[30]; //Con rắn
    POINT gate[5];
    POINT rock[20]; // vị trí đầu của vật cản
    POINT food[4]; // Thức ăn
    POINT step[4] = { {-1,0},{1,0},{0,-1},{0,1} };// bước đi tiếp theo
    int CHAR_LOCK;//Biến xác định hướng không thể di chuyển (Ở một thời điểm có một hướng rắn không thể di chuyển)
    int MOVING;//Biến xác định hướng di chuyển của snake (Ở một thời điểm có ba hướng rắn có thể di chuyển)
    int SPEED;// Có thể hiểu như level, level càng cao thì tốc độ càng nhanh
    int HEIGH_CONSOLE, WIDTH_CONSOLE;// Độ rộng và độ cao của màn hình console
    int FOOD_INDEX; // Chỉ số food hiện hành đang có trên màn hình
    int SIZE_SNAKE; // Kích thước của snake, lúc đầu có 6 và tối đa lên tới 10
    int STATE; // Trạng thái sống hay chết của rắn
    int score, level, food_ate;

    void clearSnake();
    void GotoXY(int , int );
    bool IsValid(int , int );
    void GenerateFood();
    void ResetData();
    void NewGame();
    void DrawBoard(int, int, int, int);
    void ExitGame();
    void PauseGame();
    void ThreadFunc();
    void clearRock();
    void throughGate();
    void Eat1();
    void Eat2();
    void ProcessDead();
    int playGame();
    int check();
    void startGame();
    void drawRock();
    bool checkRock(int, int);
    void createRock();
    void drawFood();
    void drawGate();
    int Move(POINT);
    void saveGame();
    void drawScore();
    void drawSnake();
    void clearGate();
    void effectDead();
    bool IsValid2(POINT gate[]);
    void GenerateGate();
    void saveScore();



    SnakeGame(){
        SNAKE_NAME = "SNAKE";
        ROCK = "NHOM16";
    }

    SnakeGame(string name, string rock){
        SNAKE_NAME = name;
        ROCK = rock;
    }
};

void ShowCur(bool CursorVisibility); // Ham xoa con tro chuot
void SetColor(WORD color); //Ham tao mau
void FixConsoleWindow(); // Cố định cửa sổ trò chơi

int main() {
    cin.tie(0); cout.tie(0);
	SnakeGame snakeGame;
	snakeGame.playGame();
}

void FixConsoleWindow() {
	HWND consoleWindow = GetConsoleWindow();
	LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
	style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
	SetWindowLong(consoleWindow, GWL_STYLE, style);
}

void ShowCur(bool CursorVisibility) // Ham xoa con tro chuot
{
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursor = { 1, CursorVisibility };
	SetConsoleCursorInfo(handle, &cursor);
}

void SetColor(WORD color) //Ham tao mau
{
	HANDLE hConsoleOutput;
	hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
	GetConsoleScreenBufferInfo(hConsoleOutput, &screen_buffer_info);

	WORD wAttributes = screen_buffer_info.wAttributes;
	color &= 0x000f;
	wAttributes &= 0xfff0;
	wAttributes |= color;

	SetConsoleTextAttribute(hConsoleOutput, wAttributes);
}

void SnakeGame:: drawRock() {
	for (int i = 0; i < min(level, 5) * 3; i++) {
		GotoXY(rock[i].x, rock[i].y); cout << ROCK;
	}
}

bool SnakeGame::checkRock(int x, int y) {

	for (int i = 0; i < SIZE_SNAKE; i++)
		if (x <= snake[i].x && snake[i].x <= x + ROCK.length() - 1 && snake[i].y == y)
			return 0;

	if (x <= food[FOOD_INDEX].x && food[FOOD_INDEX].x <= x + ROCK.length() - 1 && food[FOOD_INDEX].y == y)
		return 0;

	return 1;
}

void SnakeGame::createRock() {
	int x, y;

	for (int i = 0; i < min(level, 5) * 3; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - ROCK.length()) + 1;
			y = rand() % (HEIGH_CONSOLE - ROCK.length()) + 1;
		} while (checkRock(x, y) == false);

		rock[i] = { x,y };

		GotoXY(x, y); cout << ROCK;
	}
}

void SnakeGame:: drawScore() {

	ifstream inp("ScoreBoard.txt");
	vector<pair<int, string>> s;
	string name;
	int sc;

	GotoXY(WIDTH_CONSOLE + 25, 0); cout << "Top 5";

	if (!inp) return;

	while (inp >> sc >> name)
		s.push_back({ sc,name });

	inp.close();

	sort(s.rbegin(), s.rend());

	for (int i = 0; i < s.size() && i < 5; ++i) {
		GotoXY(WIDTH_CONSOLE + 2, i + 2);
		cout << setw(25) << s[i].second << setw(5) << s[i].first;// << s.size();
	}

}

void SnakeGame:: drawSnake() {
	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		cout << SNAKE_NAME[i % SNAKE_NAME.size()];
	}
}

void SnakeGame:: drawFood() {
	char s = SNAKE_NAME[(SIZE_SNAKE) % SNAKE_NAME.length()];

	GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);

	cout << s;
}

void SnakeGame:: drawGate() {
	char s = 254;
	GotoXY(gate[0].x, gate[0].y);
	cout << s;
	for (int i = 1; i < 4; i++) {
		GotoXY(gate[i].x, gate[i].y);
		cout << "X";
	}
}

void SnakeGame::clearGate() {
	for (int i = 0; i < 4; i++) {
		GotoXY(gate[i].x, gate[i].y);
		cout << " ";
	}
}

void SnakeGame:: effectDead() {

	drawSnake();

	for (int i = 0; i < 10; ++i) {
		for (int i = 0; i < SIZE_SNAKE; ++i) {
			GotoXY(snake[i].x, snake[i].y);
			cout << "-"; Sleep(3);
		}
		for (int i = 0; i < SIZE_SNAKE; ++i) {
			GotoXY(snake[i].x, snake[i].y);
			cout << "\\"; Sleep(3);
		}
		for (int i = 0; i < SIZE_SNAKE; ++i) {
			GotoXY(snake[i].x, snake[i].y);
			cout << "|"; Sleep(3);
		}
		for (int i = 0; i < SIZE_SNAKE; ++i) {
			GotoXY(snake[i].x, snake[i].y);
			cout << "/"; Sleep(3);
		}

	}

	drawSnake();
}

int SnakeGame:: Move(POINT step) {
	step = step + snake[SIZE_SNAKE - 1];

	if (step.x * step.y == 0 || step.x == WIDTH_CONSOLE || step.y == HEIGH_CONSOLE)
		return ProcessDead(), 0;

	for (int i = 0; i < SIZE_SNAKE; i++)
		if (step == snake[i])
			return ProcessDead(), 0;

	for (int i = 0; i < min(level, 5) * 3; i++)
		if (rock[i].x <= step.x && step.x <= rock[i].x + ROCK.length() - 1 && rock[i].y == step.y)
			return ProcessDead(), 0;

	if (food_ate == MAX_SPEED * MAX_SIZE_FOOD)
	{
		for (int i = 1; i < 5; i++)
			if (gate[i] == step)
				return ProcessDead(), 0;
	};

	if (step == food[FOOD_INDEX])
	{
		food_ate++;
		score++;
		snake[SIZE_SNAKE] = food[FOOD_INDEX];
		SIZE_SNAKE++;
		Eat1();
		if (FOOD_INDEX < MAX_SIZE_FOOD - 1)FOOD_INDEX++;
	}
	else
	{
		for (int i = 1; i < SIZE_SNAKE; i++)
			snake[i - 1] = snake[i];

		snake[SIZE_SNAKE - 1] = step;
		GotoXY(0, HEIGH_CONSOLE + 1);
		cout << "Score: " << score << setw(10) << "level: " << level << setw(10) << "Speed: " << SPEED;
		Eat2();
	}
}

void SnakeGame::saveGame() {
	string s;

	GotoXY(0, HEIGH_CONSOLE + 2);

	cout << "===SAVE GAME===           " << '\n';
	cout << "File name: "; cin >> s;

	ofstream out(s + ".GAME");

	out << CHAR_LOCK << " " << MOVING << " " << SPEED << " " << FOOD_INDEX << " " << WIDTH_CONSOLE << " " << HEIGH_CONSOLE << " " << SIZE_SNAKE << " ";

	for (int i = 0; i < SIZE_SNAKE; ++i)
		out << snake[i].x << " " << snake[i].y << " ";

	out << FOOD_INDEX << " ";

	for (int i = 0; i < 4; ++i) out << food[i].x << " " << food[i].y << " ";

	out << STATE << " " << score << " " << level << " " << food_ate << " ";

	for (int i = 0; i < min(level, 5) * 3; ++i)
		out << rock[i].x << " " << rock[i].y << " ";

    for (int i=0; i<4; ++i)
        out << gate[i].x << " " << gate[i].y << " ";
	out.close();
}
int SnakeGame::check() {

	string s;
	cout << "===LOAD GAME===" << '\n';
	cout << "File name: "; cin >> s;

	ifstream inp(s + ".GAME");

	if (!inp) {

		cout << "Tro choi khong co, chung ta bat dau tu dau!\n\n";
		system("pause");
		NewGame();
		return 0;
	}

	inp >> CHAR_LOCK >> MOVING >> SPEED >> FOOD_INDEX >> WIDTH_CONSOLE >> HEIGH_CONSOLE >> SIZE_SNAKE;

	for (int i = 0; i < SIZE_SNAKE; ++i)
		inp >> snake[i].x >> snake[i].y;

	inp >> FOOD_INDEX;

	for (int i = 0; i < 4; ++i) inp >> food[i].x >> food[i].y;

	inp >> STATE >> score >> level >> food_ate;

	for (int i = 0; i < min(level, 5) * 3; ++i)
		inp >> rock[i].x >> rock[i].y;

    for (int i=0; i<4; ++i)
        inp >> gate[i].x >> gate[i].y ;

	inp.close();

	system("cls");

	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE);
	drawRock();
}

void SnakeGame::startGame() { // Bắt đầu chơi
	system("cls");

	int k;
	cout << "\t   ________     ____         ___              ______              ____    _____   ___________" << '\n';
	cout << "\t  /______  \\   /___/\\       /__/|            /_____/\\            /__ /|  /____/  /__________/|" << '\n';
	cout << "\t /  ____ \\ /   |   \\ \\      | | |           /  __  \\ \\           |  | | /  / /   |  ________|/" << '\n';
	cout << "\t|  | | |_|/    | |\\ \\ \\     | | |          /  / /\\  \\ \\          |  | |/  / /    |  | |       " << '\n';
	cout << "\t\\  \\ \\         | | \\ \\ \\    | | |         /  / /  \\  \\ \\         |  | /  / /     |  | |______       " << '\n';
	cout << "\t \\  \\ \\        | | |\\ \\ \\   | | |        /  / /    \\  \\ \\        |  |/  / /      |  |_______/|" << '\n';
	cout << "\t  \\  \\ \\       | | | \\ \\ \\  | | |       /  / /      \\  \\ \\       |  |\\  \\ \\      |  ________|/" << '\n';
	cout << "\t   \\  \\ \\      | | |  \\ \\ \\ | | |      /  /_/________\\  \\ \\      |  | \\  \\ \\     |  | |       " << '\n';
	cout << "\t __ \\  \\ \\     | | |   \\ \\ \\| | |     /  _____________\\  \\ \\     |  | |\\  \\ \\    |  | |       " << '\n';
	cout << "\t/_/| \\  \\ \\    | | |    \\ \\ | | |    /  / /            \\  \\ \\    |  | | \\  \\ \\   |  | |______       " << '\n';
	cout << "\t| |/__|  |/    | | /     \\ \\| | /   /  / /              \\  \\ \\   |  | /  \\  \\ \\  |  |_______/|" << '\n';
	cout << "\t|_______//     |_|/       \\___|/   /__/_/                \\__\\/   |__|/    \\__\\/  |__________|/" << '\n';
	cout << "\n\n";
	cout << "\t\t\t\t\t\t=====MENU=====\n";
	cout << "\t\t\t\t\t\t1. New game\n";
	cout << "\t\t\t\t\t\t2. Open old game\n\n\n";
	cout << "\t\t\t\t\t\t   NHOM 16:\n";
	cout << "\t\t\t\t\t       Pham Thanh Luan \n";
	cout << "\t\t\t\t\t       Nguyen Nhat Linh\n";
	cout << "\t\t\t\t\t       Nguyen Tran Duc Duy\n";
	//cin >> k;

	//if (k == 2) check(); else NewGame();
	cout << "Enter your selection: ";
	while (cin >> k)
	{
		if (k == 1)
		{
			NewGame();
			break;
		}
		else if (k == 2)
		{
			check();
			break;
		}
		else
		{
			cout << "The selection not true!" << '\n';
			cout << "Enter your selection: " << '\n';
		}
	}

	GotoXY(0, HEIGH_CONSOLE + 1);
	cout << "Score: " << score << setw(10) << "level: " << level << setw(10) << "Speed: " << SPEED;

}

void SnakeGame::GotoXY(int x, int y) {
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

bool SnakeGame::IsValid(int x, int y) {

	for (int i = 0; i < SIZE_SNAKE; i++)
		if (snake[i].x == x && snake[i].y == y)
			return false;

	for (int i = 0; i < min(level, 5) * 3; i++)
		if (rock[i].x <= x && x <= rock[i].x + ROCK.length() - 1 && rock[i].y == y)
			return 0;

	return true;
}

bool SnakeGame::IsValid2(POINT gate[]) {

	for (int i = 0; i < SIZE_SNAKE; i++)
		for (int j = 0; j < 5; j++)
			if (snake[i].x == gate[j].x && snake[i].y == gate[j].y)
				return false;

	for (int i = 0; i < min(level, 5) * 3; i++)
		for (int j = 0; j < 5; j++)
			if (rock[i].x <= gate[j].x && gate[j].x <= rock[i].x + ROCK.length() - 1 && rock[i].y == gate[j].y)
				return 0;

	return true;
}

void SnakeGame::GenerateFood() {
	int x, y;

	for (int i = 0; i < MAX_SIZE_FOOD; i++) {
		do {
			x = rand() % (WIDTH_CONSOLE - 4) + 2;
			y = rand() % (HEIGH_CONSOLE - 4) + 2;
		} while (IsValid(x, y) == false);

		food[i] = { x,y };
	}
}

void SnakeGame::GenerateGate() {
	int x, y;
	do {
		x = rand() % (WIDTH_CONSOLE - 9) + 4;
		y = rand() % (HEIGH_CONSOLE - 9) + 4;
		gate[0] = { x,y };
		gate[1] = { x - 2,y };
		gate[2] = { x + 2,y };
		gate[3] = { x,y - 1 };
		//gate[4] = { x,y + 1 };
	} while (IsValid2(gate));
}

void SnakeGame::ResetData() {
	//Khởi tạo các giá trị toàn cục
	CHAR_LOCK = 'A', MOVING = 'D', SPEED = 1; FOOD_INDEX = 0, WIDTH_CONSOLE = 70,
		HEIGH_CONSOLE = 20, SIZE_SNAKE = 6;

	score = 0; level = 0; food_ate = 0;

	//Khởi tạo giá trị mặc định cho snake
	snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
	snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
	snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
	GenerateFood();//Tạo mảng thức ăn food
}

void SnakeGame::NewGame() {
	system("cls");
	ResetData(); // Khởi tạo dữ liệu gốc
	DrawBoard(0, 0, WIDTH_CONSOLE, HEIGH_CONSOLE); // Vẽ màn hình game
	STATE = 1;//Bắt đầu cho Thread chạy
	createRock();
	GenerateGate();
}

void SnakeGame::DrawBoard(int x, int y, int width, int height) {
	int curPosX = 0; int curPosY = 0;
	GotoXY(x, y); cout << char(220);
	for (int i = 1; i < width; i++)cout << char(220);
	cout << char(221);
	GotoXY(x, height + y); cout << char(221);
	for (int i = 1; i < width; i++)cout << char(220);
	cout << char(221);
	for (int i = y + 1; i < height + y; i++) {
		GotoXY(x, i); cout << char(221);
		GotoXY(x + width, i); cout << char(221);
	}

	int i = 15;
	GotoXY(WIDTH_CONSOLE + 25, i); cout << "L:\tSAVE GAME"; i++;
	GotoXY(WIDTH_CONSOLE + 25, i); cout << "P:\tPAUSE GAME"; i++;
	GotoXY(WIDTH_CONSOLE + 25, i); cout << "ESC:\tEXIT GAME"; i++;

	GotoXY(WIDTH_CONSOLE + 2, 15); cout << "MOVE KEY";
	GotoXY(WIDTH_CONSOLE + 5, 16); cout << "W";
	GotoXY(WIDTH_CONSOLE + 4, 17); cout << "A";
	GotoXY(WIDTH_CONSOLE + 6, 17); cout << "D";
	GotoXY(WIDTH_CONSOLE + 5, 18); cout << "S";

	drawScore();
	//GotoXY(curPosX, curPosY);
}

void SnakeGame::ExitGame() {
	system("cls");
	cout << "GOOD BYE!\n\n";
	system("pause");
}

void SnakeGame::PauseGame(){
	GotoXY(0, HEIGH_CONSOLE + 2);
	cout << "Press any key to coninue!";
	while (!_kbhit());
}

//Hàm cập nhật dữ liệu toàn cục
void SnakeGame:: Eat1() {
	if (FOOD_INDEX == MAX_SIZE_FOOD - 1)
	{
		if (SPEED != MAX_SPEED)
		{
			GenerateFood();
			FOOD_INDEX = -1;
			SPEED++;
		};
	}
}

void SnakeGame::throughGate()
{
	while (SIZE_SNAKE > 1)
	{
		for (int i = 1; i < SIZE_SNAKE; i++)
		{
			snake[i - 1] = snake[i];
		}
		snake[SIZE_SNAKE - 1] = { NULL, NULL };
		drawSnake();
		Sleep(400 - 100 * SPEED);
		clearSnake();
		SIZE_SNAKE--;
	}
	snake[SIZE_SNAKE - 1] = { NULL, NULL };
}

void SnakeGame::Eat2()
{
	if (food_ate == MAX_SPEED * MAX_SIZE_FOOD)
	{
		GotoXY(food[FOOD_INDEX].x, food[FOOD_INDEX].y);
		printf(" ");
		drawGate();
		if (snake[SIZE_SNAKE - 1] == gate[0])
		{
			throughGate();
			SIZE_SNAKE = 6;
			snake[0] = { 10, 5 }; snake[1] = { 11, 5 };
			snake[2] = { 12, 5 }; snake[3] = { 13, 5 };
			snake[4] = { 14, 5 }; snake[5] = { 15, 5 };
			clearGate();
			clearRock();
			SPEED = 1;
			FOOD_INDEX = 0;
			level++;
			food_ate = 0;
			GenerateGate();
			createRock();
			GenerateFood();
		}
	}
}

void SnakeGame::saveScore() {

	cout << "What's your name?: ";

	string s; cin >> s;

	ofstream out;
	out.open("ScoreBoard.txt", ios::out | ios::app);

	out << score << " " << s << '\n';

	out.close();

}

void SnakeGame::ProcessDead() {
	effectDead();
	STATE = 0;
	GotoXY(0, HEIGH_CONSOLE + 2);

	int k;
	cout << "DEAD!\n\nDo you want to save score:\n1. YES\n2. NO\n\n";
	cin >> k;

	system("cls");

	if (k == 1) saveScore();

	printf("Type y to new game or anykey to exit");
}

//Hàm vẽ màn hình
void SnakeGame::clearSnake() {

	for (int i = 0; i < SIZE_SNAKE; i++) {
		GotoXY(snake[i].x, snake[i].y);
		printf(" ");
	}
}

void SnakeGame::clearRock() {
    string s(ROCK.length(),' ');
	for (int i = 0; i < min(level, 5) * 3; i++) {
		GotoXY(rock[i].x, rock[i].y); cout <<s;
	}
}

//Thủ tục cho thread
void SnakeGame::ThreadFunc() {
	while (!_kbhit() && STATE == 1) {
		clearSnake();

		if (MOVING != CHAR_LOCK) switch (MOVING) {

		case 'A':   Move(step[0]);
			break;

		case 'D':   Move(step[1]);
			break;

		case 'W':   Move(step[2]);
			break;

		case 'S':   Move(step[3]);
			break;
		}


		if (STATE == 1)
		{
			drawSnake();
			if (food_ate != MAX_SPEED * MAX_SIZE_FOOD)drawFood();
		}

		// Sleep(650 - SPEED*175);//Hàm ngủ theo tốc độ SPEEED
		Sleep(400 - SPEED * 100);
	}
}

int SnakeGame::playGame(){
    srand(time(NULL));
	int temp;
	ShowCur(0);
	FixConsoleWindow();
	SetColor(3);
	startGame();

	while (1) {

		ThreadFunc();

		temp = toupper(_getch());

		if (STATE == 1) {
			if (temp == 'P') {
				PauseGame();
			}
			else if (temp == 27) {
				ExitGame();
				return 0;
			}
			if (temp == 'L') {
				saveGame();
				ExitGame();
				return 0;
			}
			else {
				if ((temp != CHAR_LOCK) && (temp == 'D' || temp == 'A' || temp == 'W' || temp == 'S'))
				{
					if (temp == 'D') CHAR_LOCK = 'A';
					else if (temp == 'W') CHAR_LOCK = 'S';
					else if (temp == 'S') CHAR_LOCK = 'W';
					else CHAR_LOCK = 'D';
					MOVING = temp;
				}
			}
		}
		else {
			if (temp == 'Y') startGame();
			else
				return ExitGame(), 0;
		}
	}
}
