list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

list(APPEND cpm_packages
        glfw
        freetype
        vma
        implot
        imgui-node-editor
        glm
        SPIRV-Headers
        SPIRV-Tools
        glslang
        shaderc
        stb
        entt
        json
        IconFontCppHeaders)

include(CPM)

include(${CMAKE_CURRENT_SOURCE_DIR}/packages/cmrc.cmake)

foreach (package IN LISTS cpm_packages)
    include(${CMAKE_CURRENT_SOURCE_DIR}/packages/${package}.cmake)
endforeach ()

target_link_libraries(${PROJECT_NAME} PUBLIC ${cpm_packages})

