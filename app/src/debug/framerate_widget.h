#pragma once

#include "imgui/imgui.h"

#include <vector>
#include <algorithm>

class FrameTimeTracker {
private:
    std::vector<float> frametimes;
    std::vector<float> fps_values;
    int max_samples = 300;  // Number of samples to keep
    float time_accumulator = 0.0f;
    int frame_count = 0;
    float avg_fps = 0.0f;
    float min_frametime = 999.0f;
    float max_frametime = 0.0f;

public:
    void Update(float deltaTime) {
        float frametime_ms = deltaTime * 1000.0f; // Convert to milliseconds

        // Add frametime sample
        frametimes.push_back(frametime_ms);
        if (frametimes.size() > max_samples) {
            frametimes.erase(frametimes.begin());
        }

        // Add FPS sample
        float current_fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
        fps_values.push_back(current_fps);
        if (fps_values.size() > max_samples) {
            fps_values.erase(fps_values.begin());
        }

        // Update statistics
        time_accumulator += deltaTime;
        frame_count++;

        // Update average every 60 frames
        if (frame_count >= 60) {
            avg_fps = 60.0f / time_accumulator;
            time_accumulator = 0.0f;
            frame_count = 0;
        }

        // Update min/max frametime
        if (!frametimes.empty()) {
            min_frametime = *std::min_element(frametimes.begin(), frametimes.end());
            max_frametime = *std::max_element(frametimes.begin(), frametimes.end());
        }
    }

    void RenderImGuiWindow() {
        if (ImGui::Begin("Performance Monitor")) {
            ImGuiIO& io = ImGui::GetIO();

            // Current frame stats
            ImGui::Text("Current Frame:");
            ImGui::Text("  FPS: %.1f", io.Framerate);
            ImGui::Text("  Frame Time: %.3f ms", 1000.0f / io.Framerate);

            ImGui::Separator();

            // Average stats
            ImGui::Text("Average (60 frames):");
            ImGui::Text("  FPS: %.1f", avg_fps);
            ImGui::Text("  Frame Time: %.3f ms", (avg_fps > 0.0f) ? (1000.0f / avg_fps) : 0.0f);

            ImGui::Separator();

            // Min/Max stats
            ImGui::Text("Min/Max Frame Time:");
            ImGui::Text("  Min: %.3f ms", min_frametime);
            ImGui::Text("  Max: %.3f ms", max_frametime);

            ImGui::Separator();

            // Frame time graph
            if (!frametimes.empty()) {
                ImGui::Text("Frame Time Graph (ms):");
                ImGui::PlotLines("##frametime",
                    frametimes.data(),
                    static_cast<int>(frametimes.size()),
                    0,
                    nullptr,
                    0.0f,
                    50.0f,  // Max 50ms scale
                    ImVec2(0, 80));

                ImGui::Text("FPS Graph:");
                ImGui::PlotLines("##fps",
                    fps_values.data(),
                    static_cast<int>(fps_values.size()),
                    0,
                    nullptr,
                    0.0f,
                    120.0f,  // Max 120 FPS scale
                    ImVec2(0, 80));
            }

            ImGui::Separator();

            // Controls
            ImGui::SliderInt("Max Samples", &max_samples, 60, 600);

            if (ImGui::Button("Clear History")) {
                frametimes.clear();
                fps_values.clear();
                min_frametime = 999.0f;
                max_frametime = 0.0f;
            }

            ImGui::SameLine();

            // Color-coded performance indicator
            float current_fps = io.Framerate;
            ImVec4 color;
            const char* status;

            if (current_fps >= 60.0f) {
                color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                status = "EXCELLENT";
            }
            else if (current_fps >= 30.0f) {
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                status = "GOOD";
            }
            else if (current_fps >= 15.0f) {
                color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
                status = "POOR";
            }
            else {
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
                status = "TERRIBLE";
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            ImGui::Text("Performance: %s", status);
            ImGui::PopStyleColor();
        }
        ImGui::End();
    }

    void RenderCompactOverlay() {
        // Compact overlay in corner
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav;

        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos;
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;

        // Position in top-right corner
        window_pos.x = work_pos.x + work_size.x - PAD;
        window_pos.y = work_pos.y + PAD;
        window_pos_pivot.x = 1.0f;
        window_pos_pivot.y = 0.0f;

        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

        if (ImGui::Begin("Performance Overlay", nullptr, window_flags)) {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Frame: %.2f ms", 1000.0f / io.Framerate);

            // Mini frametime graph
            if (!frametimes.empty() && frametimes.size() > 10) {
                // Only show last 60 samples for overlay
                int start_idx = std::max(0, static_cast<int>(frametimes.size()) - 60);
                int count = static_cast<int>(frametimes.size()) - start_idx;

                ImGui::PlotLines("##mini",
                    frametimes.data() + start_idx,
                    count,
                    0,
                    nullptr,
                    0.0f,
                    50.0f,
                    ImVec2(120, 30));
            }
        }
        ImGui::End();
    }
};