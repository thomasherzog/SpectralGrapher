CPMAddPackage(
        NAME implot
        GITHUB_REPOSITORY epezent/implot
        GIT_TAG dea3387cdcc1d6a7ee3607f8a37a9dce8a85224f
        DOWNLOAD_ONLY YES
)

if (implot_ADDED)
    add_library(implot ${implot_SOURCE_DIR}/implot.cpp ${implot_SOURCE_DIR}/implot_items.cpp ${implot_SOURCE_DIR}/implot_demo.cpp)
    target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
    target_link_libraries(implot PUBLIC imgui)
endif ()