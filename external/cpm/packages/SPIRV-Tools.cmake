CPMAddPackage(
        NAME SPIRV-Tools
        GITHUB_REPOSITORY KhronosGroup/SPIRV-Tools
        GIT_TAG v2021.4
        OPTIONS
        "SPIRV_SKIP_TESTS ON"
        "SPIRV_SKIP_EXECUTABLES ON"
)