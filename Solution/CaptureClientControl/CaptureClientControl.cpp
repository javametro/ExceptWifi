// CaptureClientControl.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
using boost::asio::ip::tcp;
namespace asio = boost::asio;

tcp::socket* globalSocket = nullptr;

HHOOK mouseHook = NULL;
HHOOK keyboardHook = NULL;

// Note: You'll need to pass the socket reference to this function
void SendKeyboardData(int keyCode) {
    if (globalSocket) {
        std::string message = "Key: " + std::to_string(keyCode) + "\n";
        asio::async_write(*globalSocket, asio::buffer(message),
            [](const boost::system::error_code& error, std::size_t bytes_transferred) {
                if (!error) {
                    std::cout << bytes_transferred << " bytes transferred\n";
                }
                else {
                    std::cerr << "Error sending data: " << error.message() << "\n";
                }
            });
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        if (wParam == WM_KEYDOWN) {
            // Send the p->vkCode to the remote endpoint
            SendKeyboardData(p->vkCode);
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        // Process mouse event here and send to remote endpoint
        std::cout << "Mouse event captured." << std::endl;
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

void SetKeyboardHook() {
    keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (keyboardHook == NULL) {
        std::cerr << "Failed to install keyboard hook!" << std::endl;
    }
}

void SetMouseHook() {
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    if (mouseHook == NULL) {
        std::cerr << "Failed to install mouse hook!" << std::endl;
    }
}

int main() {
    try {
        asio::io_context io_context;

        // Server endpoint - adjust address and port as necessary
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "12345");
        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        globalSocket = &socket;
        // Set hooks for keyboard and mouse events, passing the socket for use in callbacks
        SetKeyboardHook();
        SetMouseHook();

        // Run the io_context to perform asynchronous operations
        io_context.run();

        // Message loop to keep the application running and processing system messages
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // Cleanup: Unhook and close socket
        UnhookWindowsHookEx(keyboardHook);
        UnhookWindowsHookEx(mouseHook);

        // Close the socket
        socket.close();

        globalSocket = nullptr;

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

// Example modification for KeyboardProc to send data





// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
