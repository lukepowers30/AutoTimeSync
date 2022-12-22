#include <WinSock2.h>
#include <WS2tcpip.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <minwinbase.h>
#include <Windows.h>
#include <regex>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")

SYSTEMTIME parseResponse(char*);
bool verifyOK(char* recvBuffer);


int main(int argc, char **argv) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
    WSADATA wsaData;
    int iResult;
    INT iRetval;
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cout << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }




	SOCKET mySocket;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo *result;

    // 2a09:8280:1::3:e
    //std::cout << getaddrinfo("2a09:8280:1::3:e", "http", &hints, &result);

    getaddrinfo("2a09:8280:1::3:e", "http", &hints, &result);
    char addrstr[100];
    inet_ntop(result->ai_family, result->ai_addr->sa_data, addrstr, 100);
    auto *ptr = (struct sockaddr_in6*)result->ai_addr;
    inet_ntop(AF_INET6, ptr, addrstr, 100);

    char message[] = "GET /api/timezone/Etc/UTC.txt HTTP/1.0\r\n\r\n";
    char receiveBuffer[2000];
    int numSent = 0;
    int numRecv = 0;
    SYSTEMTIME newTime;



    struct sockaddr_in6 tempAddr;
    int len = sizeof(tempAddr);

    while (true) {
        
        mySocket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        
        
        int connectResponse = connect(mySocket, (sockaddr*)ptr, sizeof(struct sockaddr_in6));
        std::cout << "Connect Response Code: " << connectResponse << std::endl;
        getsockname(mySocket, (sockaddr*)&tempAddr, &len);
        std::cout << "Using source port: " << ntohs(tempAddr.sin6_port) << std::endl;
        
        // Used sizeof() - 1 to remove the null terminator
        numSent = send(mySocket, message, sizeof(message)-1, 0);
        numRecv = recv(mySocket, receiveBuffer, 2000, 0);
        std::regex r("HTTP/1\.0 200 OK\r\n");
        if ((!std::regex_search(receiveBuffer, r)) || (numRecv == -1)) {
            continue;
        }
        newTime = parseResponse(receiveBuffer);
        int setError = SetSystemTime(&newTime);
        closesocket(mySocket);
        std::this_thread::sleep_for(std::chrono::seconds(300));
    }
	return 0;
}


SYSTEMTIME parseResponse(char* buffer) {
    SYSTEMTIME currentTime;

    std::string stringBuffer = std::string(buffer);
    std::string dayOfWeek = stringBuffer.substr(stringBuffer.find("day_of_week: ") + 13, stringBuffer.find("\r\n", stringBuffer.find("day_of_week: ")) - stringBuffer.find("day_of_week: "));
    currentTime.wDayOfWeek = std::stoi(dayOfWeek);
    std::string year = stringBuffer.substr(stringBuffer.find("datetime: ") + 10, 4);
    currentTime.wYear = std::stoi(year);
    std::string day = stringBuffer.substr(stringBuffer.find("datetime: ") + 18, 2);
    currentTime.wDay = std::stoi(day);
    std::string month = stringBuffer.substr(stringBuffer.find("datetime: ") + 15, 2);
    currentTime.wMonth = std::stoi(month);
    std::string hour = stringBuffer.substr(stringBuffer.find("datetime: ") + 21, 2);
    currentTime.wHour = std::stoi(hour);
    std::string minute = stringBuffer.substr(stringBuffer.find("datetime: ") + 24, 2);
    currentTime.wMinute = std::stoi(minute);
    std::string second = stringBuffer.substr(stringBuffer.find("datetime: ") + 27, 2);
    currentTime.wSecond = std::stoi(second);
    std::string millisecond = stringBuffer.substr(stringBuffer.find("datetime: ") + 30, 3);
    currentTime.wMilliseconds = std::stoi(millisecond);
    std::cout << "Year: " << year << "\nMonth: " << month << "\nDay: " << day << "\nHour: " << hour << "\nMinute: " << minute << "\nSecond: " << second << std::endl;
    return currentTime;
}

bool verifyOK(const char* recvBuffer) {
    return std::regex_match(recvBuffer, std::regex("(^HTTP/1.0 200 OK\r\n)*"));
}

