CPMAddPackage(
        NAME vma
        GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG beb26af01cda4b68091d700826c77893f2fdbfd2
        DOWNLOAD_ONLY ON
)

if (vma_ADDED)
    add_library(vma INTERFACE)
    target_include_directories(vma INTERFACE ${vma_SOURCE_DIR}/include)
endif ()