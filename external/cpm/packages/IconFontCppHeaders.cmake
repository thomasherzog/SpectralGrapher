CPMAddPackage(
        NAME IconFontCppHeaders
        GITHUB_REPOSITORY juliettef/IconFontCppHeaders
        GIT_TAG 7d6ff1f4ba51e7a2b142be39457768abece1549c
        DOWNLOAD_ONLY ON
)

if (IconFontCppHeaders_ADDED)
    add_library(IconFontCppHeaders INTERFACE)
    target_include_directories(IconFontCppHeaders INTERFACE ${IconFontCppHeaders_SOURCE_DIR})
endif ()