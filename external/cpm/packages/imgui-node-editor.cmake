CPMAddPackage(
        NAME imgui-node-editor
        GITHUB_REPOSITORY thedmd/imgui-node-editor
        GIT_TAG d79ebdeb2375fa6c1f88a40e52e7d08c8d472b62
        DOWNLOAD_ONLY YES
)

if (imgui-node-editor_ADDED)
    add_library(imgui-node-editor
            ${imgui-node-editor_SOURCE_DIR}/crude_json.cpp
            ${imgui-node-editor_SOURCE_DIR}/imgui_canvas.cpp
            ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor.cpp
            ${imgui-node-editor_SOURCE_DIR}/imgui_node_editor_api.cpp)

    target_include_directories(imgui-node-editor PUBLIC ${imgui-node-editor_SOURCE_DIR})
    target_link_libraries(imgui-node-editor PUBLIC imgui)
endif ()