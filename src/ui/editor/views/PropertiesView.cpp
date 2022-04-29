#include "ui/editor/views/PropertiesView.h"

void PropertiesView::renderView(const std::unique_ptr<ComputeRenderer> &computeRenderer) {
    ImGui::Begin("Properties");

    if (ImGui::DragFloat3("Position", (float *) &computeRenderer->ubo.position, 0.1f)) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::DragFloat3("Rotation", (float *) &computeRenderer->ubo.rotation, 0.1f)) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::DragFloat("Field Of View", &computeRenderer->ubo.fov, 0.1f)) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::InputInt("Samples", &computeRenderer->ubo.sampleCount)) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::InputFloat("Epsilon", &computeRenderer->ubo.epsilon, 0.00005, 0.00005, "%.5f")) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::InputInt("Max Time", &computeRenderer->ubo.maxTime)) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::InputInt("Max Steps", &computeRenderer->ubo.maxSteps)) {
        computeRenderer->ubo.sampleIndex = 0;
    }
    if (ImGui::ColorEdit3("Background Color", (float *) &computeRenderer->ubo.backgroundColor)) {
        computeRenderer->ubo.sampleIndex = 0;
    }

    ImGui::End();
}

void PropertiesView::renderView(const std::unique_ptr<MandelbrotRenderer> &mandelbrotRenderer) {
    ImGui::Begin("Properties");

    ImGui::DragFloat4("Bounds", (float *) &mandelbrotRenderer->ubo.bounds, 0.1f);

    ImGui::DragFloat2("Center", (float *) &mandelbrotRenderer->ubo.center, 0.1f);

    ImGui::DragFloat("Scale", (float *) &mandelbrotRenderer->ubo.scale, 0.1f);

    ImGui::DragInt("Iterations", (int *) &mandelbrotRenderer->ubo.iterations, 1);

    ImGui::End();
}
