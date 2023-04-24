// Socket Programming.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
using namespace std;


#define DEFAULT_BUFLEN 512

#define FILEFORMAT ".txt"

void printConnectionStatus(string ip, string port, bool bconnected, bool bsend);
void toUpperCase(string& text);
string getDateTime();
int main()
{
    string ip_address = "192.168.1.106";
    u_short portNumber = 16011;
    // initialize the WSA
    WSADATA wsadata;
    int WSAResult=WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (WSAResult != 0) {
        std::cout << "WSA initialization failed" << endl;
        cin.get();
    }

    //Create Socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cout << "Fail to create socket" << endl;
        cin.get();
        return 0;
    }

    //Set up connection
    try {
        std::cout << "Enter IP address: ";
        getline(cin, ip_address);
        string port;
        std::cout << "Enter Port: ";
        getline(cin, port);
        portNumber = (u_short)stoi(port);
    }
    catch (exception& e) {
        std::cout << "invalid address and port number" << endl;
        return 1;
    }
   
    SOCKADDR_IN address;
    address.sin_family = AF_INET;
    address.sin_port = htons(portNumber);

    if (inet_pton(AF_INET, ip_address.c_str(), &address.sin_addr.s_addr) <= 0) {
        std::cout << "Invalid Address" << endl;
        cin.get();
        return 0;
    };

    int connection = connect(sock, (struct sockaddr*)&address, sizeof(address));
    if (connection == SOCKET_ERROR) {
        std::cout << "Fail to set up connection "<< WSAGetLastError() << endl;
        cin.get();
        return 0;
    }
    std::cout << "connection succeed" << endl;
    string filename = "log"+getDateTime();
    std::cout << string(filename + FILEFORMAT) << endl;
    ofstream resultFile(string(filename+FILEFORMAT));
    string cmd = "";
    char recvBuf[DEFAULT_BUFLEN] = {0};
    int recvBufLen = DEFAULT_BUFLEN;
    int recvResult = 0;
    do {
        cmd = "";
        memset(recvBuf, 0, DEFAULT_BUFLEN);
        std::cout << "Enter command: ";
        getline(cin,cmd);
        toUpperCase(cmd);
        // send cmd
        int sendResult=send(sock, cmd.c_str(), cmd.size(), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cout << "Send Error" << endl;
        }
        // Read Command
        if (cmd.compare("READ") == 0) {
            string recvString = "";
            printConnectionStatus(ip_address, to_string(portNumber), connection != SOCKET_ERROR, sendResult != SOCKET_ERROR);
            do {
                memset(recvBuf, 0, DEFAULT_BUFLEN);
                //receive buffer for READ
                recvResult = recv(sock, recvBuf, recvBufLen, 0);
                if (recvResult < 0) {
                    std::cout << "receive error " << WSAGetLastError() << endl;
                }
                recvString = recvBuf;
                std::cout << recvString << endl;
                resultFile << recvString << endl;
            } while (recvString[recvString.size() - 1] != '#');
            continue;
        }
        //Post Command
        if (cmd.compare("POST") == 0) {
            string text;
            printConnectionStatus(ip_address, to_string(portNumber), connection != SOCKET_ERROR, sendResult != SOCKET_ERROR);
            do {
                std::cout << "Enter string: ";
                getline(cin, text);
                int sendResult = send(sock, text.c_str(), text.size(), 0);
                if (sendResult == SOCKET_ERROR) {
                    std::cout << "Send text Error" << endl;
                }
            } while (text[text.size()-1]!='#');
        }
        //receive buffer for POST and QUIT
        recvResult = recv(sock, recvBuf, recvBufLen, 0);
        if (recvResult < 0) {
            std::cout<<"receive error "<< WSAGetLastError() << endl;
        }
        string recvString= recvBuf;
        std::cout << recvString << endl;

        if (cmd.compare("QUIT")==0) {
            resultFile.close();
            closesocket(sock);
            WSACleanup();
            std::cout << "Disconnect Server" << endl;
        }
    } while (cmd.compare("QUIT")!=0);

}

void printConnectionStatus(string ip, string port, bool bconnected, bool bsend) {
    string connectedStatus = bconnected ? "OK" : "ERROR";
    string sendStatus = bsend ? "OK" : "ERROR";
    std::cout << "IP Address: " << ip << "  " << "Port Number: " << port << endl;
    std::cout << "Connect Status: " << connectedStatus << endl;
    std::cout << "Send Status: " << sendStatus << endl;
}
void toUpperCase(string& text) {
    for_each(text.begin(), text.end(), [](char& c) {
        c = toupper(c);
        }
    );
}

string getDateTime() {
    time_t currentTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
    tm* timeInfo= new tm();
    localtime_s(timeInfo ,&currentTime);
    char buffer[80];
    strftime(buffer,sizeof(buffer), "%Y-%m-%d_%H-%M-%S", timeInfo);
    delete timeInfo;
    return string(buffer);
}

