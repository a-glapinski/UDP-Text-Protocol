// TS_Serwer_UDP.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
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
#include <random>
#include <vector>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning (disable: 4996)

// Zmienne globalne
std::string pakiet;

// Pola pakietu

std::string poleOperacji;		// "o"
std::string poleLiczb_1;		// "l1"
std::string poleLiczb_2;		// "l2"
std::string poleStatusu;		// "s"
std::string poleIdentyfikatora;         // "i"
//std::string poleInstrukcji;	        // "k" 
std::string poleACK;			// "p" 
std::string poleCzasu;			// "t" 

const unsigned int MAX_BUF_LENGTH = 4096;
char buf[MAX_BUF_LENGTH];
// Wykorzystywany przy sortowaniu liczb
std::vector<int> wektor_liczb;

// Funkcje
void wyczysc_pakiet() {
	ZeroMemory(buf, 4096);
	poleOperacji.clear();
	poleLiczb_1.clear();
	poleLiczb_2.clear();
	poleStatusu.clear();
	//poleInstrukcji.clear();
	poleACK.clear();
	poleCzasu.clear();
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


void zapakuj_pakiet() {
	pakiet = "o->" + poleOperacji + "#"
		+ "l1->" + poleLiczb_1 + "#"
		+ "l2->" + poleLiczb_2 + "#"
		+ "s->" + poleStatusu + "#"
		+ "i->" + poleIdentyfikatora + "#"
		+ "p->" + poleACK + "#"
		+ "t->" + poleCzasu + "#";
}

void odpakuj_pakiet() 
{
	/*std::string s;
	for (auto e : pakiet)
	{
		s += e;
	}*/
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
			/*if (std::stoi(poleIdentyfikatora) >= 100 && std::stoi(poleIdentyfikatora) <= 999)
				continue;
			else*/
			poleIdentyfikatora += s[i];
		}
	} 
	for (int i = found[5] + 4; i < found[6]; i++) {
		poleACK += s[i];
	}
	for (int i = found[6] + 4; i < int(s.size()) - 1; i++) {
		poleCzasu += s[i];
	}
	//std::cout << found[6] << std::endl << s.size() - 1 << std::endl;

}

void pokaz_pakiet() {
	std::cout << pakiet << std::endl;
	std::cout << "========== PAKIET ==========" << std::endl;
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


void blad(SOCKET &mainSocket, sockaddr_in &addr) { //czyszczenie pol + ustawienie pola statusu w przypadku blednych danych
	poleStatusu = "z";
	poleLiczb_1.clear();
	poleLiczb_2.clear();
	poleCzasu.clear();
	poleCzasu = zwroc_czas();
	zapakuj_pakiet();
	sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)& addr, sizeof(struct sockaddr));
	wyczysc_pakiet();
}

