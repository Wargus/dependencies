project ( libmng C )
cmake_minimum_required ( VERSION 2.8 )
include ( cmake/dist.cmake )

# find libs
find_package ( ZLIB REQUIRED )
find_package ( JPEG REQUIRED )
find_path ( LCMS_INCLUDE_DIR NAMES lcms.h )
find_library ( LCMS_LIBRARY NAMES lcms )
include_directories ( ${ZLIB_INCLUDE_DIR} ${JPEG_INCLUDE_DIR} ${LCMS_INCLUDE_DIR} )

# libmng Library
set ( LIBMNG_SRCS libmng_callback_xs.c libmng_chunk_io.c libmng_chunk_descr.c libmng_chunk_prc.c 
  libmng_chunk_xs.c libmng_cms.c libmng_display.c libmng_dither.c libmng_error.c 
  libmng_filter.c libmng_hlapi.c libmng_jpeg.c libmng_object_prc.c libmng_pixels.c 
  libmng_prop_xs.c libmng_read.c libmng_trace.c libmng_write.c libmng_zlib.c )
add_definitions ( -DMNG_ACCESS_CHUNKS -DMNG_STORE_CHUNKS -DMNG_FULL_CMS -DMNG_SUPPORT_FULL -DMNG_SUPPORT_TRACE -DMNG_ERROR_TELLTALE -DMNG_SUPPORT_READ -DMNG_SUPPORT_WRITE -DMNG_SUPPORT_DISPLAY )
if ( MINGW )
  add_definitions ( -DMNG_BUILD_DLL )
  set ( M_LIBRARY m )
elseif ( LINUX )
	add_definitions ( -DMNG_BUILD_SO )
	set ( M_LIBRARY m )
endif ( )

add_library ( mng ${LIBMNG_SRCS} )
target_link_libraries ( mng ${ZLIB_LIBRARY} ${JPEG_LIBRARY} ${LCMS_LIBRARY} ${M_LIBRARY} )
if ( MINGW )
  set_target_properties ( mng PROPERTIES LINK_FLAGS "-Wl,--add-stdcall-alias -Wl,--kill-at" 
    CLEAN_DIRECT_OUTPUT 1 )
endif ( )

# install	
install_library ( mng )
install_header ( libmng.h libmng_conf.h libmng_types.h )
install_data ( CHANGES LICENSE README )
