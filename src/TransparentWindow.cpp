#include "TransparentWindow.h"
#include <iostream>
#include <cmath>
#include <windows.h>
#include <shellapi.h>

TransparentWindow::TransparentWindow() : window(nullptr),
                                         buttonEvent(0),
                                         cursorPosX(0),
                                         cursorPosY(0),
                                         offsetCursorPosX(0),
                                         offsetCursorPosY(0),
                                         oldWndProc(nullptr),
                                         hasBorder(false),
                                         running(false),
                                         settings("../settings.ini")
{

    windowPosX = settings.getSetting<int>("windowPosX");
    windowPosY = settings.getSetting<int>("windowPosY");
    windowSizeX = settings.getSetting<int>("windowWidth");
    windowSizeY = settings.getSetting<int>("windowHeight");
    numOfBars = settings.getSetting<int>("numBars");
    color_red = settings.getSetting<float>("color_red");
    color_green = settings.getSetting<float>("color_green");
    color_blue = settings.getSetting<float>("color_blue");
    color_alpha = settings.getSetting<float>("color_alpha");
    renderThread = std::thread(&TransparentWindow::run, this);
}

TransparentWindow::~TransparentWindow()
{
    if (running)
    {
        running = false;
        renderThread.join();
        unsubclassWindow();
        menu.uninitialize();
    }
    if (window)
    {
        glfwDestroyWindow(window);
    }
    settings.setSetting("windowPosX", windowPosX);
    settings.setSetting("windowPosY", windowPosY);
    settings.setSetting("windowWidth", windowSizeX);
    settings.setSetting("windowHeight", windowSizeY);
    settings.setSetting("numBars", numOfBars);
    settings.setSetting("color_red", color_red);
    settings.setSetting("color_green", color_green);
    settings.setSetting("color_blue", color_blue);
    settings.setSetting("color_alpha", color_alpha);
    settings.save("../settings.ini");
}

void TransparentWindow::setBarHeights(const std::vector<float> &heights)
{
    barHeights = heights;
    if (prevBarHeights.empty())
    {
        prevBarHeights = barHeights;
    }
}

void TransparentWindow::waitForClose()
{
    renderThread.join();
}

bool TransparentWindow::isRunning() const
{
    return running;
}

void TransparentWindow::waitUntilTransparentWindowIsRunning()
{
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [this]()
            { return this->isRunning(); });
}

GLFWwindow *TransparentWindow::getWindow()
{
    return window;
}

void TransparentWindow::run()
{
    if (running)
    {
        return;
    }
    initialize();
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.notify_one();
    }

    while (running)
    {
        glfwPollEvents();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        glfwSwapBuffers(window);

        if (glfwWindowShouldClose(window))
        {
            running = false;
        }
    }
}

void TransparentWindow::initialize()
{
    glfwSetErrorCallback(errorCallback);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    window = glfwCreateWindow(windowSizeX, windowSizeY, "Semi-Transparent Window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
    }
    glfwSetWindowPos(window, windowPosX, windowPosY);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

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
    running = true;
}

void TransparentWindow::draw()
{
    drawBars();
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

    float upSpeed = 0.7f;
    float downSpeed = 0.1f;

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

        glColor4f(color_red, color_green, color_blue, color_alpha);
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

        // Get current window rect and adjust size and position
        RECT rect;
        GetWindowRect(hWnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Account for the new border size
        width += GetSystemMetrics(SM_CXSIZEFRAME) * 2;
        height += GetSystemMetrics(SM_CYSIZEFRAME) * 2;

        // Account for the position change
        int left = rect.left - GetSystemMetrics(SM_CXSIZEFRAME);
        int top = rect.top - GetSystemMetrics(SM_CYSIZEFRAME);

        SetWindowPos(hWnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);

        hasBorder = true;
    }
    else if (hasBorder)
    {
        LONG style = GetWindowLong(hWnd, GWL_STYLE);
        style &= ~WS_THICKFRAME;
        SetWindowLong(hWnd, GWL_STYLE, style);

        // Get current window rect and adjust size and position
        RECT rect;
        GetWindowRect(hWnd, &rect);
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;

        // Account for the removed border size
        width -= GetSystemMetrics(SM_CXSIZEFRAME) * 2;
        height -= GetSystemMetrics(SM_CYSIZEFRAME) * 2;

        // Account for the position change
        int left = rect.left + GetSystemMetrics(SM_CXSIZEFRAME);
        int top = rect.top + GetSystemMetrics(SM_CYSIZEFRAME);

        SetWindowPos(hWnd, nullptr, left, top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);

        hasBorder = false;
    }
}

void TransparentWindow::errorCallback(int error, const char *description)
{
    std::cerr << "Error: " << description << std::endl;
}
