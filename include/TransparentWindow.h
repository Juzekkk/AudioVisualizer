#pragma once
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vector>
#include <windows.h>
#include <shellapi.h>

class TransparentWindow
{
public:
    TransparentWindow();
    ~TransparentWindow();
    void createWindow();
    void draw();
    GLFWwindow *getWindow() const;

    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    void setBarHeights(const std::vector<float> &heights);
    void showContextMenu(HWND hWnd);
    void subclassWindow();
    void unsubclassWindow();
    static LRESULT CustomWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    NOTIFYICONDATA nid;

private:
    GLFWwindow *window;
    int buttonEvent;
    int cp_x, cp_y;
    int offset_cpx, offset_cpy;
    int w_posx, w_posy;
    WNDPROC oldWndProc;
    std::vector<float> prevBarHeights;

    std::vector<float> barHeights;
    void drawBars();
};
