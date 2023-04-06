#include "TransparentWindow.h"
#include <iostream>
#include <cmath>

TransparentWindow::TransparentWindow() : window(nullptr), buttonEvent(0), cp_x(0), cp_y(0), offset_cpx(0), offset_cpy(0), w_posx(0), w_posy(0) {}

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
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screenCenterX = (mode->width - 400) / 2;
    int screenCenterY = (mode->height + 450) / 2;
    glfwSetWindowPos(window, screenCenterX, screenCenterY);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos)
    //                          {
    //     TransparentWindow* tw = static_cast<TransparentWindow*>(glfwGetWindowUserPointer(window));
    //     tw->cursor_position_callback(window, xpos, ypos); });

    // glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods)
    //                            {
    //     TransparentWindow* tw = static_cast<TransparentWindow*>(glfwGetWindowUserPointer(window));
    //     tw->mouse_button_callback(window, button, action, mods); });

    glfwSetWindowUserPointer(window, this);

    HWND hWnd = glfwGetWin32Window(window);
    LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    exStyle &= ~WS_EX_LAYERED;
    exStyle |= WS_EX_TRANSPARENT;
    SetWindowLong(hWnd, GWL_EXSTYLE, exStyle);

}

void TransparentWindow::draw()
{
    drawBars();
}

GLFWwindow *TransparentWindow::getWindow() const
{
    return window;
}

void TransparentWindow::cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (buttonEvent)
    {
        offset_cpx = xpos - cp_x;
        offset_cpy = ypos - cp_y;
        glfwGetWindowPos(window, &w_posx, &w_posy);
        glfwSetWindowPos(window, w_posx + offset_cpx, w_posy + offset_cpy);
        offset_cpx = 0;
        offset_cpy = 0;
        cp_x += offset_cpx;
        cp_y += offset_cpy;
    }
}

void TransparentWindow::mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        buttonEvent = 1;
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        cp_x = std::floor(x);
        cp_y = std::floor(y);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        buttonEvent = 0;
        cp_x = 0;
        cp_y = 0;
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

    float upSpeed = 1.0f;    // Speed when the bar height goes up
    float downSpeed = 0.3f;  // Speed when the bar height goes down

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
