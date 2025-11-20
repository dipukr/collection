#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <string>

struct Object {
    String name;
    float radius;  // relative units
    float r, g, b; // color
};

void DrawCircle(float cx, float cy, float radius, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    int segments = 200;
    for (int i = 0; i <= segments; i++) {
        float a = (float)i / segments * 2.0f * 3.14159265f;
        float x = cx + cosf(a) * radius;
        float y = cy + sinf(a) * radius;
        glVertex2f(x, y);
    }
    glEnd();
}

int main() {
    glfwInit();
    
    GLFWwindow* win = glfwCreateWindow(800, 600, "Cosmic Size Comparison", NULL, NULL);
    if (!win) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    Array<Object> objects = {
        {"Earth",     0.15f, 0.3f, 0.6f, 1.0f},
        {"Jupiter",   0.40f, 1.0f, 0.7f, 0.3f},
        {"Sun",       1.20f, 1.0f, 0.9f, 0.3f},
        {"Red Giant", 2.50f, 1.0f, 0.3f, 0.1f}
    };

    int current = 0;
    float scale = 0.1f;
    double last = glfwGetTime();

    while (!glfwWindowShouldClose(win)) {
        glfwPollEvents();

        double now = glfwGetTime();
        float dt = float(now - last);
        last = now;

        scale += dt * 0.25f;
        if (scale >= 1.0f) {
            scale = 0.1f;
            current = (current + 1) % objects.size();
        }

        Object& o = objects[current];

        glViewport(0, 0, 800, 600);
        glClearColor(0.05f, 0.05f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Simple orthographic space: -2..2 horizontally, -1.5..1.5 vertically
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-2.0, 2.0, -1.5, 1.5, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        DrawCircle(0.0f, 0.0f, o.radius * scale, o.r, o.g, o.b);

        glfwSwapBuffers(win);
    }

    glfwDestroyWindow(win);
    glfwTerminate();
}
