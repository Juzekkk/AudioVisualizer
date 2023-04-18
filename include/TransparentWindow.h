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
    TransparentWindow(int windowPosX = 0,
                      int windowPosY = 0,
                      int windowSizeX = 400,
                      int windowSizeY = 200,
                      int numOfBars = 12,
                      int color_red = 1,
                      int color_green = 1,
                      int color_blue = 1,
                      int color_alpha = 1);

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
    WNDPROC oldWndProc;
    std::vector<float> prevBarHeights;
    std::vector<float> barHeights;
    bool hasBorder;
    bool running;
    std::mutex mutex;
    std::condition_variable cv;

    int windowPosX, windowPosY;
    int windowSizeX, windowSizeY;
    int numOfBars;
    int color_red, color_green, color_blue, color_alpha;

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
