set(PNGLIB_MAJOR 1)
set(PNGLIB_MINOR 6)
set(PNGLIB_RELEASE 17)
set(PNGLIB_NAME libpng${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNGLIB_VERSION ${PNGLIB_MAJOR}.${PNGLIB_MINOR}.${PNGLIB_RELEASE})

set(ZLIB_LIBRARY zlib)
set(ZLIB_INCLUDE_DIR ../zlib/)

# SET LIBNAME
set(PNG_LIB_NAME png${PNGLIB_MAJOR}${PNGLIB_MINOR})
set(PNG_LIBRARY ${PNG_LIB_NAME} PARENT_SCOPE)

# Use the prebuilt pnglibconf.h file from the scripts folder
# TODO: fix this by building with awk; without this no cmake build can be
# configured directly (to do so indirectly use your local awk to build a
# pnglibconf.h in the build directory.)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/libpng/scripts/pnglibconf.h.prebuilt
               ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(libpng_public_hdrs
  libpng/png.h
  libpng/pngconf.h
  ${CMAKE_CURRENT_BINARY_DIR}/pnglibconf.h
)
set(libpng_sources
  ${libpng_public_hdrs}
  libpng/pngdebug.h
  libpng/pnginfo.h
  libpng/pngpriv.h
  libpng/pngstruct.h
  libpng/png.c
  libpng/pngerror.c
  libpng/pngget.c
  libpng/pngmem.c
  libpng/pngpread.c
  libpng/pngread.c
  libpng/pngrio.c
  libpng/pngrtran.c
  libpng/pngrutil.c
  libpng/pngset.c
  libpng/pngtrans.c
  libpng/pngwio.c
  libpng/pngwrite.c
  libpng/pngwtran.c
  libpng/pngwutil.c
)

# NOW BUILD OUR TARGET
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/libpng/" ${ZLIB_INCLUDE_DIR})

add_library(${PNG_LIB_NAME} STATIC ${libpng_sources})
target_link_libraries(${PNG_LIB_NAME} ${ZLIB_LIBRARY} ${M_LIBRARY})

# Ensure the CMAKE_LIBRARY_OUTPUT_DIRECTORY is set
IF(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib")
ENDIF(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)

set_target_properties(${PNG_LIB_NAME_STATIC} PROPERTIES
  OUTPUT_NAME ${PNG_LIB_NAME}
  CLEAN_DIRECT_OUTPUT 1)
