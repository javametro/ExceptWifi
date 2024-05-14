#include <winsock2.h>
#include <ws2tcpip.h> // Needed for InetNtop
#include <iphlpapi.h>
#include <stdio.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    ULONG outBufLen = 0;
    GetIpAddrTable(NULL, &outBufLen, FALSE);
    PMIB_IPADDRTABLE pIpAddrTable = (PMIB_IPADDRTABLE)malloc(outBufLen);
    if (GetIpAddrTable(pIpAddrTable, &outBufLen, FALSE) != NO_ERROR) {
        fprintf(stderr, "GetIpAddrTable failed.\n");
        free(pIpAddrTable);
        WSACleanup();
        return 1;
    }

    if (pIpAddrTable == nullptr) {
        return 1;
    }

    for (DWORD i = 0; i < pIpAddrTable->dwNumEntries; i++) {
        if (pIpAddrTable->table[i].wType == MIB_IPADDR_PRIMARY) {
            wchar_t ipString[INET6_ADDRSTRLEN];
            InetNtop(AF_INET, &pIpAddrTable->table[i].dwAddr, ipString, INET6_ADDRSTRLEN);
            printf("LAN IP Address: %ws\n", ipString);

            struct in_addr addr;
            addr.S_un.S_addr = (u_long)pIpAddrTable->table[i].dwAddr;
            addr.S_un.S_un_b.s_b4 = (addr.S_un.S_un_b.s_b4 & 0xFFFFFF00) | 1; // Start of range (e.g., 192.168.1.1)

            printf("Possible Peer IP Range:\n");
            for (int j = 1; j < 255; j++) {
                addr.S_un.S_un_b.s_b4++;
                InetNtop(AF_INET, &addr, ipString, INET6_ADDRSTRLEN);
                printf("  %ws\n", ipString);
            }

            break;
        }
    }

    // Clean up
    free(pIpAddrTable);
    WSACleanup();
    return 0;
}
