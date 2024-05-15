#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <fstream> // For file operations
#include <string>
#include <filesystem> // for std::filesystem::path

#pragma comment (lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    listen(serverSocket, 3); // Allow up to 3 pending connections
    std::cout << "Server listening on port " << PORT << std::endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed!" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Receive filename (potentially with path)
    char filenameBuffer[256];
    recv(clientSocket, filenameBuffer, sizeof(filenameBuffer), 0);
    // Extract base filename using std::filesystem
    std::filesystem::path filePath(filenameBuffer);
    std::string filename = filePath.filename().string(); // Get only the filename

    std::cout << "Receiving file: " << filename << std::endl;

    // Get current directory
    std::filesystem::path currentPath = std::filesystem::current_path();
    std::string fullPath = (currentPath / filename).string();


    // Receive file data and save to current directory
    std::ofstream outfile(fullPath, std::ios::binary);
    char buffer[BUFFER_SIZE];
    int bytesReceived = 1;  // Initialize to a non-zero value
    while (bytesReceived > 0) {  // Loop until no more data is received
        bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) { // Only write if data was received
            outfile.write(buffer, bytesReceived);
        }
    }
    outfile.close();
    std::cout << "File received successfully!" << std::endl;

    // Close sockets
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
