#include "SystemTrayMenu.h"
#include <iostream>

SystemTrayMenu::SystemTrayMenu() : nid{} {}

SystemTrayMenu::~SystemTrayMenu()
{
    uninitialize();
}

void SystemTrayMenu::initialize(HWND hWnd)
{
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_APP + 1;
    nid.hIcon = (HICON)LoadImage(NULL, TEXT("icon.ico"), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (!nid.hIcon)
    {
        DWORD error = GetLastError();
        std::cout << "Failed to load icon: " << error << std::endl;
    }
    strcpy(nid.szTip, "Audio Vizualizer");

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void SystemTrayMenu::showContextMenu(HWND hWnd)
{
    HMENU hMenu = CreatePopupMenu();
    if (hMenu)
    {
        AppendMenu(hMenu, MF_STRING, 1, TEXT("Close"));

        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hWnd);
        TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
    }
}

void SystemTrayMenu::uninitialize()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}