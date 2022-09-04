#include "ui/editor/views/ViewportView.h"

#include <GLFW/glfw3.h>
#include "IconsMaterialDesign.h"

void ViewportView::renderView(std::unique_ptr<ComputeRenderer> const &computeRenderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    auto [frameWidth, frameHeight] = ImGui::GetContentRegionAvail();
    computeRenderer->ubo.viewportSize = glm::vec2(frameWidth, frameHeight);
    computeRenderer->ubo.sampleIndex = 0;

    ImGui::Image(
            computeRenderer->imguiTexture,
            ImVec2(frameWidth, frameHeight),
            ImVec2(0, 0),
            ImVec2(frameWidth / computeRenderer->imageWidth, frameHeight / computeRenderer->imageHeight)
    );

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


void ViewportView::renderView(std::unique_ptr<MandelbrotRenderer> const &mandelbrotRenderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    auto [frameWidth, frameHeight] = ImGui::GetContentRegionAvail();
    mandelbrotRenderer->ubo.viewportSize = glm::vec2(frameWidth, frameHeight);

    ImGui::Image(
            mandelbrotRenderer->imguiTexture,
            ImVec2(frameWidth, frameHeight),
            ImVec2(0, 0),
            ImVec2(frameWidth / mandelbrotRenderer->imageWidth, frameHeight / mandelbrotRenderer->imageHeight)
    );

    if (ImGui::IsItemHovered()) {
        if (ImGui::GetIO().MouseWheel != 0) {
            std::cout << "Mouse Position: " << ImGui::GetMousePos().x - ImGui::GetWindowPos().x << " "
                      << ImGui::GetMousePos().y - ImGui::GetWindowPos().y << std::endl;

            mandelbrotRenderer->ubo.scale += ImGui::GetIO().MouseWheel / 2.0f *
                                             (mandelbrotRenderer->ubo.scale != 0 ? mandelbrotRenderer->ubo.scale : 1);
        }
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);

            auto calcBounds = [](glm::vec4 bounds, glm::vec2 imgSize) {
                if (imgSize.x > imgSize.y) {
                    float shift = ((abs(bounds.x - bounds.y)) * imgSize.x / imgSize.y - abs(bounds.x - bounds.y))
                                  / 2.0f;
                    bounds.y += shift;
                    bounds.x -= shift;
                } else if (imgSize.y > imgSize.x) {
                    float shift = ((abs(bounds.z - bounds.w)) * imgSize.y / imgSize.x - abs(bounds.z - bounds.w))
                                  / 2.0f;
                    bounds.w += shift;
                    bounds.z -= shift;
                }
                return bounds;
            };

            auto bounds = calcBounds(mandelbrotRenderer->ubo.bounds, glm::vec2(frameWidth, frameHeight));

            mandelbrotRenderer->ubo.center -= glm::vec2(
                    ((delta.x * ((bounds.y - bounds.x) / frameWidth)) / mandelbrotRenderer->ubo.scale),
                    ((delta.y * ((bounds.z - bounds.w) / frameHeight)) / mandelbrotRenderer->ubo.scale)
            );

            ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
    }

    ImGui::End();
    ImGui::PopStyleVar();
}
