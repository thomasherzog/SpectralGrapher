cmake_minimum_required(VERSION 3.20)
project(SpectralGrapher)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

set(source_files "")

if (WIN32)
    list(APPEND source_files
            src/native/windows/Win32CustomTitlebar.cpp)
endif ()

list(APPEND source_files
        src/main.cpp

        src/graphics/vulkan/core/Context.cpp
        src/graphics/vulkan/core/SharedContext.cpp
        src/graphics/vulkan/core/Instance.cpp
        src/graphics/vulkan/core/PhysicalDevice.cpp
        src/graphics/vulkan/core/Device.cpp
        src/graphics/vulkan/core/Surface.cpp
        src/graphics/vulkan/core/InFlightFrames.cpp
        src/graphics/vulkan/core/Swapchain.cpp
        src/graphics/vulkan/core/Texture.cpp

        src/graphics/vulkan/debug/DebugMessenger.cpp

        src/windowing/WindowManager.cpp
        src/windowing/BaseWindow.cpp
        src/windowing/VulkanWindow.cpp
        src/windowing/ImGuiWindow.cpp

        #src/shape/ShapeManager.cpp
        #src/shape/ShapeType.cpp
        #src/shape/ShapeFieldType.cpp

        src/ui/editor/EditorWindow.cpp
        src/ui/editor/StartWindow.cpp

        src/ui/editor/views/ViewportView.cpp
        src/ui/editor/views/PropertiesView.cpp
        src/ui/editor/views/DockSpaceView.cpp
        src/ui/editor/views/ViewportBarsView.cpp

        src/ui/editor/popups/Popup.cpp
        src/ui/editor/popups/SettingsPopup.cpp

        src/ui/appearance/UIThemeManager.cpp
        src/ui/components/NativeTitlebar.cpp

        src/renderer/general/RecordedCommandBuffer.cpp
        src/renderer/general/SwapchainImageRenderer.cpp
        src/renderer/general/ImGuiRenderer.cpp

        src/renderer/ComputeRenderer.cpp
        src/renderer/MandelbrotRenderer.cpp
        src/renderer/FunctionQueue.cpp
        )

add_executable(SpectralGrapher ${source_files})

target_compile_definitions(SpectralGrapher PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

add_subdirectory(build)
add_subdirectory(external)
add_subdirectory(assets)