#include <iostream>
#include <memory>
#include "core/point.h"
#include "core/geometry_utils.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "demo_manager.h"
#include "geometry_canvas.h"

using namespace geometry;

// Application state
struct AppState {
    DemoManager demo_manager;
    std::unique_ptr<GeometryCanvas> canvas;
    bool show_demo_window = false;
    float clear_color[4] = {0.1f, 0.1f, 0.15f, 1.0f};
    
    AppState() {
        canvas = std::make_unique<GeometryCanvas>(&demo_manager);
        canvas->SetBackgroundColor(clear_color[0], clear_color[1], 
                                   clear_color[2], clear_color[3]);
    }
};

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "BeyondConvex - Geometry Viewer", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    // Our application state
    AppState app_state;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Start Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Main menu bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Reset Demo")) {
                    app_state.demo_manager.ResetCurrentDemo();
                }
                if (ImGui::MenuItem("Exit")) {
                    glfwSetWindowShouldClose(window, true);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Demo Window", NULL, &app_state.show_demo_window);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Demos")) {
                const auto& demos = app_state.demo_manager.GetDemos();
                int current_index = app_state.demo_manager.GetCurrentDemoIndex();
                for (size_t i = 0; i < demos.size(); ++i) {
                    bool is_selected = (static_cast<int>(i) == current_index);
                    if (ImGui::MenuItem(demos[i]->Name().c_str(), NULL, is_selected)) {
                        app_state.demo_manager.SetCurrentDemo(static_cast<int>(i));
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
        
        // Show demo window
        if (app_state.show_demo_window) {
            ImGui::ShowDemoWindow(&app_state.show_demo_window);
        }
        
        // Demo selection window (left side, fixed position)
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Always);
        ImGui::Begin("Demo Control", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        const auto& demos = app_state.demo_manager.GetDemos();
        int current_index = app_state.demo_manager.GetCurrentDemoIndex();
        
        ImGui::Text("Select Demo:");
        for (size_t i = 0; i < demos.size(); ++i) {
            bool is_selected = (static_cast<int>(i) == current_index);
            if (ImGui::Selectable(demos[i]->Name().c_str(), is_selected)) {
                app_state.demo_manager.SetCurrentDemo(static_cast<int>(i));
            }
        }
        
        ImGui::Separator();
        if (ImGui::Button("Reset Demo")) {
            app_state.demo_manager.ResetCurrentDemo();
        }
        ImGui::End();
        
        // Demo info window (left side, below control, fixed position)
        ImGui::SetNextWindowPos(ImVec2(0, 320), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Always);
        if (auto* demo = app_state.demo_manager.GetCurrentDemo()) {
            ImGui::Begin("Demo Info", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            
            // Show description with word wrap
            ImGui::TextWrapped("%s", demo->Description().c_str());
            ImGui::Separator();
            
            // Show demo-specific UI
            demo->RenderUI();
            ImGui::End();
        }
        
        // Draw geometry canvas (right side, full height)
        ImGui::SetNextWindowPos(ImVec2(310, 20), ImGuiCond_FirstUseEver);
        app_state.canvas->Render();
        
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(
            app_state.clear_color[0] * 0.5f,
            app_state.clear_color[1] * 0.5f,
            app_state.clear_color[2] * 0.5f,
            app_state.clear_color[3] * 0.5f
        );
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
