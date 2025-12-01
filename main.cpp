#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "SerialPort.hpp" 
#include <string> 
#include <vector>
#include <cmath>

#define V_REF 3.3f
#define ADC_MAX 1023
#define WAVE_SIZE 256 // Matches the buffer size on AVR

void StylePIDController() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.FrameRounding = 5.0f;
}

int main(int, char**) {
    glfwInit(); 
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rho5 Waveform Generator", NULL, NULL); 
    glfwMakeContextCurrent(window); 
    glfwSwapInterval(1); 

    ImGui::CreateContext(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init("#version 130"); 
    StylePIDController();

    SerialPort serial;
    char com_port[32] = "COM3";
    std::string status_message = "Disconnected";

    // Waveform Data (0-255)
    // Initialize to mid-point (128)
    std::vector<uint8_t> waveform(WAVE_SIZE, 128); 
    
    // Float buffer for displaying in ImGui Plot
    std::vector<float> plot_data(WAVE_SIZE);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 
        ImGui_ImplOpenGL3_NewFrame(); 
        ImGui_ImplGlfw_NewFrame(); 
        ImGui::NewFrame(); 

        ImGui::Begin("Waveform Draw"); 

        // Connection
        ImGui::InputText("COM Port", com_port, 32);
        ImGui::SameLine(); 
        if (ImGui::Button("Connect")) {
            if (serial.connect(com_port)) status_message = "Connected";
            else status_message = "Failed";
        }
        ImGui::Text("Status: %s", status_message.c_str());
        ImGui::Separator(); 

        // ---------------------------------------------------------
        // WAVEFORM EDITOR
        // ---------------------------------------------------------
        ImGui::Text("Draw your waveform below (Click and Drag)");

        // 1. Convert byte data to float for visualization
        for(int i=0; i<WAVE_SIZE; i++) {
            plot_data[i] = (float)waveform[i];
        }

        // 2. Draw the Plot
        // We use an invisible button overlay to capture mouse interaction
        ImVec2 canvas_size(700, 300);
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        
        ImGui::PlotLines("##Waveform", plot_data.data(), WAVE_SIZE, 0, NULL, 0.0f, 255.0f, canvas_size);

        // 3. Handle Mouse Input
        if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            ImVec2 mouse_pos = ImGui::GetMousePos();
            
            // Calculate X index (0 to 255)
            float x_rel = mouse_pos.x - canvas_pos.x;
            int index = (int)((x_rel / canvas_size.x) * WAVE_SIZE);

            // Calculate Y value (255 at top, 0 at bottom)
            // Note: ImGui Y coordinates grow downwards, so we invert
            float y_rel = mouse_pos.y - canvas_pos.y;
            int value = 255 - (int)((y_rel / canvas_size.y) * 255);

            // Clamp and Update
            if (index >= 0 && index < WAVE_SIZE) {
                if (value < 0) value = 0;
                if (value > 255) value = 255;
                
                // Brush size (update neighbors for smoothness)
                waveform[index] = (uint8_t)value;
                if(index > 0) waveform[index-1] = (uint8_t)value;
                if(index < WAVE_SIZE-1) waveform[index+1] = (uint8_t)value;
            }
        }

        // 4. Buttons
        if (ImGui::Button("Upload Waveform")) {
            if (serial.isConnected()) {
                // Protocol: Send 'w' then the 256 bytes
                std::string header = "w";
                serial.write(header);
                
                // Note: SerialPort::write takes std::string which might stop at \0
                // So we do a manual write loop or use a specialized write method.
                // For simplicity, let's just send char by char to be safe
                for(uint8_t byte : waveform) {
                    std::string s(1, (char)byte);
                    serial.write(s);
                }
                status_message = "Waveform Sent!";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset to Flat")) {
            std::fill(waveform.begin(), waveform.end(), 128);
        }

        ImGui::End(); 
        ImGui::Render(); 
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT); 
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 
        glfwSwapBuffers(window); 
    }
    
    if (serial.isConnected()) serial.disconnect();
    ImGui_ImplOpenGL3_Shutdown(); 
    ImGui_ImplGlfw_Shutdown(); 
    ImGui::DestroyContext(); 
    glfwTerminate(); 
    return 0;
}