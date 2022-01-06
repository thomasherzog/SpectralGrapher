CPMAddPackage(
        NAME entt
        GITHUB_REPOSITORY skypjack/entt
        GIT_TAG v3.9.0
        DOWNLOAD_ONLY ON
)

if (entt_ADDED)
    add_library(entt INTERFACE)
    target_include_directories(entt INTERFACE ${entt_SOURCE_DIR}/src)
endif ()