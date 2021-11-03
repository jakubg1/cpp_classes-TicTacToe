#include <iostream>
#include <thread>
//#include <string>

using namespace std;



// consts
const short LINES[][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};
const char SYMBOLS[] = {' ', 'O', 'X'};
const string INPUTS[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

// game variables
short board[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
short turn = 1; // 1-gracz albo 2-komputer
short winner = 0;
bool aiPlaced = false;



void pause(bool message = true) {
    if (message) {
        cout << "Nacisnij dowolny klawisz, aby kontynuowac..." << endl;
    }
    system("pause >nul");
}



bool askYesOrNo(string message) {
    cout << message << " (Y/N)" << endl;
    char choice;
    do {
        cin >> choice;
    } while (choice != 'n' && choice != 'y');

    return choice == 'y';
}



short promptForTile() {
    short select = -1;
    while (select == -1) {
        string input;
        cin >> input;
        for (int i = 0; i < 9; i++) {
            if (input == INPUTS[i]) {
                select = i;
                break;
            }
        }
        if (select == -1) {
            cout << "Nieprawidlowy ruch! Wpisz liczbe od 1 do 9." << endl;
        }
    }

    return select;
}



bool checkWin() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            if (board[LINES[i][j]] != turn) {
                break;
            } else if (j == 2) {
                return true;
            }
        }
    }
    return false;
}



bool checkFill() {
    for (int i = 0; i < 9; i++) {
        if (board[i] == 0) {
            return false;
        }
    }
    return true;
}



void printBoard() {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            char symbol = SYMBOLS[board[i * 3 + j]];
            cout << " " << symbol << " ";
            if (j < 2) {
                cout << "|";
            }
        }
        cout << endl;
        if (i < 2) {
            cout << "-----------" << endl;
        }
    }
}



void resetGame() {
    for (int i = 0; i < 9; i++) {
        board[i] = 0;
    }
    turn = 1; // 1-gracz albo 2-komputer
    winner = 0;
}



void printGame() {
    cout << " --- KOLKO I KRZYZYK --- " << endl;
    cout << endl;

    printBoard();
    cout << endl;

    if (turn == 1) {
        cout << "Twoj ruch!" << endl;
        short select = 0;
        do {
            select = promptForTile();
            if (board[select] != 0) {
                cout << "To pole jest juz wypelnione!" << endl;
            }
        } while (board[select] != 0);
        board[select] = turn;
    } else {
        cout << "Czekaj na ruch komputera..." << endl;
        // czekamy az AI polozy symbol
        while (!aiPlaced) {
            _sleep(500);
        }
        // jak polozyl, to gasimy flage i idziemy dalej
        aiPlaced = false;
    }

    // to co ponizej potem wciagniemy pod yourTurn

    bool win = checkWin();
    bool fill = checkFill();

    // jezeli wygralismy, to ... wygralismy
    if (win) {
        winner = turn;
        return;
    }
    // jesli plansza zapelniona, to remis
    if (fill) {
        winner = -1;
        return;
    }
    // w przeciwnym razie, nastepny gracz
    if (turn == 1) {
        turn = 2;
    } else {
        turn = 1;
    }
}



void game() {
    // inicjalizacja
    resetGame();

    // rob kroki gry dopoki nie bedzie wyloniony zwyciezca
    while (winner == 0) {
        printGame();
        cout << endl;
    }

    printBoard();
    cout << endl;

    // gdy juz mamy zwyciezce, wypisz go i pochwal
    switch (winner) {
        case 1:
            cout << "Wygrales!!!" << endl;
            break;
        case 2:
            cout << "Przegrales!!!" << endl;
            break;
        case -1:
            cout << "Remis!" << endl;
    }
}



/////////////////
//// AI ZONE ////
/////////////////



int aiGetLineDanger(int l) {
    // zagrozenie liczymy dla komputera
    // jesli komputer ma cokolwiek na danej linii postawione, wynosi 0
    // jesli my postawilismy n symboli, to mamy n
    // n=2 oznacza ze gracz moze wygrac!

    for (int i = 0; i < 3; i++) {
        short n = board[LINES[l][i]];
        if (n == 2) {
            return 0;
        }
        // todo
    }
}



void aiPlaceRandom() {
    while (!aiPlaced) {
        short rnd = rand() % 9;
        if (board[rnd] != 0) {
            continue;
        }
        board[rnd] = turn;
        aiPlaced = true;
    }
}



void aiPlaceIntelligent() {
    while (!aiPlaced) {
        // w pierwszej kolejnosci, nie dopuszczamy zeby przeciwnik (czyli my) wygral
        for (int i = 0; i < 8; i++) {

        }


        short rnd = rand() % 9;
        if (board[rnd] != 0) {
            continue;
        }
        board[rnd] = turn;
        aiPlaced = true;
    }
}



void aiMain() {
    // funkcja konczy sie gdy nastapi zwyciezca
    while (winner == 0) {
        if (turn == 2) {
            aiPlaceRandom();
        }

        // co sekunde nastepuje decyzja
        _sleep(1000);
    }
}



////////////////////////
//// END OF AI ZONE ////
////////////////////////



int main() {
    srand(time(NULL));
    //string text = "Hello world!";
    //cout << text;

    cout << "Witaj w Kolko i Krzyzyk!" << endl;
    cout << "Nacisnij dowolny klawisz aby rozpoczac gre!" << endl;
    pause(false);

    bool again = true;
    while (again) {
        thread ai(aiMain);
        game();
        ai.join();

        again = askYesOrNo("Czy chcesz zagrac jeszcze raz?");
    }

    return 0;
}
