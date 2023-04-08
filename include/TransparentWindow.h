#pragma once
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vector>
#include <windows.h>
#include <shellapi.h>

#include "SystemTrayMenu.h"

class TransparentWindow
{
public:
    TransparentWindow();
    ~TransparentWindow();
    void createWindow();
    void draw();
    GLFWwindow *getWindow() const;
    void setBarHeights(const std::vector<float> &heights);

private:
    GLFWwindow *window;
    SystemTrayMenu menu;
    int buttonEvent;
    int cursorPosX, cursorPosY;
    int offsetCursorPosX, offsetCursorPosY;
    int windowPosX, windowPosY;
    WNDPROC oldWndProc;
    std::vector<float> prevBarHeights;
    std::vector<float> barHeights;
    bool hasBorder;

    void drawBars();
    void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    void subclassWindow();
    void unsubclassWindow();

    static void cursorPositionCallbackWrapper(GLFWwindow *window, double xpos, double ypos);
    static void mouseButtonCallbackWrapper(GLFWwindow *window, int button, int action, int mods);
    void handleSystemTrayMenuCommand(UINT command);
    static LRESULT CALLBACK customWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    void setBorder(bool border);
};
