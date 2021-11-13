CPMAddPackage(
        NAME SPIRV-Headers
        GITHUB_REPOSITORY KhronosGroup/SPIRV-Headers
        GIT_TAG 814e728b30ddd0f4509233099a3ad96fd4318c07
        DOWNLOAD_ONLY ON
        OPTIONS
        "SPIRV_HEADERS_SKIP_EXAMPLES ON"
        "SPIRV_HEADERS_SKIP_INSTALL ON"
)

if (SPIRV-Headers_ADDED)
    add_library(SPIRV-Headers INTERFACE)
    target_include_directories(SPIRV-Headers INTERFACE ${SPIRV-Headers_SOURCE_DIR}/include)
endif ()