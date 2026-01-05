#include "Application.h"
#include <iostream>
#include <GLFW/glfw3.h>
int main() {
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }
    if (!gladLoaderLoadVulkan(NULL, NULL, NULL)) {
        return EXIT_FAILURE;
    }
    try
    {
        Application app;
        app.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwTerminate();
    return EXIT_SUCCESS;
}