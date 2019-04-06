// TS_Klient_UDP.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include "pch.h"

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning (disable: 4996)

/* Zmienne globalne */

std::string pakiet;
// Pola pakietu 

std::string poleOperacji;		// "o"
std::string poleLiczb_1;		// "l1" // — 2,147,483,648 do 2,147,483,647
std::string poleLiczb_2;		// "l2" // — 2,147,483,648 do 2,147,483,647
std::string poleStatusu;		// "s"
std::string poleIdentyfikatora; // "i"
//std::string poleInstrukcji;	// "k" jak komenda
std::string poleACK;			// "p" jak potwierdzenie
std::string poleCzasu;			// "t" jak time

const unsigned int MAX_BUF_LENGTH = 4096;
char buf[MAX_BUF_LENGTH];

std::vector<int> wektor_liczb;

void wyczysc_pakiet() {
	ZeroMemory(buf, MAX_BUF_LENGTH);
	poleOperacji.clear();
	poleLiczb_1.clear();
	poleLiczb_2.clear();
	poleStatusu.clear();
	//poleInstrukcji.clear();
	poleACK.clear();
	poleCzasu.clear();
}

void zapakuj_pakiet() {
	pakiet = "o->" + poleOperacji + "#"
		+ "l1->" + poleLiczb_1 + "#"
		+ "l2->" + poleLiczb_2 + "#"
		+ "s->" + poleStatusu + "#"
		+ "i->" + poleIdentyfikatora + "#"
		//+ "k->" + poleInstrukcji + "#"
		+ "p->" + poleACK + "#"
		+ "t->" + poleCzasu + "#";
}

void odpakuj_pakiet() {
	std::string s = buf;
	int found[7];
	found[0] = s.find("o->");
	found[1] = s.find("#l1->");
	found[2] = s.find("#l2->");
	found[3] = s.find("#s->");
	found[4] = s.find("#i->");
	found[5] = s.find("#p->");
	found[6] = s.find("#t->");

	for (int i = found[0] + 3; i < found[1]; i++) {
		poleOperacji += s[i];
	}
	for (int i = found[1] + 5; i < found[2]; i++) {
		poleLiczb_1 += s[i];
	}
	for (int i = found[2] + 5; i < found[3]; i++) {
		poleLiczb_2 += s[i];
	}
	for (int i = found[3] + 4; i < found[4]; i++) {
		poleStatusu += s[i];
	}
	if (poleIdentyfikatora.empty()) {
		for (int i = found[4] + 4; i < found[5]; i++) {
			poleIdentyfikatora += s[i];
		}
	}
	for (int i = found[5] + 4; i < found[6]; i++) {
		poleACK += s[i];
	}
	for (int i = found[6] + 4; i < int(s.size()) - 1; i++) {
		poleCzasu += s[i];
	}
}

void pokaz_pakiet() {
	std::cout << "========== PAKIET ==========" << std::endl;
	std::cout << pakiet << std::endl;
	std::cout << "Pole operacji: " << poleOperacji << std::endl;
	std::cout << "Pole liczby 1: " << poleLiczb_1 << std::endl;
	std::cout << "Pole liczby 2: " << poleLiczb_2 << std::endl;
	std::cout << "Pole numeru ID: " << poleIdentyfikatora << std::endl;
	std::cout << "Pole statusu: " << poleStatusu << std::endl;
	//std::cout << "Pole instrukcji: " << poleInstrukcji << std::endl;
	std::cout << "Pole ACK: " << poleACK << std::endl;
	std::cout << "Pole czasu: " << poleCzasu << std::endl;
	std::cout << "============================" << std::endl;
}

std::string zwroc_czas() {
	time_t czas; // Zmienna do przechowywania obecnego czasu w postaci surowej
	struct tm * data; // Struktura przechowujaca informacje o czasie (dacie)
	char bufor[24]; // Tablica char tymczasowo przechowujaca date

	time(&czas); // Obecny czas
	data = localtime(&czas); // Zamiana z time_t na struct tm *

	strftime(bufor, sizeof(bufor), "%d.%m.%Y %H:%M:%S", data); // Zapisywanie daty w buforze
	std::string znacznikCzasu(bufor);

	return znacznikCzasu;
}

