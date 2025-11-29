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
    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
    style.FrameRounding = 5.0f;
}

int main(int, char**) {
    glfwInit(); 
    GLFWwindow* window = glfwCreateWindow(600, 400, "PID Controller GUI", NULL, NULL); 
    glfwMakeContextCurrent(window); 
    glfwSwapInterval(1); 

    ImGui::CreateContext(); 
    ImGui_ImplGlfw_InitForOpenGL(window, true); 
    ImGui_ImplOpenGL3_Init("#version 130"); 
    StylePIDController();

    SerialPort serial;
    char com_port[32] = "COM3";
    std::string status_message = "Disconnected";

    float kp = 0.5f;   
    float ki = 0.1f;   
    float kd = 0.05f;  
    float target_voltage = 0.0f; 
    int setpoint_adc = 0; 

    int measured_value = 0; 
    int current_output = 0; 
    
    // Debug: What setpoint does the AVR *actually* have?
    int avr_reported_setpoint = 0;

    char command_buffer[64];
    std::string serial_accumulator = "";

    // [FIX] Rate Limiting Variables
    double last_send_time = 0.0;
    const double SEND_INTERVAL = 0.05; // Send max every 50ms (20Hz)

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents(); 
        ImGui_ImplOpenGL3_NewFrame(); 
        ImGui_ImplGlfw_NewFrame(); 
        ImGui::NewFrame(); 

        ImGui::Begin("PID Controller Dashboard"); 

        // Connection
        ImGui::InputText("COM Port", com_port, 32);
        ImGui::SameLine(); 
        if (ImGui::Button("Connect")) {
            if (serial.connect(com_port)) status_message = "Connected";
            else status_message = "Failed";
        }
        ImGui::SameLine();
        if (ImGui::Button("Disconnect")) {
            serial.disconnect();
            status_message = "Disconnected - to reconnect restart app";
        }
        ImGui::Text("Status: %s", status_message.c_str());
        ImGui::Separator(); 

        // [FIX] Check Time for Throttling
        double current_time = glfwGetTime();
        bool ready_to_send = (current_time - last_send_time) > SEND_INTERVAL;

        // PID Sliders
        // We calculate 'changed' so we only send if the user moved something AND time is up
        bool changed = false;
        
        ImGui::Text("PID Gains");
        changed |= ImGui::SliderFloat("Kp", &kp, 0.0f, 5.0f, "%.3f");
        if (changed && ready_to_send && serial.isConnected()) {
            snprintf(command_buffer, sizeof(command_buffer), "p%.3f\n", kp);
            serial.write(command_buffer);
            last_send_time = current_time;
            changed = false; // Reset for next check
        }

        changed |= ImGui::SliderFloat("Ki", &ki, 0.0f, 1.0f, "%.3f");
        if (changed && ready_to_send && serial.isConnected()) {
            snprintf(command_buffer, sizeof(command_buffer), "i%.3f\n", ki);
            serial.write(command_buffer);
            last_send_time = current_time;
            changed = false;
        }

        changed |= ImGui::SliderFloat("Kd", &kd, 0.0f, 1.0f, "%.3f");
        if (changed && ready_to_send && serial.isConnected()) {
            snprintf(command_buffer, sizeof(command_buffer), "d%.3f\n", kd);
            serial.write(command_buffer);
            last_send_time = current_time;
            changed = false;
        }

        ImGui::Separator();

        // Setpoint
        ImGui::Text("Setpoint");
        if (ImGui::SliderFloat("Target Voltage", &target_voltage, 0.0f, V_REF, "%.2f V")) {
            setpoint_adc = (int)((target_voltage / V_REF) * ADC_MAX);
            if (ready_to_send && serial.isConnected()) {
                snprintf(command_buffer, sizeof(command_buffer), "s%d\n", setpoint_adc);
                serial.write(command_buffer);
                last_send_time = current_time;
            }
        }
        
        // Debug Info
        ImGui::Text("PC Target ADC: %d", setpoint_adc);
        ImGui::Text("AVR Actual ADC: %d", avr_reported_setpoint); // [FIX] Visualizing the bug

        // Reading Data
        if (serial.isConnected()) {
            char read_buffer[64];
            int bytesRead = serial.read(read_buffer, sizeof(read_buffer) - 1);
            if (bytesRead > 0) {
                read_buffer[bytesRead] = '\0';
                serial_accumulator += read_buffer;
                size_t newline_pos;
                while ((newline_pos = serial_accumulator.find('\n')) != std::string::npos) {
                    std::string line = serial_accumulator.substr(0, newline_pos);
                    if (!line.empty() && line.back() == '\r') line.pop_back();

                    // Parse D,setpoint,measured,output
                    sscanf(line.c_str(), "D,%d,%d,%d", 
                           &avr_reported_setpoint, &measured_value, &current_output);
                    
                    serial_accumulator.erase(0, newline_pos + 1);
                }
            }
        }
        
        ImGui::Separator();
        ImGui::Text("LIVE DATA");
        float measured_v = ((float)measured_value / ADC_MAX) * V_REF;
        ImGui::Text("Measured: %.2f V", measured_v);
        ImGui::Text("PWM Output: %d / 255", current_output);

        ImGui::End(); 
        ImGui::Render(); 
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f); 
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