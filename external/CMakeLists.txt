find_package(Vulkan REQUIRED)

#add_subdirectory(glfw)
add_subdirectory(imgui)
add_subdirectory(imgui_impl)
add_subdirectory(vma)

include(FetchContent)
FetchContent_Declare(glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
)
FetchContent_GetProperties(glfw)
if (NOT glfw_POPULATED)
    FetchContent_Populate(glfw)
    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC
        Vulkan::Vulkan
        glfw
        imgui
        imgui_impl
        vma)