void wyslij_ACK(SOCKET &mainSocket, sockaddr_in &addr) {
	poleCzasu = zwroc_czas();
	poleACK = "P";
	zapakuj_pakiet();
	sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& addr, sizeof(struct sockaddr));
	std::cout << "WYSLANO ACK.\n";
	poleACK.clear(); // z powrotem na ""
}

bool odbierz_ACK(SOCKET &mainSocket, sockaddr_in &addr) {
	int addr_len = sizeof(struct sockaddr);
	wyczysc_pakiet();
	ZeroMemory(buf, MAX_BUF_LENGTH);
	recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& addr, &addr_len);
	//std::cout << "Odebrano recvfrom\n";
	odpakuj_pakiet();
	if (poleACK == "P") {
		std::cout << "ODEBRANO ACK.\n";
		//poleACK.clear(); // z powrotem na ""
		return true;
	}
	else
		return false;
}

void podajLiczbe(int &liczba) {
	std::string line;
	//std::cout << "Podaj wartosc:\n";
	while (std::getline(std::cin, line))
	{
		std::stringstream ss(line);
		if (ss >> liczba)// && czyPoprawne(liczba))
		{
			if (ss.eof())
			{   // Sukces
				break;
			}
		}
		std::cout << "Podano nieprawidlowa wartosc lub przekraczajaca zakres -2,147,483,648 do 2,147,483,647. Sprobuj ponownie.\n";
		//std::cout << "Podaj wartosc:\n";
	}
}

void podaj2Liczby(int &liczba1, int &liczba2) {	
	std::string line;
	std::cout << "Podaj wartosc a:\n";
	while (std::getline(std::cin, line))
	{
		std::stringstream ss(line);
		if (ss >> liczba1)// && czyPoprawne(liczba1))
		{
			if (ss.eof())
			{   // Sukces
				break;
			}
		}
		std::cout << "Podano nieprawidlowa wartosc lub przekraczajaca zakres -2,147,483,648 do 2,147,483,647. Sprobuj ponownie.\n";
		std::cout << "Podaj wartosc a:\n";
	}

	std::cout << "Podaj wartosc b:\n";
	while (std::getline(std::cin, line))
	{
		std::stringstream ss(line);
		if (ss >> liczba2)// && czyPoprawne(liczba2))
		{
			if (ss.eof())
			{   // Sukces
				break;
			}
		}
		std::cout << "Podano nieprawidlowa wartosc lub przekraczajaca zakres -2,147,483,648 do 2,147,483,647. Sprobuj ponownie.\n";
		std::cout << "Podaj wartosc b:\n";
	}
}

/*****************************************************/
// Operacje

void operacja1() {
	poleOperacji.clear(); // ""

	poleOperacji = "a"; // "a" - losowanie liczb z przedzialu (a, b>
	int a, b;
	podaj2Liczby(a, b);

	poleLiczb_1 = std::to_string(a);
	std::cout << poleLiczb_1;
	poleLiczb_2 = std::to_string(b);
	std::cout << poleLiczb_2;

	poleCzasu = zwroc_czas();
	zapakuj_pakiet();
	pokaz_pakiet();
}

void operacja2() {
	poleOperacji.clear(); // ""

	poleOperacji = "A"; // "A" - operacja a modulo b

	int a, b;
	podaj2Liczby(a, b);

	poleLiczb_1 = std::to_string(a);
	poleLiczb_2 = std::to_string(b);

	poleCzasu = zwroc_czas();
	zapakuj_pakiet();
	pokaz_pakiet();
}

void operacja3() {
	poleOperacji.clear(); // ""

	poleOperacji = "d"; // "d" - operacja dodawania

	int a, b;
	podaj2Liczby(a, b);

	poleLiczb_1 = std::to_string(a);
	poleLiczb_2 = std::to_string(b);

	poleCzasu = zwroc_czas();
	zapakuj_pakiet();
	pokaz_pakiet();
}

void operacja4() {
	poleOperacji.clear(); // ""

	poleOperacji = "o"; // "o" - operacja odejmowania

	int a, b;
	podaj2Liczby(a, b);

	poleLiczb_1 = std::to_string(a);
	poleLiczb_2 = std::to_string(b);

	poleCzasu = zwroc_czas();
	zapakuj_pakiet();
	pokaz_pakiet();
}

