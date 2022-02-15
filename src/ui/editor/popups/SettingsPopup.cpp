#include "ui/editor/popups/SettingsPopup.h"

SettingsPopup::SettingsPopup() = default;

SettingsPopup::~SettingsPopup() = default;

void SettingsPopup::renderPopup(std::shared_ptr<vulkan::Context> &context) {
    if (isPopupOpen) {
        ImGui::OpenPopup("Settings");
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(900, 600), ImVec2(FLT_MAX, FLT_MAX));

    if (ImGui::BeginPopupModal("Settings", nullptr, 0)) {

        if(!isPopupOpen) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::BeginChild("SettingsNaviation", ImVec2(250, -50), false);

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("SettingsView", ImVec2(0, -50), true);

        if (ImGui::BeginTable("Devices", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
            auto devices = context->getInstance()->getInstance().enumeratePhysicalDevices();
            ImGui::TableSetupColumn("Device name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Device type", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("API version", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (auto &device: devices) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s", device.getProperties().deviceName);
                ImGui::TableNextColumn();
                ImGui::Text("%s", vk::to_string(device.getProperties().deviceType).c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%i.%i.%i",
                            VK_VERSION_MAJOR(device.getProperties().apiVersion),
                            VK_VERSION_MINOR(device.getProperties().apiVersion),
                            VK_VERSION_PATCH(device.getProperties().apiVersion));
            }
            ImGui::EndTable();
        }
        ImGui::EndChild();

        ImGui::BeginChild("SettingsBottomBar", ImVec2(0, 0), true);
        if (ImGui::Button("Hello")) {
            isPopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndChild();

        ImGui::End();
    }
}


