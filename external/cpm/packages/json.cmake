CPMAddPackage(
        NAME json
        VERSION 3.10.5
        URL https://github.com/nlohmann/json/releases/download/v3.10.5/include.zip
)

if (json_ADDED)
    add_library(json INTERFACE IMPORTED)
    target_include_directories(json INTERFACE ${json_SOURCE_DIR}/include)
endif()