void operacja5(SOCKET &mainSocket, sockaddr_in &addr) {
	int addr_len = sizeof(struct sockaddr);

	poleOperacji.clear();

	std::string odp;
	std::cout << "Sortowanie:\na) rosnaco\nb) malejaco\n";
	std::string line;
	while (std::getline(std::cin, line))
	{
		std::stringstream ss(line);
		if (ss >> odp && (odp == "a" || odp == "A" || odp == "b" || odp == "B"))
		{
			if (ss.eof())
			{   // Sukces
				break;
			}
		}
		std::cout << "Podano nieprawidlowa wartosc. Sprobuj ponownie.\n";
	}

	int count;
	std::cout << "Ile liczb chcesz przeslac?\n";
	line.clear();
	while (std::getline(std::cin, line))
	{
		std::stringstream ss(line);
		if (ss >> count && count > 0)
		{
			if (ss.eof())
			{   // Sukces
				break;
			}
		}
		std::cout << "Podano nieprawidlowa wartosc lub <=0. Sprobuj ponownie.\n";
	}
	//podajLiczbe(count);

	for (int i = 0; i < count; i++) {
		poleOperacji = "s";

		poleACK.clear();
		int liczba;
		std::cout << "Podaj liczbe:\n";
		podajLiczbe(liczba);

		poleLiczb_1 = std::to_string(liczba);
		if (i == count - 1) {
			if (odp == "a" || odp == "A") {
				poleOperacji = "or"; // Ostatni rosnaco
			}
			if (odp == "b" || odp == "B") {
				poleOperacji = "om"; // Ostatni malejaco
			}
		}
		poleCzasu = zwroc_czas();
		zapakuj_pakiet();
		sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& addr, sizeof(struct sockaddr));
		odbierz_ACK(mainSocket, addr);
		wyczysc_pakiet();
	}

	for (int i = 0; i < count; i++) {
		wyczysc_pakiet();
		recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& addr, &addr_len);
		std::cout << "Odebrano pakiet nr " << i << std::endl;
		odpakuj_pakiet();
		pokaz_pakiet();
		wyslij_ACK(mainSocket, addr);
		wektor_liczb.push_back(std::stoi(poleLiczb_1)); // jak cos to sprobuj atoi, ja dopiero bede jadl :/
	}
	std::cout << "Posortowane liczby:\n";
	std::cout << "[ ";
	for (auto &e : wektor_liczb) {
		std::cout << e << " ";
	}
	std::cout << "]";
}


