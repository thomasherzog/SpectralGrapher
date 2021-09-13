find_package(Vulkan REQUIRED)

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




