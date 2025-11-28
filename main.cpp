#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "SerialPort.hpp" 
#include <string> 

#define V_REF 3.3f
#define ADC_MAX 1023

void StylePIDController() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_Text]                 = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]             = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Border]               = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    style.Colors[ImGuiCol_FrameBg]              = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    style.Colors[ImGuiCol_FrameBgActive]        = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive]        = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    style.Colors[ImGuiCol_Button]               = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Header]               = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.GrabRounding = 5.0f;
}

int main(int, char**) {
    // Setup window
    glfwInit(); 
    if (GLFW_FALSE == glfwInit()) { 
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    GLFWwindow* window = glfwCreateWindow(600, 400, "PID Controller GUI", NULL, NULL); 
    if (window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); 
    glfwSwapInterval(1); 

    // Setup ImGui context
    ImGui::CreateContext(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init("#version 130"); 

    StylePIDController();

    // Application State Variables
    SerialPort serial;
    char com_port[32] = "COM3";
    std::string status_message = "Disconnected";

    float kp = 0.5f;   
    float ki = 0.1f;   
    float kd = 0.05f;  
    float target_voltage = V_REF / 2.0f; 
    int setpoint_adc = 512; 

    int measured_value = 0; 
    int current_output = 0; 

    char command_buffer[64];

    // [ROBUST FIX] Persistent buffer to hold partial data across frames
    std::string serial_accumulator = "";

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 

        ImGui_ImplOpenGL3_NewFrame(); 
        ImGui_ImplGlfw_NewFrame(); 
        ImGui::NewFrame(); 

        ImGui::Begin("PID Controller Dashboard"); 

        // Connection Section
        ImGui::SetNextItemWidth(150); 
        ImGui::InputText("COM Port", com_port, 32);
        ImGui::SameLine(); 
        
        if (ImGui::Button("Connect")) {
            if (serial.connect(com_port)) {
                status_message = "Successfully connected to ";
                status_message += com_port;
            } else {
                status_message = "Failed to connect!";
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Disconnect")) {
            serial.disconnect();
            status_message = "Disconnected";
        }
        
        ImGui::Text("Status: %s", status_message.c_str());
        ImGui::Separator(); 

        // PID Control Section
        ImGui::Text("PID Gains");
        
        if (ImGui::SliderFloat("Kp", &kp, 0.0f, 5.0f, "%.3f")) {
            snprintf(command_buffer, sizeof(command_buffer), "p%f\n", kp);
            if (serial.isConnected()) serial.write(command_buffer);
        }
        
        if (ImGui::SliderFloat("Ki", &ki, 0.0f, 1.0f, "%.3f")) {
            snprintf(command_buffer, sizeof(command_buffer), "i%f\n", ki);
            if (serial.isConnected()) serial.write(command_buffer);
        }
        
        if (ImGui::SliderFloat("Kd", &kd, 0.0f, 1.0f, "%.3f")) {
            snprintf(command_buffer, sizeof(command_buffer), "d%f\n", kd);
            if (serial.isConnected()) serial.write(command_buffer);
        }

        ImGui::Separator();

        // Setpoint Control Section
        ImGui::Text("Setpoint (%0.1fV - %0.1fV)", 0.0f, V_REF);
        
        if (ImGui::SliderFloat("Target Voltage", &target_voltage, 0.0f, V_REF, "%.2f V")) {
            if (target_voltage < 0.0f) target_voltage = 0.0f;
            if (target_voltage > V_REF) target_voltage = V_REF;
            
            setpoint_adc = (int)((target_voltage / V_REF) * ADC_MAX);
            
            snprintf(command_buffer, sizeof(command_buffer), "s%d\n", setpoint_adc);
            if (serial.isConnected()) serial.write(command_buffer);
        }
        ImGui::SameLine();
        ImGui::Text("(ADC: %d)", setpoint_adc);

        // ============================================================
        // [ROBUST FIX] LIVE DATA READING AND PARSING
        // ============================================================
        if (serial.isConnected()) {
            char read_buffer[64];
            int bytesRead = serial.read(read_buffer, sizeof(read_buffer) - 1);

            if (bytesRead > 0) {
                read_buffer[bytesRead] = '\0';
                
                // 1. Accumulate the new chunk of data
                serial_accumulator += read_buffer;

                // 2. Process all complete lines (indicated by '\n')
                size_t newline_pos;
                while ((newline_pos = serial_accumulator.find('\n')) != std::string::npos) {
                    
                    // Extract the single line
                    std::string line = serial_accumulator.substr(0, newline_pos);
                    
                    // Optional: Remove '\r' if it exists (CRLF handling)
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }

                    // Parse the line
                    int temp_setpoint, temp_measured, temp_output;
                    if (sscanf(line.c_str(), "D,%u,%u,%u", 
                               &temp_setpoint, &temp_measured, &temp_output) == 3) {
                        measured_value = temp_measured;
                        current_output = temp_output;
                    }

                    // 3. Remove the processed line from the accumulator
                    serial_accumulator.erase(0, newline_pos + 1);
                }
            }
        }
        // ============================================================
        
        ImGui::Separator();
        ImGui::Text("LIVE DATA");
        
        float measured_voltage = ((float)measured_value / ADC_MAX) * V_REF;
        ImGui::Text("Measured Output (Vout): %.2f V", measured_voltage);
        ImGui::Text("Measured ADC Value: %d / 1023", measured_value);
        ImGui::Text("PWM Output (Duty Cycle): %d / 255", current_output);

        ImGui::End(); 

        ImGui::Render(); 
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f); 
        glClear(GL_COLOR_BUFFER_BIT); 
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 
        glfwSwapBuffers(window); 
    }
    
    if (serial.isConnected()) {
        serial.disconnect();
    }

    ImGui_ImplOpenGL3_Shutdown(); 
    ImGui_ImplGlfw_Shutdown(); 
    ImGui::DestroyContext(); 
    glfwTerminate(); 
    return 0;
}