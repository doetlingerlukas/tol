#include <iostream>
#include "string.h"

// GLAD
#include <glad/glad.h>

// GLFW
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// This example is taken from https://learnopengl.com/
// https://learnopengl.com/code_viewer.php?code=getting-started/hellowindow2
// The code originally used GLEW, I replaced it with Glad

// Compile:
// g++ example/c++/hellowindow2.cpp -Ibuild/include build/src/glad.c -lglfw -ldl

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT = 600;

using namespace std;

// The MAIN function, from here we start the application and run the game loop
int main(int argc, char **argv) {
    std::cout << "Starting GLFW context, OpenGL 4.1" << std::endl;

    // Init GLFW
    glfwInit();

    if(glfwVulkanSupported() == GLFW_TRUE) {
      cout << "Vulkan supported." << endl;
    } else {
      cout << "Vulkan not supported" << endl;
    }

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window;

    if (argc > 1 && strcmp(argv[1], "--full") == 0) {
      const GLFWvidmode* screen = glfwGetVideoMode(glfwGetPrimaryMonitor());
      window = glfwCreateWindow(screen->width, screen->height, "tol", NULL, NULL);
    } else {
      window = glfwCreateWindow(WIDTH, HEIGHT, "tol", NULL, NULL);
    }

    // Create a GLFWwindow object that we can use for GLFW's functions
    glfwMakeContextCurrent(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Set the required callback functions
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // Define the viewport dimensions
    glViewport(0, 0, WIDTH, HEIGHT);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();

        // Render
        // Clear the colorbuffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminates GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
