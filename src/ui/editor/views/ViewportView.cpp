#include "ui/editor/views/ViewportView.h"

void ViewportView::renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    auto[frameWidth, frameHeight] = ImGui::GetContentRegionAvail();
    ImGui::Image(computeRenderer->imguiTexture, ImVec2(frameWidth, frameHeight));
    if (ImGui::IsItemHovered()) {
        if (ImGui::GetIO().KeyCtrl && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
            printf("%f - %f\n", delta.x, delta.y);
        } else // Mouse wheel zoom
        if (ImGui::GetIO().MouseWheel != 0) {
            glm::vec2 x = glm::vec2(std::cos(computeRenderer->ubo.rotation.x),
                                    std::sin(computeRenderer->ubo.rotation.x));
            glm::vec2 y = glm::vec2(std::cos(computeRenderer->ubo.rotation.y),
                                    std::sin(computeRenderer->ubo.rotation.y));
            glm::vec2 z = glm::vec2(std::cos(computeRenderer->ubo.rotation.z),
                                    std::sin(computeRenderer->ubo.rotation.z));

            glm::mat3x3 rotationMatrix;
            rotationMatrix[0] = glm::vec3(
                    x.x * z.x - x.y * y.x * z.y,
                    -x.x * z.y - x.y * y.x * z.x,
                    x.y * y.y
            );
            rotationMatrix[1] = glm::vec3(
                    x.y * z.x + x.x * y.x * z.y,
                    -x.y * z.y + x.x * y.x * z.x,
                    -x.x * y.y
            );
            rotationMatrix[2] = glm::vec3(
                    y.y * z.y,
                    y.y * z.x,
                    y.x
            );
            auto directionVector = rotationMatrix * glm::vec3(1, 0, 0);
            computeRenderer->ubo.position += directionVector * (ImGui::GetIO().MouseWheel / 2.0f);
            computeRenderer->ubo.sampleIndex = 0;
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
