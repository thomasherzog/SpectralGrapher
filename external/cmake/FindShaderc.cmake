if (WIN32)
    find_path(Shaderc_INCLUDE_DIR
            NAMES shaderc/shaderc.h
            HINTS "$ENV{VULKAN_SDK}/Include")

    if (CMAKE_BUILD_TYPE MATCHES Debug AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        find_library(Shaderc_LIBRARY
                NAMES "shaderc_combinedd"
                HINTS "$ENV{VULKAN_SDK}/Lib"
                NO_DEFAULT_PATH)
    else ()
        find_library(Shaderc_LIBRARY
                NAMES "shaderc_combined"
                HINTS "$ENV{VULKAN_SDK}/Lib"
                NO_DEFAULT_PATH)
    endif ()

else ()
    find_path(Shaderc_INCLUDE_DIR
            NAMES shaderc/shaderc.h
            HINTS "$ENV{VULKAN_SDK}/include")
    find_library(Shaderc_LIBRARY
            NAMES "shaderc_combined"
            HINTS "$ENV{VULKAN_SDK}/lib"
            NO_DEFAULT_PATH)

endif ()

set(Shaderc_LIBRARIES ${Shaderc_LIBRARY})
set(Shaderc_INCLUDE_DIRS ${Shaderc_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Shaderc
        DEFAULT_MSG
        Shaderc_LIBRARY Shaderc_INCLUDE_DIR)

mark_as_advanced(Shaderc_INCLUDE_DIR Shaderc_LIBRARY)

if (Shaderc_FOUND AND NOT TARGET Shaderc::Shaderc)
    add_library(Shaderc::Shaderc UNKNOWN IMPORTED)
    set_target_properties(Shaderc::Shaderc PROPERTIES
            IMPORTED_LOCATION "${Shaderc_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${Shaderc_INCLUDE_DIRS}")
endif ()