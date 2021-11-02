cmake_minimum_required(VERSION 3.20)
project(SpectralGrapher)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(SpectralGrapher
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

        src/examples/TestImWindow.cpp
        src/examples/SimpleRasterizerWindowEx.cpp

        src/renderer/RecordedCommandBuffer.cpp
        src/renderer/ComputeRenderer.cpp
        src/renderer/SwapchainImageRenderer.cpp
        src/renderer/ImGuiRenderer.cpp

        )

target_compile_definitions(SpectralGrapher PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
add_subdirectory(external)
add_subdirectory(assets)