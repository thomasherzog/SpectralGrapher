CPMAddPackage(
        NAME stb
        GITHUB_REPOSITORY nothings/stb
        GIT_TAG af1a5bc352164740c1cc1354942b1c6b72eacb8a
        DOWNLOAD_ONLY ON
)

if (stb_ADDED)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
endif ()