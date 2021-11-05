#include <iostream>
#include <sstream>
#include <thread>

#include <stdio.h>
#include <winsock2.h>

using namespace std;



// consts
const short LINES[][3] = {
    {0, 1, 2}, {3, 4, 5}, {6, 7, 8},
    {0, 3, 6}, {1, 4, 7}, {2, 5, 8},
    {0, 4, 8}, {2, 4, 6}
};
const char SYMBOLS[] = {' ', 'O', 'X'};
const string INPUTS[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

const int NET_PORT = 40332;
const short NET_BUFFERLEN = 256;



short gameMode = 0; // 0 - z komputerem, 1 - jako serwer, 2 - jako klient
string serverAddress = "";

struct NET_RECVDATA {
    string address;
    unsigned short port;
    char data[NET_BUFFERLEN];
};



// functions
void sleep(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}



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



string promptForInput(string message) {
    string input = "";
    cout << message;
    cin >> input;

    return input;
}



short promptForChoice(const string choices[], short n_choices) {
    short select = -1;
    while (select == -1) {
        for (short i = 0; i < n_choices; i++) {
            cout << (i + 1) << ") " << choices[i] << endl;
        }
        cout << "Wybierz: ";
        string input;
        cin >> input;
        short inn;
        try {
            inn = stoi(input);
        } catch (const exception& e) {
            inn = -1;
        }
        if (inn > 0 && inn <= n_choices) {
            for (short i = 1; i <= n_choices; i++) {
                if (inn == i) {
                    select = i;
                    break;
                }
            }
        }
        if (select == -1) {
            cout << "Nieprawidlowy wybor! Wpisz liczbe od 1 do " << n_choices << "." << endl;
        }
    }

    return select;
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



void recvPrint(NET_RECVDATA data) {
    cout << "[RECV] (" << data.address << ":" << data.port << ") -> " << data.data << endl;
}



// classes
class Networking {
    private:
        bool isServer;

        SOCKET s;
        // sother wypelnia sie "danymi kontaktowymi" miejsca skad przyszla ostatnia ramka
        // jesli nic, to send zakonczy sie bledem!
        struct sockaddr_in sother;
        int slen = sizeof(sother);
    


    public:
        void start(bool isServer, string address = "127.0.0.1") {
            // otwieramy bramke SMS
            WSADATA wsaData;

            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != NO_ERROR) {
                cout << "Network error 1" << endl;
            }

            s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (s == INVALID_SOCKET) {
                cout << "Socket error 1" << endl;
                //WSACleanup();
            }

            if (isServer) {
                struct sockaddr_in server;
                server.sin_family = AF_INET;
                server.sin_addr.s_addr = INADDR_ANY;
                server.sin_port = htons(NET_PORT);

                //serwer
                if (bind(s, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR) {
                    cout << "Bind error 1" << endl;
                    closesocket(s);
                }
            } else {
                //setup address structure
                memset((char*)&sother, 0, sizeof(sother));
                sother.sin_family = AF_INET;
                sother.sin_port = htons(NET_PORT);
                sother.sin_addr.S_un.S_addr = inet_addr(address.c_str());
            }

            this->isServer = isServer;
        }



        void stop() {
            // zamykamy bramke SMS
            closesocket(s);
            WSACleanup();
        }



        void send(const char buffer[]) {
            string address = inet_ntoa(sother.sin_addr);
            unsigned short port = ntohs(sother.sin_port);

            int send_len = sendto(s, buffer, strlen(buffer), 0, (SOCKADDR*)&sother, slen);
            if (send_len == SOCKET_ERROR) {
                cout << "[SEND] (" << address << ":" << port << ") Error - didn't send anything! (EC:" << WSAGetLastError() << ")" << endl;
            } else {
                cout << "[SEND] (" << address << ":" << port << ") <- " << buffer << endl;
            }
        }



        NET_RECVDATA recv() {
            char buffer[NET_BUFFERLEN];
            memset(buffer, '\0', NET_BUFFERLEN);
            int recv_len = recvfrom(s, buffer, NET_BUFFERLEN, 0, (SOCKADDR*)&sother, &slen);
            if (recv_len == SOCKET_ERROR) {
                // normalnie tutaj powinnismy wejsc tylko gdy z zewnatrz zamkniemy socket
                // wtedy nie ma gdzie wyslac i wysyla error
				cout << "[RECV] Error - networking terminated! (EC:" << WSAGetLastError() << ")" << endl;
                exit(EXIT_FAILURE);
            }

            NET_RECVDATA result;
            result.address = inet_ntoa(sother.sin_addr);
            result.port = ntohs(sother.sin_port);
            for (short i = 0; i < NET_BUFFERLEN; i++) {
                result.data[i] = buffer[i];
            }
            return result;
        }
};

Networking netw = Networking();



class Board {
    private:
        short tiles[9];



    public:
        void setSymbol(int tile, short value) {
            tiles[tile] = value;
        }

        short getSymbol(int tile) {
            return tiles[tile];
        }

        void reset() {
            for (int i = 0; i < 9; i++) {
                tiles[i] = 0;
            }
        }

        bool isFilled() {
            for (int i = 0; i < 9; i++) {
                if (tiles[i] == 0) {
                    return false;
                }
            }
            return true;
        }

        short getWinner() {
            for (int p = 1; p <= 2; p++) {
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 3; j++) {
                        if (tiles[LINES[i][j]] != p) {
                            break;
                        } else if (j == 2) {
                            return p;
                        }
                    }
                }
            }
            return 0;
        }



        void print() {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    char symbol = SYMBOLS[tiles[i * 3 + j]];
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
};



class Game {
    private:
        // game variables
        Board board = Board();
        short turn = 1; // 1-gracz albo 2-komputer
        short winner = 0; // -1: remis, 0: gra trwa, 1: gracz, 2: komputer
        bool aiPlaced = false;

        int playerScore = 0;
        int opponentScore = 0;
        string playerName = "Ty";
        string opponentName = "Komputer";



    public:
        Board* getBoard() {
            return &board;
        }

        short getTurn() {
            return turn;
        }

        void nextTurn() {
            turn = turn == 1 ? 2 : 1;
        }

        bool isOver() {
            return winner != 0;
        }

        void aiMarkPlacement() {
            aiPlaced = true;
        }



        void reset() {
            board.reset();
            turn = gameMode < 2 ? 1 : 2;
            winner = 0;
        }

        void resetScores() {
            playerScore = 0;
            opponentScore = 0;
        }

        string getPlayerName() {
            return playerName;
        }

        void setPlayerName(string name) {
            playerName = name;
        }

        string getOpponentName() {
            return opponentName;
        }

        void setOpponentName(string name) {
            opponentName = name;
        }



        void print() {
            cout << " --- KOLKO I KRZYZYK --- " << endl;
            cout << endl;

            cout << "<<< " << playerName << " " << playerScore << " - " << opponentScore << " " << opponentName << " >>>" << endl;
            cout << endl;

            board.print();
            cout << endl;

            if (turn == 1) {
                cout << "Twoj ruch!" << endl;
                short select = 0;
                do {
                    select = promptForTile();
                    if (board.getSymbol(select) != 0) {
                        cout << "To pole jest juz wypelnione!" << endl;
                    }
                } while (board.getSymbol(select) != 0);
                board.setSymbol(select, turn);

                if (gameMode > 0) {
                    stringstream ss;
                    ss << "sets|" << select;
                    netw.send(ss.str().c_str());
                }
            }
            else {
                if (gameMode == 0) {
                    cout << "Czekaj na ruch komputera..." << endl;
                } else {
                    cout << "Czekaj na ruch przeciwnika..." << endl;
                }
                // czekamy az AI/przeciwnik polozy symbol
                while (!aiPlaced) {
                    sleep(500);
                }
                // jak polozyl, to gasimy flage i idziemy dalej
                aiPlaced = false;
            }

            // to co ponizej potem wciagniemy pod yourTurn

            short win = board.getWinner();
            bool fill = board.isFilled();

            // jezeli wygralismy, to ... wygralismy
            if (win > 0) {
                winner = win;
                if (win == 1)
                    playerScore++;
                else
                    opponentScore++;
                return;
            }
            // jesli plansza zapelniona, to remis
            if (fill) {
                winner = -1;
                return;
            }
            // w przeciwnym razie, nastepny gracz
            nextTurn();
        }



        void loop() {
            // rob kroki gry dopoki nie bedzie wyloniony zwyciezca
            while (winner == 0) {
                print();
                cout << endl;
            }

            board.print();
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
};



class AI {
    private:
        Game* game;



    public:
        AI(Game* gamePtr) {
            this->game = gamePtr;
        }



        short getLineDanger(int l) {
            // zagrozenie liczymy dla komputera
            // wzor: liczba symboli gracza - liczba symboli komputera
            // n=2 oznacza ze gracz moze wygrac!
            // n=-2 oznacza ze komputer moze wygrac!

            short x = 0;

            for (int i = 0; i < 3; i++) {
                short n = game->getBoard()->getSymbol(LINES[l][i]);
                switch(n) {
                    case 1:
                        x++;
                        break;
                    case 2:
                        x--;
                }
            }

            return x;
        }



        short getMoveRandom() {
            while (true) {
                short rnd = (rand()) % 9;
                if (game->getBoard()->getSymbol(rnd) != 0) {
                    continue;
                }
                return rnd;
            }
        }



        short getMoveIntelligent() {
            // w pierwszej kolejnosci, sprawdzamy czy komputer moze wygrac
            for (int i = 0; i < 8; i++) {
                if (getLineDanger(i) == -2) {
                    // wstawiamy symbol w wolne miejsce i wygrywamy!
                    for (int j = 0; j < 3; j++) {
                        short x = LINES[i][j];
                        if (game->getBoard()->getSymbol(x) == 0) {
                            return x;
                        }
                    }
                }
            }

            // teraz nie dopuszczamy zeby przeciwnik (czyli my) wygral
            for (int i = 0; i < 8; i++) {
                if (getLineDanger(i) == 2) {
                    // linia jest zagrozona - wstawiamy symbol w wolne miejsce
                    for (int j = 0; j < 3; j++) {
                        short x = LINES[i][j];
                        if (game->getBoard()->getSymbol(x) == 0) {
                            return x;
                        }
                    }
                }
            }

            // w przeciwnym wypadku, postaw symbol w losowym miejscu
            return getMoveRandom();
        }



        void place() {
            short x = -1;
            do {
                x = getMoveIntelligent();
            } while (x == -1);
            game->getBoard()->setSymbol(x, game->getTurn());
            game->aiMarkPlacement();
        }



        void loop() {
            // funkcja konczy sie gdy nastapi zwyciezca
            while (!game->isOver()) {
                if (game->getTurn() == 2) {
                    place();
                }

                // co sekunde nastepuje decyzja
                sleep(1000);
            }
        }
};



class Opponent {
    private:
        Game* game;
        bool joined = false;



    public:
        Opponent(Game* gamePtr) {
            this->game = gamePtr;
        }



        void markJoined() {
            joined = true;
        }

        bool hasJoined() {
            return joined;
        }



        void place(short x) {
            if (!game->isOver() && game->getTurn() == 2) {
                game->getBoard()->setSymbol(x, game->getTurn());
                game->aiMarkPlacement();
            } else {
                cout << "Error: opponent wanted to place a symbol during your turn!" << endl;
            }
        }
};



// objects
Game game = Game();
Opponent opp = Opponent(&game);



/////////////////
//// AI ZONE ////
/////////////////



void aiMain() {
    if (gameMode > 0)
        return;
    
    // generator za kazdym razem zaczyna z tym samym stanem na watek, wiec robimy cos takiego
    srand(time(NULL));

    AI ai = AI(&game);
    ai.loop();
}



////////////////////////
//// END OF AI ZONE ////
////////////////////////



/////////////////////////
//// NETWORKING ZONE ////
/////////////////////////




void netMain() {
    if (gameMode == 0)
        return;
    
    bool isServer = gameMode == 1;
    netw.start(isServer, serverAddress);
    if (!isServer) {
        // trzeba cos wyslac, inaczej recv zwroci blad
        netw.send("hello");
    }

    while (true) {
        // petla przerwie sie w momencie wystapienia bledu, np. gdy zamkniemy polaczenie
        NET_RECVDATA data = netw.recv();
        string msg = data.data;

        // interpretujemy rozne komunikaty
        if (msg.compare("hello") == 0 && isServer) {
            // serwer: gdy dostaniemy hello (sygnal ze podlaczyl sie klient)
            opp.markJoined();
            netw.send("hosthello");
            stringstream ss;
            ss << "oppname|" << game.getPlayerName();
            netw.send(ss.str().c_str());
        }
        else if (msg.compare("hosthello") == 0 && !isServer) {
            // klient: gdy serwer odbierze hello i zwraca potwierdzenie
            opp.markJoined();
            stringstream ss;
            ss << "oppname|" << game.getPlayerName();
            netw.send(ss.str().c_str());
        }
        else if (msg.compare(0, 8, "oppname|") == 0) {
            // informacja o nazwie przeciwnika
            game.setOpponentName(msg.substr(8));
        }
        else if (msg.compare(0, 5, "sets|") == 0) {
            // informacja o polozeniu symbolu na planszy
            short x = stoi(msg.substr(5));
            opp.place(x);
        }

        recvPrint(data);
    }
}



////////////////////////////////
//// END OF NETWORKING ZONE ////
////////////////////////////////



int main() {
    cout << "Witaj w Kolko i Krzyzyk!" << endl;
    cout << "Wybierz jedno z ponizszych aby rozpoczac gre!" << endl;
    string choices[] = {"Gram z komputerem", "Gram z inna osoba"};
    short choice = promptForChoice(choices, 2);
    if (choice == 2) {
        string choices[] = {"Hostuj gre", "Dolacz do gry"};
        short choice = promptForChoice(choices, 2);
        gameMode = choice;
        if (choice == 2) {
            string input = promptForInput("Wpisz IP serwera (lub X = localhost): ");
            if (input.compare("x") == 0 || input.compare("X") == 0)
                input = "127.0.0.1";
            serverAddress = input;
        }
        string input = promptForInput("Podaj swoja nazwe: ");
        game.setPlayerName(input);
    }

    cout << "Your game mode is: " << gameMode << endl;

    thread net(netMain);
    if (gameMode > 0 && !opp.hasJoined()) {
        cout << "Oczekiwanie na drugiego gracza..." << endl;
        // czekamy az drugi gracz dojdzie
        while (!opp.hasJoined()) {
            sleep(500);
        }
    }

    bool again = true;
    while (again) {
        game.reset();
        thread ai(aiMain);
        game.loop();
        ai.join();

        again = askYesOrNo("Czy chcesz zagrac jeszcze raz?");
    }

    netw.stop();
    net.join();



    return 0;
}
