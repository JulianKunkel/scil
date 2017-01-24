if ( EXISTS ${LIBNETCDF_LIBRARIES} )
  set(FILE_PLUGINS_EXTRA ${FILE_PLUGINS_EXTRA} "file-netcdf.c")
  include_directories(${LIBNETCDF_INCLUDE_DIRS})
endif()

add_library(scil-tools SHARED ${FILE_PLUGINS_EXTRA} "file-csv.c" "file-plugin.c" "file-bin.c")
target_link_libraries(scil-tools ${LIBNETCDF_LIBRARIES})

install(TARGETS scil-tools LIBRARY DESTINATION lib)
