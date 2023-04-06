#pragma once
#include <GLFW/glfw3.h>
#include <vector>

class TransparentWindow
{
public:
    TransparentWindow();
    void createWindow();
    void draw();
    GLFWwindow *getWindow() const;

    void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    void setBarHeights(const std::vector<float> &heights);

private:
    GLFWwindow *window;
    int buttonEvent;
    int cp_x, cp_y;
    int offset_cpx, offset_cpy;
    int w_posx, w_posy;

    std::vector<float> barHeights;
    void drawBars();
};