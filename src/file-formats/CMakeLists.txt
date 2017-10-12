if ( EXISTS ${LIBNETCDF_LIBRARIES} )
  set(FILE_PLUGINS_EXTRA ${FILE_PLUGINS_EXTRA} "file-netcdf.c")
  include_directories(${LIBNETCDF_INCLUDE_DIRS})
endif()

add_library(scil-file-formats SHARED ${FILE_PLUGINS_EXTRA} "file-csv.c" "scil-file-format.c" "file-bin.c" "file-brick-of-floats.c")

if ( EXISTS ${LIBNETCDF_LIBRARIES} )
  target_link_libraries(scil-file-formats ${LIBNETCDF_LIBRARIES})
  target_compile_definitions(scil-file-formats PUBLIC HAVE_NETCDF)
endif()

install(TARGETS scil-file-formats LIBRARY DESTINATION lib)
install(FILES scil-file-formats.h DESTINATION include)