int main()
{
	// Inicjacja Winsock
	WSADATA wsaData;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR)
		printf("Initialization error.\n");

	// Utworzenie deskryptora gniazda - socket(AF_INET - rodzina adresow IPv4, typ gniazda, protokol)
	SOCKET mainSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mainSocket == INVALID_SOCKET)
	{
		printf("Error creating socket: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Struktura sockaddr_in - reprezentuje adres IP
	sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_port = htons(27016);

	sockaddr_in their_addr;
	memset(&their_addr, 0, sizeof(their_addr)); // Wyzeruj reszte struktury
	their_addr.sin_family = AF_INET; // host byte order
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr.sin_port = htons(27016); // short, network byte order

	int addr_len = sizeof(struct sockaddr);

	// Wysylanie i odbieranie przez klienta

	// Klient wysyla chec nawiazania polaczenia i zadanie podania ID
	std::string odp;
	std::cout << "Czy chcesz polaczyc sie z serwerem? (y/n)\n";

	std::string line;
	while (std::getline(std::cin, line))
	{
		std::stringstream ss(line);
		if (ss >> odp && (odp == "n" || odp == "N" || odp == "y" || odp == "Y"))
		{
			if (ss.eof())
			{   // Sukces
				break;
			}
		}
		std::cout << "Podano nieprawidlowa wartosc. Sprobuj ponownie.\n";
		std::cout << "Czy chcesz polaczyc sie z serwerem? (y/n)\n";
	}

	if (odp == "n" || odp == "N") {
		std::cout << "Wyjscie z programu.\n";
		return 0;
	}
	if (odp == "y" || odp == "Y") {
		wyczysc_pakiet();
		poleCzasu = zwroc_czas();
		poleOperacji = "c";
		zapakuj_pakiet();
		pokaz_pakiet();
		sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
		std::cout << "Wyslano zadanie podania ID.\n";
	}

	// Klient otrzymuje potwierdzenie otrzymania zadania podania ID
	if (odbierz_ACK(mainSocket, their_addr)) {
		pokaz_pakiet();
	}
	else {
		std::cout << "Nie otrzymano ACK.\n";
	}

	wyczysc_pakiet();
	recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
	odpakuj_pakiet();
	std::cout << "Odebrano ID: " << poleIdentyfikatora << std::endl;
	pokaz_pakiet();
	wyslij_ACK(mainSocket, their_addr);

	while (1) {
		// Menu glowne
		std::cout << "\n";
		std::string menu = "Wybierz operacje: \n1. Wylosowanie wartosci z przedzialu (a, b>\n2. Operacja a % b\n3. Operacja a + b\n4. Operacja a - b\n5. Sortowanie wielu liczb\n6. Zakoncz dzialanie programu\n";
		std::cout << menu;

		int ans;
		std::string line;
		while (std::getline(std::cin, line)) {
			std::stringstream ss(line);
			if (ss >> ans) {
				if (ss.eof()) {
					break;
				}
			}
			std::cout << "Wprowadzono niepoprawny numer operacji. Sprobuj ponownie.\n";
			std::cout << "\n";
			std::cout << menu;
		}
		/****************************************************/
		wyczysc_pakiet(); //
		switch (ans) {
		case 1: {
			operacja1();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			odbierz_ACK(mainSocket, their_addr);
			wyczysc_pakiet();
			recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
			odpakuj_pakiet();
			pokaz_pakiet();
			wyslij_ACK(mainSocket, their_addr);
			if (poleOperacji == "a" && poleStatusu == "z") {
				std::cout << "Blad: a > b.\n";
			}
			else if (poleStatusu == "a") {
				std::cout << "Losowa liczba z przedzialu (a, b> wynosi: " << std::stoi(poleLiczb_1);
			}
			break;
		}	
		case 2: {
			operacja2();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			odbierz_ACK(mainSocket, their_addr);
			wyczysc_pakiet();
			recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
			odpakuj_pakiet();
			pokaz_pakiet();
			wyslij_ACK(mainSocket, their_addr);
			if (poleOperacji == "A" && poleStatusu == "z") {
				std::cout << "Blad: b <= 0.\n";
			}
			else if (poleStatusu == "A") {
				std::cout << "a % b = " << stoi(poleLiczb_1);
			}
			break;
		}
		case 3: {
			operacja3();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			odbierz_ACK(mainSocket, their_addr);
			wyczysc_pakiet();
			recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
			odpakuj_pakiet();
			pokaz_pakiet();
			wyslij_ACK(mainSocket, their_addr);
			if (poleOperacji == "d" && poleStatusu == "z") {
				std::cout << "Blad: a + b > 2,147,483,647.\n";
			}
			else if (poleStatusu == "d") {
				std::cout << "a + b = " << stoi(poleLiczb_1);
			}
			break;
		}
		case 4: {
			operacja4();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			odbierz_ACK(mainSocket, their_addr);
			wyczysc_pakiet();
			recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
			odpakuj_pakiet();
			pokaz_pakiet();
			wyslij_ACK(mainSocket, their_addr);
			if (poleOperacji == "o" && poleStatusu == "z") {
				std::cout << "Blad: a - b < -2,147,483,648.\n";
			}
			else if (poleStatusu == "o") {
				std::cout << "a - b = " << stoi(poleLiczb_1);
			}
			break;
		}
		case 5: {
			operacja5(mainSocket, their_addr);
			wektor_liczb.clear();
			break;
		}
		case 6: {
			std::cout << "Konczenie dzialania programu.\n";
			wyczysc_pakiet();

			poleOperacji = "w"; // Zakonczenie polaczenia
			poleCzasu = zwroc_czas();

			zapakuj_pakiet();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& their_addr, sizeof(struct sockaddr));
			odbierz_ACK(mainSocket, their_addr);

			wyczysc_pakiet();
			recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
			odpakuj_pakiet();
			pokaz_pakiet();
			wyslij_ACK(mainSocket, their_addr);

			closesocket(mainSocket);
			WSACleanup();
			system("pause");
			return 0;
			break;
		}
		default: {
			std::cout << "Wprowadzono niepoprawny numer operacji. Sprobuj ponownie.\n";
			break;
		}
		}
	}
}

