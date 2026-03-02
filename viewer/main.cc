#include <iostream>
#include "core/point.h"
#include "core/geometry_utils.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <vector>

using namespace geometry;

// Application state
struct AppState {
    std::vector<Point> points;
    bool show_demo_window = false;
    float clear_color[4] = {0.1f, 0.1f, 0.15f, 1.0f};
};

// Draw geometry on ImGui canvas
void DrawGeometryCanvas(AppState& state) {
    ImGui::Begin("Geometry Canvas");
    
    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
    ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(
        (int)(state.clear_color[0] * 255),
        (int)(state.clear_color[1] * 255),
        (int)(state.clear_color[2] * 255),
        (int)(state.clear_color[3] * 255)
    ));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));
    
    // Handle mouse input
    if (ImGui::IsMouseHoveringRect(canvas_p0, canvas_p1)) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            double x = mouse_pos.x - canvas_p0.x;
            double y = mouse_pos.y - canvas_p0.y;
            state.points.push_back(Point(x, y));
        }
    }
    
    // Draw points
    for (const auto& pt : state.points) {
        ImVec2 pos(canvas_p0.x + pt.X(), canvas_p0.y + pt.Y());
        draw_list->AddCircleFilled(pos, 5.0f, IM_COL32(255, 100, 100, 255));
    }
    
    // Draw point count
    ImGui::Text("Points: %zu", state.points.size());
    ImGui::Text("Click on canvas to add points");
    
    ImGui::End();
}

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
                if (ImGui::MenuItem("Clear Points")) {
                    app_state.points.clear();
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
            ImGui::EndMainMenuBar();
        }
        
        // Show demo window
        if (app_state.show_demo_window) {
            ImGui::ShowDemoWindow(&app_state.show_demo_window);
        }
        
        // Draw geometry canvas
        DrawGeometryCanvas(app_state);
        
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
