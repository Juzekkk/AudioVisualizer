#pragma once
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "SystemTrayMenu.h"

class TransparentWindow
{
public:
    TransparentWindow();
    ~TransparentWindow();
    void setBarHeights(const std::vector<float> &heights);
    void waitForClose();
    bool isRunning() const;
    void waitUntilTransparentWindowIsRunning();
    GLFWwindow *getWindow();

private:
    void run();
    void initialize();
    static void errorCallback(int error, const char *description);
    GLFWwindow *window;
    std::thread renderThread;

    SystemTrayMenu menu;
    int buttonEvent;
    int cursorPosX, cursorPosY;
    int offsetCursorPosX, offsetCursorPosY;
    int windowPosX, windowPosY;
    WNDPROC oldWndProc;
    std::vector<float> prevBarHeights;
    std::vector<float> barHeights;
    bool hasBorder;
    bool running;
    std::mutex mutex;
    std::condition_variable cv;

    void draw();
    void drawBars();
    void setBorder(bool border);
    void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    void subclassWindow();
    void unsubclassWindow();

    void handleSystemTrayMenuCommand(UINT command);
    static void cursorPositionCallbackWrapper(GLFWwindow *window, double xpos, double ypos);
    static void mouseButtonCallbackWrapper(GLFWwindow *window, int button, int action, int mods);
    static LRESULT CALLBACK customWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
