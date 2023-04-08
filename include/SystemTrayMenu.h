#pragma once
#include <windows.h>
#include <shellapi.h>

class SystemTrayMenu
{
public:
    SystemTrayMenu();
    ~SystemTrayMenu();
    void initialize(HWND hWnd);
    void showContextMenu(HWND hWnd);
    void uninitialize();
    bool isModifyEnabled() const;
    void toggleModifyEnabled();

private:
    NOTIFYICONDATA nid;
    bool modifyEnabled;
};
