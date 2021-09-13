add_library(vma INTERFACE)

target_link_libraries(vma INTERFACE Vulkan::Vulkan)

target_include_directories(vma INTERFACE include/)