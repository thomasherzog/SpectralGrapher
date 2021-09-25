list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

include(CPM)

find_package(Vulkan REQUIRED QUIET)

add_subdirectory(cmrc)
add_subdirectory(glfw)
add_subdirectory(freetype)
add_subdirectory(imgui)
add_subdirectory(imgui_impl)
add_subdirectory(vma)
add_subdirectory(implot)

target_link_libraries(${PROJECT_NAME} PUBLIC
        Vulkan::Vulkan
        glfw
        freetype
        imgui
        imgui_impl
        vma
        implot)