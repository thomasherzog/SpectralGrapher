list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(CPM)

find_package(Vulkan REQUIRED QUIET)

add_subdirectory(glfw)
add_subdirectory(imgui)
add_subdirectory(imgui_impl)
add_subdirectory(vma)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Vulkan::Vulkan
        glfw
        imgui
        imgui_impl
        vma)

find_package(Shaderc REQUIRED QUIET)
target_link_libraries(${PROJECT_NAME} PUBLIC Shaderc::Shaderc)