/*****************************************************/
int main()
{
	srand(time(NULL));
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
	memset(&my_addr, 0, sizeof(my_addr)); // Wyzeruj reszte struktury
	my_addr.sin_family = AF_INET; // host byte order
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_port = htons(27016); // short, network byte order

	sockaddr_in their_addr;
	memset(&their_addr, 0, sizeof(their_addr)); // Wyzeruj reszte struktury	their_addr.sin_family = AF_INET; // host byte order
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	their_addr.sin_port = htons(27016); // short, network byte order

	int addr_len = sizeof(struct sockaddr);

	// Przypisanie gniazda do adresu
	if (bind(mainSocket, (SOCKADDR *)& my_addr, sizeof(my_addr)) == SOCKET_ERROR)
	{
		printf("bind() failed.\n");
		closesocket(mainSocket);
		return 1;
	}
	printf("Oczekiwanie na polaczenie z klientem...\n");
	
	while (1)
	{
		ZeroMemory(buf, MAX_BUF_LENGTH);
		recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
		std::cout << "Odebrano pakiet" << std::endl;
		odpakuj_pakiet();
		std::cout << "Rozpakowano pakiet" << std::endl;
		pokaz_pakiet();
		if (poleACK.empty()) //wyslanie potwierdzenia ACK
		{
			poleACK = "P";
			poleCzasu.clear();
			poleCzasu = zwroc_czas();
			zapakuj_pakiet();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *) &their_addr, sizeof(struct sockaddr));
			std::cout << "Wyslano potwierdzenie ACK" << std::endl;
			poleACK.clear();
		}
		else
		{
			poleACK.clear();
			wyczysc_pakiet();
			std::cout << "Odebrano potwierdzenie ACK" << std::endl;
		}

		if (poleOperacji == "c") //chec nawiazania polaczenia od klienta + przydzielenie ID
		{
			poleStatusu = poleOperacji;
			poleOperacji.clear();
			poleIdentyfikatora = std::to_string(((std::rand() % 900) + 100));
			poleCzasu.clear();
			poleCzasu = zwroc_czas();
			zapakuj_pakiet();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
			std::cout << "Wyslano pakiet z przydzielonym ID" << std::endl;
			wyczysc_pakiet();
		}
		else if (poleOperacji == "a") //random
		{
			if (std::atoi(poleLiczb_1.c_str()) >= std::atoi(poleLiczb_2.c_str()))
			{
				blad(mainSocket, their_addr);
			}
			else
			{
				int wylosowana = (std::rand() % (std::stoi(poleLiczb_2) - (std::stoi(poleLiczb_1))) + (std::stoi(poleLiczb_1) + 1));
				poleLiczb_1.clear();
				poleLiczb_2.clear();
				poleLiczb_1 = std::to_string(wylosowana);
				poleStatusu = poleOperacji;
				poleOperacji.clear();
				poleCzasu.clear();
				poleCzasu = zwroc_czas();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
				std::cout << "Wyslano pakiet" << std::endl;
				pokaz_pakiet();
				wyczysc_pakiet();
			}
		}
		else if (poleOperacji == "A") //modulo
		{
			std::cout << std::atoi(poleLiczb_2.c_str()) << std::endl;
			if (std::atoi(poleLiczb_2.c_str()) <= 0)
			{
				blad(mainSocket, their_addr);
			}
			else {
				int modulo = std::stoi(poleLiczb_1) % std::stoi(poleLiczb_2);
				poleLiczb_1.clear();
				poleLiczb_2.clear();
				poleLiczb_1 = std::to_string(modulo);
				poleStatusu = poleOperacji;
				poleOperacji.clear();
				poleCzasu.clear();
				poleCzasu = zwroc_czas();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
				std::cout << "Wyslano pakiet" << std::endl;
				pokaz_pakiet();
				wyczysc_pakiet();

			}
		} 
		else if (poleOperacji == "d") //dodawanie
		{
			long long suma = std::stoll(poleLiczb_1) + std::stoll(poleLiczb_2);
			if (suma > INT_MAX)
				{
				blad(mainSocket, their_addr);
				}
			else
			{
				poleLiczb_1.clear();
				poleLiczb_2.clear();
				poleLiczb_1 = std::to_string(suma);
				poleStatusu = poleOperacji;
				poleOperacji.clear();
				poleCzasu.clear();
				poleCzasu = zwroc_czas();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
				std::cout << "Wyslano pakiet" << std::endl;
				pokaz_pakiet();
				wyczysc_pakiet();
			}
		}
		else if (poleOperacji == "o") //odejmowanie
		{
			long long roznica = std::stoll(poleLiczb_1) - std::stoll(poleLiczb_2);
			if (roznica < INT_MIN)
			{
				blad(mainSocket, their_addr);
			}
			else
			{
				poleLiczb_1.clear();
				poleLiczb_2.clear();
				poleLiczb_1 = std::to_string(roznica);
				poleStatusu = poleOperacji;
				poleOperacji.clear();
				poleCzasu.clear();
				poleCzasu = zwroc_czas();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
				std::cout << "Wyslano pakiet" << std::endl;
				pokaz_pakiet();
				wyczysc_pakiet();
			}
		}
		else if (poleOperacji == "s") { //sortowanie rosnaco/malejaco
			int liczba = std::stoi(poleLiczb_1);
			wektor_liczb.push_back(liczba);
			wyczysc_pakiet();
		}
		else if (poleOperacji == "or") //jest ostatnim pakietem (sort rosnaco)
		{
			wektor_liczb.push_back(std::stoi(poleLiczb_1));
			wyczysc_pakiet();
			std::sort(wektor_liczb.begin(), wektor_liczb.end());
			for (auto &e : wektor_liczb)
			{
				poleLiczb_1 = std::to_string(e);
				poleStatusu = "sr";
				poleOperacji.clear();
				poleCzasu.clear();
				poleCzasu = zwroc_czas();
				zapakuj_pakiet();
				sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
			}
			wektor_liczb.clear();

		}
		else if (poleOperacji == "om") //jest ostatnim pakietem (sort malejaco)
		{
		wektor_liczb.push_back(std::stoi(poleLiczb_1));
		wyczysc_pakiet();
		std::sort(wektor_liczb.begin(), wektor_liczb.end(), std::greater <int>());
		for (auto &e : wektor_liczb)
		{
			poleLiczb_1 = std::to_string(e);
			poleStatusu = "sm";
			poleOperacji.clear();
			poleCzasu.clear();
			poleCzasu = zwroc_czas();
			zapakuj_pakiet();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
		}
		wektor_liczb.clear();
		}
		else if (poleOperacji == "w") //zakonczenie polaczenia
		{
			poleStatusu = poleOperacji;
			poleOperacji.clear();
			poleCzasu.clear();
			poleCzasu = zwroc_czas();
			zapakuj_pakiet();
			sendto(mainSocket, pakiet.c_str(), pakiet.length(), 0, (SOCKADDR *)&their_addr, sizeof(struct sockaddr));
			wyczysc_pakiet();
			recvfrom(mainSocket, buf, MAX_BUF_LENGTH, 0, (SOCKADDR *)& their_addr, &addr_len);
			odpakuj_pakiet();
			if (poleACK == "P") {
				std::cout << "Odebrano ACK" << std::endl;
				std::cout << "Zakonczenie polaczenia" << std::endl;
				closesocket(mainSocket);
				WSACleanup();
				system("pause");
				return 0;
			}
		}
	}

}
