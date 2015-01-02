add_definitions(-DZ_HAVE_UNISTD_H -DHAVE_FSEEKO -DHAVE_SYS_TYPES_H -DHAVE_STDDEF_H -DHAVE_STDINT_H)
configure_file(	zlib/zconf.h.cmakein
		${CMAKE_CURRENT_BINARY_DIR}/zconf.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR} zlib/)

set(ZLIB_PUBLIC_HDRS
    ${CMAKE_CURRENT_BINARY_DIR}/zconf.h
    zlib/zlib.h
)
set(ZLIB_PRIVATE_HDRS
    zlib/crc32.h
    zlib/deflate.h
    zlib/gzguts.h
    zlib/inffast.h
    zlib/inffixed.h
    zlib/inflate.h
    zlib/inftrees.h
    zlib/trees.h
    zlib/zutil.h
)
set(ZLIB_SRCS
    zlib/adler32.c
    zlib/compress.c
    zlib/crc32.c
    zlib/deflate.c
    zlib/gzclose.c
    zlib/gzlib.c
    zlib/gzread.c
    zlib/gzwrite.c
    zlib/inflate.c
    zlib/infback.c
    zlib/inftrees.c
    zlib/inffast.c
    zlib/trees.c
    zlib/uncompr.c
    zlib/zutil.c
)

# parse the full version number from zlib.h and include in ZLIB_FULL_VERSION
file(READ zlib/zlib.h _zlib_h_contents)
string(REGEX REPLACE ".*#define[ \t]+ZLIB_VERSION[ \t]+\"([-0-9A-Za-z.]+)\".*"
    "\\1" ZLIB_FULL_VERSION ${_zlib_h_contents})

add_library(zlib STATIC ${ZLIB_SRCS} ${ZLIB_ASMS} ${ZLIB_PUBLIC_HDRS} ${ZLIB_PRIVATE_HDRS})
set_target_properties(zlib PROPERTIES OUTPUT_NAME z)
