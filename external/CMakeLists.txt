find_package(Vulkan REQUIRED QUIET)

add_subdirectory(imgui)
add_subdirectory(imgui_impl)

add_subdirectory(cpm)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Vulkan::Vulkan
        imgui
        imgui_impl)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PUBLIC
            comctl32.lib
            dwmapi.lib
            Uxtheme.lib
            Gdi32.lib)
endif ()