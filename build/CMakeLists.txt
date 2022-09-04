if (WIN32)
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/win/resource.rc LANGUAGE RC)
    target_sources(SpectralGrapher PRIVATE ${CMAKE_CURRENT_LIST_DIR}/win/resource.rc)
endif ()