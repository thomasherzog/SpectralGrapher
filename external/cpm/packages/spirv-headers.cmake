CPMAddPackage(
        NAME spirv-headers
        GITHUB_REPOSITORY KhronosGroup/SPIRV-Headers
        GIT_TAG 92f21c9b214178ce67cf1e31a00a33312590403a
        DOWNLOAD_ONLY ON
        OPTIONS
        "SPIRV_HEADERS_SKIP_EXAMPLES ON"
        "SPIRV_HEADERS_SKIP_INSTALL ON"
)

if (spirv-headers_ADDED)
    add_library(spirv-headers INTERFACE)
    target_include_directories(spirv-headers INTERFACE ${spirv-headers_SOURCE_DIR}/include)
    add_library(SPIRV-Headers ALIAS spirv-headers)
endif ()