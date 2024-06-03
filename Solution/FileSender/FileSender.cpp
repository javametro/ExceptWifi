#include <winsock2.h>
#include <ws2tcpip.h> // For InetPton
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    // ... (Winsock initialization same as server)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, L"169.254.190.171", &serverAddr.sin_addr.s_addr); // Replace with server's IP
    serverAddr.sin_port = htons(PORT);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    //file path
    std::string filePath = "E:\\testtesttest\\ExceptWifi\\Solution\\LANCableDetectIPAddress\\LANCableDetectIPAddress.cpp";
    std::ifstream infile(filePath, std::ios::binary);

    if (!infile) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    // Send filename
    send(clientSocket, filePath.c_str(), filePath.length() + 1, 0); // +1 for null-terminator

    // Send file data
    char buffer[BUFFER_SIZE];
    while (infile.read(buffer, BUFFER_SIZE)) {
        send(clientSocket, buffer, infile.gcount(), 0);
    }

    infile.close();
    std::cout << "File sent successfully!" << std::endl;

    // ... (Close sockets same as server)
     // Close sockets
    closesocket(clientSocket);
    WSACleanup();
}
