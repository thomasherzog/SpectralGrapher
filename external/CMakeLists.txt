find_package(Vulkan REQUIRED QUIET)

add_subdirectory(imgui)
add_subdirectory(imgui_impl)

add_subdirectory(cpm)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Vulkan::Vulkan
        imgui
        imgui_impl)