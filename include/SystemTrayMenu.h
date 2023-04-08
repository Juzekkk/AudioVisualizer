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
    bool isDragEnabled() const;
    void toggleDragEnabled();

private:
    NOTIFYICONDATA nid;
    bool dragEnabled;
};
