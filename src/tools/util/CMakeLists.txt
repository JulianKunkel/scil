include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if ( EXISTS ${LIBNETCDF_LIBRARIES} )
  set(FILE_PLUGINS_EXTRA ${FILE_PLUGINS_EXTRA} "file-formats/file-netcdf.c")
  include_directories(${LIBNETCDF_INCLUDE_DIRS})
endif()

set( FILE_PLUGINS_EXTRA ${FILE_PLUGINS_EXTRA}
  "file-formats/file-csv.c" "file-formats/scil-file-format.c" "file-formats/file-bin.c" "file-formats/file-brick-of-floats.c"
  )

add_library(scil-tools-util SHARED "scil-option.c" ${FILE_PLUGINS_EXTRA})

if ( EXISTS ${LIBNETCDF_LIBRARIES} )
  target_link_libraries(scil-tools-util ${LIBNETCDF_LIBRARIES})
  target_compile_definitions(scil-tools-util PUBLIC HAVE_NETCDF)
endif()

install(TARGETS scil-tools-util LIBRARY DESTINATION lib)
