#include "TransparentWindow.h"
#include <iostream>
#include <cmath>

TransparentWindow::TransparentWindow() : window(nullptr), buttonEvent(0), cursorPosX(0), cursorPosY(0), offsetCursorPosX(0), offsetCursorPosY(0), windowPosX(0), windowPosY(0), oldWndProc(nullptr), hasBorder(false) {}

TransparentWindow::~TransparentWindow()
{
    unsubclassWindow();
    glfwDestroyWindow(window);
    menu.uninitialize();
    glfwTerminate();
}

void TransparentWindow::createWindow()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    window = glfwCreateWindow(400, 200, "Semi-Transparent Window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    // Center the window on the screen
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screenCenterX = (mode->width - 400) / 2;
    int screenCenterY = (mode->height + 450) / 2;
    glfwSetWindowPos(window, screenCenterX, screenCenterY);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetCursorPosCallback(window, TransparentWindow::cursorPositionCallbackWrapper);
    glfwSetMouseButtonCallback(window, TransparentWindow::mouseButtonCallbackWrapper);

    glfwSetWindowUserPointer(window, this);

    HWND hWnd = glfwGetWin32Window(window);
    LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    exStyle &= ~WS_EX_LAYERED;
    exStyle |= WS_EX_TRANSPARENT;
    exStyle &= ~WS_EX_APPWINDOW;
    exStyle |= WS_EX_TOOLWINDOW;
    SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);
    menu.initialize(hWnd);

    ShowWindow(hWnd, SW_SHOW);
    subclassWindow();
}

void TransparentWindow::draw()
{
    drawBars();
}

GLFWwindow *TransparentWindow::getWindow() const
{
    return window;
}

void TransparentWindow::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (menu.isModifyEnabled())
    {
        if (buttonEvent)
        {
            offsetCursorPosX = xpos - cursorPosX;
            offsetCursorPosY = ypos - cursorPosY;
            glfwGetWindowPos(window, &windowPosX, &windowPosY);
            glfwSetWindowPos(window, windowPosX + offsetCursorPosX, windowPosY + offsetCursorPosY);
            offsetCursorPosX = 0;
            offsetCursorPosY = 0;
            cursorPosX += offsetCursorPosX;
            cursorPosY += offsetCursorPosY;
        }
    }
}

void TransparentWindow::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (menu.isModifyEnabled())
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            buttonEvent = 1;
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            cursorPosX = std::floor(x);
            cursorPosY = std::floor(y);
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            buttonEvent = 0;
            cursorPosX = 0;
            cursorPosY = 0;
        }
    }
}

void TransparentWindow::setBarHeights(const std::vector<float> &heights)
{
    barHeights = heights;

    if (prevBarHeights.empty())
    {
        prevBarHeights = barHeights;
    }
}

void TransparentWindow::drawBars()
{
    if (barHeights.empty())
    {
        return;
    }

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    float gapWidth = 25.0f;
    float totalGapWidth = (barHeights.size() + 1) * gapWidth;
    float barWidth = (static_cast<float>(display_w) - totalGapWidth) / barHeights.size();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, display_w, 0, display_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float upSpeed = 1.0f;   // Speed when the bar height goes up
    float downSpeed = 0.3f; // Speed when the bar height goes down

    for (size_t i = 0; i < barHeights.size(); ++i)
    {
        float targetHeight = barHeights[i];
        float currentHeight = prevBarHeights[i];
        float heightDiff = targetHeight - currentHeight;

        if (heightDiff > 0)
        {
            currentHeight += heightDiff * upSpeed;
        }
        else
        {
            currentHeight += heightDiff * downSpeed;
        }

        float x = (i + 1) * gapWidth + i * barWidth;
        float y = display_h / 2.0f;
        float height = currentHeight * display_h / 2.0f;

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + barWidth, y);
        glVertex2f(x + barWidth, y + height);
        glVertex2f(x, y + height);
        glEnd();

        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + barWidth, y);
        glVertex2f(x + barWidth, y - height);
        glVertex2f(x, y - height);
        glEnd();

        prevBarHeights[i] = currentHeight;
    }
}

void TransparentWindow::subclassWindow()
{
    HWND hWnd = glfwGetWin32Window(window);
    oldWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&TransparentWindow::customWindowProc)));
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
}

void TransparentWindow::unsubclassWindow()
{
    HWND hWnd = glfwGetWin32Window(window);
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
}

void TransparentWindow::cursorPositionCallbackWrapper(GLFWwindow *window, double xpos, double ypos)
{
    TransparentWindow *tw = static_cast<TransparentWindow *>(glfwGetWindowUserPointer(window));
    tw->cursorPositionCallback(window, xpos, ypos);
}

void TransparentWindow::mouseButtonCallbackWrapper(GLFWwindow *window, int button, int action, int mods)
{
    TransparentWindow *tw = static_cast<TransparentWindow *>(glfwGetWindowUserPointer(window));
    tw->mouseButtonCallback(window, button, action, mods);
}

void TransparentWindow::handleSystemTrayMenuCommand(UINT command)
{
    switch (command)
    {
    case 1:
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        break;
    case 2:
        menu.toggleModifyEnabled();
        if (hasBorder)
            setBorder(false);
        else
            setBorder(true);
        break;
    }
}

LRESULT CALLBACK TransparentWindow::customWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TransparentWindow *pThis = reinterpret_cast<TransparentWindow *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    switch (uMsg)
    {
    case WM_APP + 1:
        if (lParam == WM_RBUTTONDOWN || lParam == WM_CONTEXTMENU)
        {
            pThis->menu.showContextMenu(hWnd);
        }
        break;
    case WM_COMMAND:
        pThis->handleSystemTrayMenuCommand(LOWORD(wParam));
        break;
    case WM_DESTROY:
        pThis->unsubclassWindow();
        break;
    }

    return CallWindowProc(pThis->oldWndProc, hWnd, uMsg, wParam, lParam);
}

void TransparentWindow::setBorder(bool border)
{
    HWND hWnd = glfwGetWin32Window(window);

    if (border && !hasBorder)
    {
        LONG style = GetWindowLong(hWnd, GWL_STYLE);
        style |= WS_THICKFRAME;
        SetWindowLong(hWnd, GWL_STYLE, style);
        hasBorder = true;
    }
    else if (hasBorder)
    {
        LONG style = GetWindowLong(hWnd, GWL_STYLE);
        style &= ~WS_THICKFRAME;
        SetWindowLong(hWnd, GWL_STYLE, style);
        hasBorder = false;
    }
}