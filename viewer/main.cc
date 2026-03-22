#include <iostream>
#include <memory>
#include "core/point2d.h"
#include "../src/utils/geometry_utils.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "scene_manager.h"
#include "geometry_canvas.h"

using namespace geometry;

// Application state
struct AppState {
    SceneManager scene_manager;
    std::unique_ptr<GeometryCanvas> canvas;
    bool show_demo_window = false;
    float clear_color[4] = {0.1f, 0.1f, 0.15f, 1.0f};
    
    AppState() {
        canvas = std::make_unique<GeometryCanvas>(&scene_manager);
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
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "BeyondConvex - Geometry Viewer", NULL, NULL);
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
                if (ImGui::MenuItem("Reset Scene")) {
                    app_state.scene_manager.ResetCurrentScene();
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
            if (ImGui::BeginMenu("Scenes")) {
                const auto& scenes = app_state.scene_manager.GetScenes();
                int current_index = app_state.scene_manager.GetCurrentSceneIndex();
                for (size_t i = 0; i < scenes.size(); ++i) {
                    bool is_selected = (static_cast<int>(i) == current_index);
                    if (ImGui::MenuItem(scenes[i]->Name().c_str(), NULL, is_selected)) {
                        app_state.scene_manager.SetCurrentScene(static_cast<int>(i));
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
        
        // Scene selection window (left side, fixed position)
        ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_Always);
        ImGui::Begin("Scene Control", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        const auto& scenes = app_state.scene_manager.GetScenes();
        int current_index = app_state.scene_manager.GetCurrentSceneIndex();
        
        ImGui::Text("Select Scene:");
        for (size_t i = 0; i < scenes.size(); ++i) {
            bool is_selected = (static_cast<int>(i) == current_index);
            if (ImGui::Selectable(scenes[i]->Name().c_str(), is_selected)) {
                app_state.scene_manager.SetCurrentScene(static_cast<int>(i));
            }
        }
        
        ImGui::Separator();
        if (ImGui::Button("Reset Scene")) {
            app_state.scene_manager.ResetCurrentScene();
        }
        ImGui::End();
        
        // Scene info window (left side, below control, fixed position)
        ImGui::SetNextWindowPos(ImVec2(0, 320), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 740), ImGuiCond_Always);
        if (auto* scene = app_state.scene_manager.GetCurrentScene()) {
            ImGui::Begin("Scene Info", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            
            // Show description with word wrap
            ImGui::TextWrapped("%s", scene->Description().c_str());
            ImGui::Separator();
            
            // Show scene-specific UI
            scene->RenderUI();
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
