#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    std::cout << "Initializing GLFW..." << std::endl;
    
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    std::cout << "GLFW initialized successfully" << std::endl;
    
    GLFWwindow* window = glfwCreateWindow(640, 480, "Test", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    std::cout << "Window created successfully" << std::endl;
    
    glfwMakeContextCurrent(window);
    
    std::cout << "Entering main loop..." << std::endl;
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
    }
    
    std::cout << "Exiting..." << std::endl;
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
