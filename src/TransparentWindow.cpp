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

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE); // Enable transparency
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);              // Remove window decorations

    window = glfwCreateWindow(300, 200, "Semi-Transparent Window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos)
                             {
        TransparentWindow* tw = static_cast<TransparentWindow*>(glfwGetWindowUserPointer(window));
        tw->cursor_position_callback(window, xpos, ypos); });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods)
                               {
        TransparentWindow* tw = static_cast<TransparentWindow*>(glfwGetWindowUserPointer(window));
        tw->mouse_button_callback(window, button, action, mods); });

    glfwSetWindowUserPointer(window, this);
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
}

void TransparentWindow::drawBars()
{
    if (barHeights.empty())
    {
        return;
    }

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    float gapWidth = 4.0f; // Width of the gap between the bars
    float barWidth = (static_cast<float>(display_w) - (barHeights.size() - 1) * gapWidth) / barHeights.size();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, display_w, 0, display_h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for (size_t i = 0; i < barHeights.size(); ++i)
    {
        float x = i * (barWidth + gapWidth);
        float y = 0;
        float height = barHeights[i] * display_h;

        glColor4f(0.2f, 0.6f, 0.8f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + barWidth, y);
        glVertex2f(x + barWidth, y + height);
        glVertex2f(x, y + height);
        glEnd();
    }
}
