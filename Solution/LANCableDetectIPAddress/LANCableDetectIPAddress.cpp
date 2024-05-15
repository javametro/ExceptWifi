#include <winsock2.h>
#include <ws2tcpip.h> // For InetNtop
#include <iphlpapi.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    // Get adapter information
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter) {
            std::cout << "Adapter Name: " << pAdapter->AdapterName << std::endl;
            std::cout << "IP Address: " << pAdapter->IpAddressList.IpAddress.String << std::endl;
            std::cout << std::endl;

            // Get ARP table and display directly connected devices
            ULONG ulArpBufLen = 0;
            GetIpNetTable(NULL, &ulArpBufLen, FALSE);
            PMIB_IPNETTABLE pArpTable = (MIB_IPNETTABLE*)malloc(ulArpBufLen);
            if (GetIpNetTable(pArpTable, &ulArpBufLen, FALSE) == NO_ERROR) {
                for (DWORD i = 0; i < pArpTable->dwNumEntries; i++) {
                    if (pArpTable->table[i].dwPhysAddrLen != 0 &&
                        pArpTable->table[i].dwType == MIB_IPNET_TYPE_DYNAMIC) {

                        // Convert IP address to dotted-decimal format using InetNtop
                        WCHAR ipString[INET6_ADDRSTRLEN]; // Use the maximum possible length
                        InetNtop(AF_INET, &pArpTable->table[i].dwAddr, ipString, INET6_ADDRSTRLEN);
                        // Convert wide character string to narrow character string for output
                            char narrowIpString[INET6_ADDRSTRLEN];
                        WideCharToMultiByte(CP_ACP, 0, ipString, -1, narrowIpString, INET6_ADDRSTRLEN, NULL, NULL);
                        std::string ipStr(narrowIpString);

                        // Filter for LAN cable connected devices (169.254.x.x)
                        if (ipStr.substr(0, 7) == "169.254") {
                            std::cout << "   Directly Connected Device (LAN): " << ipStr << std::endl;
                            break;
                        }
                    }
                }
            }
            free(pArpTable);
            pAdapter = pAdapter->Next;
        }
    }

    free(pAdapterInfo);

    // Cleanup Winsock
    WSACleanup();

    return 0;
}
