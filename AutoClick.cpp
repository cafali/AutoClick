#include <windows.h>
#include <commctrl.h>
#include <thread>
#include <iostream>
#pragma comment(lib, "comctl32.lib")

// Global variables
HWND hSlider;
HWND hLabel;
HWND hApplyButton;
HWND hQuitButton;
int clickInterval = 4000; // Default click interval in milliseconds
HANDLE threadHandle;
bool shouldClick = false;

// Function to handle WM_CREATE message
void OnCreate(HWND hwnd) {
    // Create a Consolas font
    HFONT hFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Consolas");

    // Create a slider control with Consolas font
    hSlider = CreateWindowExW(0, TRACKBAR_CLASSW, NULL,
        WS_CHILD | WS_VISIBLE | TBS_HORZ,
        10, 10, 200, 30,
        hwnd, (HMENU)1, GetModuleHandleW(NULL), NULL);
    SendMessageW(hSlider, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    // Create a label with Consolas font
    hLabel = CreateWindowExW(0, L"STATIC", L"Click Interval: 4000 ms",
        WS_CHILD | WS_VISIBLE,
        10, 50, 200, 20,
        hwnd, NULL, GetModuleHandleW(NULL), NULL);
    SendMessageW(hLabel, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    // Create an "Apply" button with Consolas font
    hApplyButton = CreateWindowW(L"BUTTON", L"Apply",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 80, 80, 25,
        hwnd, (HMENU)2, GetModuleHandleW(NULL), NULL);
    SendMessageW(hApplyButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    // Create a "Quit" button with Consolas font
    hQuitButton = CreateWindowW(L"BUTTON", L"Quit",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        100, 80, 80, 25,
        hwnd, (HMENU)3, GetModuleHandleW(NULL), NULL);
    SendMessageW(hQuitButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

    // Set the range and default position of the slider
    SendMessageW(hSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 100)); // Range: 0 to 100
    SendMessageW(hSlider, TBM_SETPOS, TRUE, 50); // Default position: 50
}

// Function to handle WM_HSCROLL message (slider movement)
void OnSliderMove(HWND hwnd) {
    // Get the current position of the slider
    int pos = SendMessageW(hSlider, TBM_GETPOS, 0, 0);

    // Calculate the new click interval based on the slider position
    clickInterval = pos * 100; // Adjust as needed

    // Update the label text
    wchar_t labelText[50];
    _snwprintf(labelText, sizeof(labelText) / sizeof(labelText[0]), L"Click Interval: %d ms", clickInterval);
    SetWindowTextW(hLabel, labelText);
}

// Function to handle "Apply" button click
void OnApplyClick(HWND hwnd) {
    // Set the flag to start clicking
    shouldClick = true;

    // Update the label text
    wchar_t labelText[50];
    _snwprintf(labelText, sizeof(labelText) / sizeof(labelText[0]), L"Click Interval: %d ms", clickInterval);
    SetWindowTextW(hLabel, labelText);
}

// Function to handle "Quit" button click
void OnQuitClick(HWND hwnd) {
    // Set the flag to stop clicking
    shouldClick = false;

    // Close the window
    DestroyWindow(hwnd);
}

// Function to handle WM_COMMAND message
void OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
    case 2: // Apply button clicked
        OnApplyClick(hwnd);
        break;

    case 3: // Quit button clicked
        OnQuitClick(hwnd);
        break;
    }
}

// Function to handle WM_CLOSE message
void OnClose(HWND hwnd) {
    DestroyWindow(hwnd);
}

// Function to handle WM_DESTROY message
void OnDestroy() {
    // Terminate the autoclicker thread
    TerminateThread(threadHandle, 0);
    CloseHandle(threadHandle);
    PostQuitMessage(0);
}

// Autoclicker function
DWORD WINAPI AutoClick(LPVOID) {
    while (true) {
        if (shouldClick) {
            // Simulate a mouse click using mouse_event
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

            // Allow the system to handle sleep events
            Sleep(100);

            // Pause for the specified click interval
            Sleep(clickInterval);
        }
        else {
            // If shouldClick is false, sleep for a short duration
            Sleep(100);
        }
    }
    return 0;
}

// Main window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        OnCreate(hwnd);
        break;

    case WM_HSCROLL:
        OnSliderMove(hwnd);
        break;

    case WM_COMMAND:
        OnCommand(hwnd, wParam, lParam);
        break;

    case WM_CLOSE:
        OnClose(hwnd);
        break;

    case WM_DESTROY:
        OnDestroy();
        break;

    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Register the window class
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND); // Use the default background color
    wc.lpszClassName = L"AutoclickerWindowClass";
    RegisterClassW(&wc);

    // Create the main window
    HWND hwnd = CreateWindowExW(0, L"AutoclickerWindowClass", L"AutoClick Settings",
        WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
        100, 100, 240, 150, // Adjusted height to accommodate the Apply and Quit buttons
        NULL, NULL, hInstance, NULL);

    // Show and update the window
    ShowWindow(hwnd, SW_SHOWNORMAL);
    UpdateWindow(hwnd);

    // Run the autoclicker in a separate thread
    threadHandle = CreateThread(NULL, 0, AutoClick, NULL, 0, NULL);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
