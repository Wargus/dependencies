set(CMAKE_CURRENT_MACRO_LIST_DIR "${CMAKE_CURRENT_LIST_DIR}" PARENT_SCOPE)

macro(vendored_lua)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_LUA)
    set(USE_VENDORED_LUA true)

    ExternalProject_Add(lua
      SOURCE_DIR ${CMAKE_CURRENT_MACRO_LIST_DIR}/lua-5.1.5/
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/lua/"
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
      INSTALL_COMMAND ""
    )

    set(Lua51_FOUND true)
    if(MSVC)
      set(LUA_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/$<CONFIGURATION>/lua51.lib")
      set(LUA_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/")
      set(TOLUA++_FOUND true)
      set(TOLUA++_INCLUDE_DIR ${CMAKE_CURRENT_MACRO_LIST_DIR}/lua-5.1.5/toluapp/include/)
      set(TOLUA++_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/$<CONFIGURATION>/toluapp51.lib")
      set(TOLUA++_APP "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/$<CONFIGURATION>/toluapp.exe")
    else()
      set(LUA_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/liblua51.a")
      set(LUA_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/")
      set(TOLUA++_FOUND true)
      set(TOLUA++_INCLUDE_DIR ${CMAKE_CURRENT_MACRO_LIST_DIR}/lua-5.1.5/toluapp/include/)
      set(TOLUA++_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/libtoluapp51.a")
      set(TOLUA++_APP "${CMAKE_CURRENT_BINARY_DIR}/lua/src/lua-build/toluapp")
    endif(MSVC)
  endif()
endmacro()

macro(vendored_zlib)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_ZLIB)
    set(USE_VENDORED_ZLIB true)

    file(GLOB ZLIB_HEADERS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/zlib/*.h")
    # zconf.h is generated into output dir, don't copy it from the source.
    list(REMOVE_ITEM ZLIB_HEADERS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/zlib/zconf.h")

    ExternalProject_Add(zlib
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/zlib"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/zlib/"
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
      INSTALL_COMMAND cmake -E copy ${ZLIB_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/zlib/src/zlib-build/"
      # the SDL included version of zlib is patched to rename zconf.h -> zconf.h.included, but that leaves the tree dirty...
      TEST_COMMAND cmake -E rename "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/zlib/zconf.h.included" "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/zlib/zconf.h"
    )
    set(ZLIB_FOUND TRUE)

    set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/zlib/src/zlib-build/")
    if(MSVC)
      set(ZLIB_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/zlib/src/zlib-build/$<CONFIGURATION>/zlibstatic$<$<CONFIG:Debug>:d>.lib")
    else()
      set(ZLIB_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/zlib/src/zlib-build/libz.a")
    endif()
  endif()
endmacro()

macro(vendored_png)
  include(ExternalProject)
  vendored_zlib()
  if(NOT DEFINED USE_VENDORED_PNG)
    set(USE_VENDORED_PNG true)

    file(GLOB PNG_HEADERS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/libpng/*.h")

    ExternalProject_Add(png
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/libpng"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/png/"
      INSTALL_COMMAND cmake -E copy ${PNG_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/png/src/png-build/"
      CMAKE_CACHE_ARGS -DZLIB_FOUND:STRING=1 -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR} -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARIES}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )

    add_dependencies(png zlib)

    set(PNG_FOUND TRUE)
    set(PNG_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/png/src/png-build/")
    if(MSVC)
      set(PNG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/png/src/png-build/$<CONFIGURATION>/libpng16_static$<$<CONFIG:Debug>:d>.lib" ${ZLIB_LIBRARIES})
    else()
      set(PNG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/png/src/png-build/libpng.a" ${ZLIB_LIBRARIES})
    endif()
  endif()
endmacro()

macro(vendored_bzip2)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_BZIP2)
    set(USE_VENDORED_BZIP2 true)

    ExternalProject_Add(bzip2
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/bzip2"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/bzip2/"
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_STATIC_LIBS:BOOL=ON
      INSTALL_COMMAND ""
    )

    set(BZIP2_FOUND TRUE)
    set(BZIP2_INCLUDE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/bzip2/")
    if(MSVC)
      set(BZIP2_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/bzip2/src/bzip2-build/$<CONFIGURATION>/bz2.lib")
    else()
      set(BZIP2_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/bzip2/src/bzip2-build/libbz2.a")
    endif()
  endif()
endmacro()

macro(vendored_ogg)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_OGG)
    set(USE_VENDORED_OGG true)

    file(GLOB OGG_HEADERS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_mixer/external/ogg/include/ogg/*.h")
    ExternalProject_Add(ogg
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_mixer/external/ogg"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/ogg/"
      INSTALL_COMMAND cmake -E copy ${OGG_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/ogg/src/ogg-build/include/ogg/"
      CMAKE_ARGS -DBUILD_SHARED_LIBS=FALSE
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )

    set(OGG_FOUND TRUE)
    set(OGG_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/ogg/src/ogg-build/include/")
    if(MSVC)
      set(OGG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/ogg/src/ogg-build/$<CONFIGURATION>/ogg.lib")
    else()
      set(OGG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/ogg/src/ogg-build/libogg.a")
    endif()
  endif()
endmacro()

macro(vendored_vorbis)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_VORBIS)
    set(USE_VENDORED_VORBIS true)
    vendored_ogg()

    ExternalProject_Add(vorbis
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_mixer/external/vorbis"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/vorbis/"
      INSTALL_COMMAND ""
      CMAKE_ARGS -DBUILD_SHARED_LIBS=FALSE
      CMAKE_CACHE_ARGS -DOGG_FOUND:STRING=1 -DOGG_INCLUDE_DIR:PATH=${OGG_INCLUDE_DIR} -DOGG_LIBRARY:FILEPATH=${OGG_LIBRARY}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )

    add_dependencies(vorbis ogg)

    set(VORBIS_FOUND)
    set(VORBIS_INCLUDE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_mixer/external/vorbis/include")
    if(MSVC)
      set(VORBIS_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/vorbis/src/vorbis-build/lib/$<CONFIGURATION>/vorbis.lib"
        "${CMAKE_CURRENT_BINARY_DIR}/vorbis/src/vorbis-build/lib/$<CONFIGURATION>/vorbisenc.lib"
        "${CMAKE_CURRENT_BINARY_DIR}/vorbis/src/vorbis-build/lib/$<CONFIGURATION>/vorbisfile.lib" ${OGG_LIBRARY})
    else()
      set(VORBIS_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/vorbis/src/vorbis-build/lib/libvorbis.a"
        "${CMAKE_CURRENT_BINARY_DIR}/vorbis/src/vorbis-build/lib/libvorbisenc.a"
        "${CMAKE_CURRENT_BINARY_DIR}/vorbis/src/vorbis-build/lib/libvorbisfile.a" ${OGG_LIBRARY})
    endif()
  endif()
endmacro()

macro(vendored_oggvorbis)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_OGGVORBIS)
    set(USE_VENDORED_OGGVORBIS true)
    vendored_ogg()
    vendored_vorbis()

    set(OGGVORBIS_FOUND TRUE)
    set(OGGVORBIS_INCLUDE_DIR ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(OGGVORBIS_LIBRARIES ${OGG_LIBRARY} ${VORBIS_LIBRARY})
  endif()
endmacro()

macro(vendored_jpeg)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_JPEG)
    set(USE_VENDORED_JPEG true)

    file(GLOB JPEG_HEADERS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/jpeg/*.h")

    ExternalProject_Add(jpeg
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/external/jpeg"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/jpeg/"
      INSTALL_COMMAND cmake -E copy ${JPEG_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/jpeg/src/jpeg-build/"
      CMAKE_ARGS -DBUILD_STATIC=ON
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )

    set(JPEG_FOUND TRUE)
    set(JPEG_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/jpeg/src/jpeg-build/")
    if(MSVC)
      set(JPEG_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/jpeg/src/jpeg-build/$<CONFIGURATION>/libjpeg.lib")
    else()
      set(JPEG_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/jpeg/src/jpeg-build/libjpeg.a")
    endif()
  endif()
endmacro()

macro(vendored_lcms)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_LCMS)
    set(USE_VENDORED_LCMS true)

    ExternalProject_Add(lcms
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/lcms"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/lcms/"
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_STATIC_LIBS:BOOL=ON
      INSTALL_COMMAND ""
    )

    set(LCMS_FOUND TRUE)
    set(LCMS_INCLUDE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/lcms/include/")
    if(MSVC)
      set(LCMS_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lcms/src/lcms-build/$<CONFIGURATION>/lcms.lib")
    else()
      set(LCMS_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/lcms/src/lcms-build/liblcms.a")
    endif()
  endif()
endmacro()

macro(vendored_mng)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_MNG)
    set(USE_VENDORED_MNG true)
    vendored_zlib()
    vendored_jpeg()
    vendored_lcms()

    ExternalProject_Add(mng
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/libmng"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/mng/"
      INSTALL_COMMAND ""
      CMAKE_CACHE_ARGS -DZLIB_FOUND:STRING=1 -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR} -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARIES}
      -DJPEG_FOUND:STRING=1 -DJPEG_INCLUDE_DIR:PATH=${JPEG_INCLUDE_DIR} -DJPEG_LIBRARY:FILEPATH=${JPEG_LIBRARIES}
      -DLCMS_FOUND:STRING=1 -DLCMS_INCLUDE_DIR:PATH=${LCMS_INCLUDE_DIR} -DLCMS_LIBRARY:FILEPATH=${LCMS_LIBRARIES}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
      -DBUILD_SHARED_LIBS:BOOL=OFF -DBUILD_STATIC_LIBS:BOOL=ON
    )

    add_dependencies(mng zlib jpeg lcms)

    set(MNG_FOUND TRUE)
    set(MNG_DEFINITIONS -DMNG_NO_INCLUDE_JNG) # see patch
    set(MNG_INCLUDE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/libmng/")
    if(MSVC)
      set(MNG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/mng/src/mng-build/$<CONFIGURATION>/mng.lib" ${ZLIB_LIBRARIES} ${LCMS_LIBRARIES})
    else()
      set(MNG_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/mng/src/mng-build/libmng.a" ${ZLIB_LIBRARIES} ${LCMS_LIBRARIES})
    endif()
  endif()
endmacro()

macro(vendored_theora)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_THEORA)
    set(USE_VENDORED_THEORA true)
    vendored_ogg()
    vendored_vorbis()

    # theora doesn't currently have a cmakelist.txt
    set(THEORA_SRC_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/theora/")
    add_library(theora STATIC "${THEORA_SRC_DIR}/lib/apiwrapper.c"
      "${THEORA_SRC_DIR}/lib/bitpack.c" "${THEORA_SRC_DIR}/lib/dequant.c"
      "${THEORA_SRC_DIR}/lib/fragment.c" "${THEORA_SRC_DIR}/lib/idct.c"
      "${THEORA_SRC_DIR}/lib/info.c" "${THEORA_SRC_DIR}/lib/internal.c"
      "${THEORA_SRC_DIR}/lib/state.c" "${THEORA_SRC_DIR}/lib/quant.c"
      "${THEORA_SRC_DIR}/lib/decapiwrapper.c" "${THEORA_SRC_DIR}/lib/decinfo.c"
      "${THEORA_SRC_DIR}/lib/decode.c" "${THEORA_SRC_DIR}/lib/huffdec.c")
    add_dependencies(theora ogg vorbis)
    target_include_directories(theora PRIVATE ${OGG_INCLUDE_DIR} "${THEORA_SRC_DIR}/include")
    if(NOT MSVC)
      target_compile_options(theora PRIVATE -Wno-shift-negative-value -Wno-shift-op-parentheses)
    endif()
    set(THEORA_FOUND TRUE)
    set(THEORA_INCLUDE_DIR ${THEORA_SRC_DIR}/include)
    if(MSVC)
      set(THEORA_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/theora.lib" ${OGG_LIBRARY})
    else()
      set(THEORA_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/libtheora.a" ${OGG_LIBRARY})
    endif()
  endif()
endmacro()

macro(vendored_sdl)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_SDL)
    set(USE_VENDORED_SDL true)

    file(GLOB SDL_HEADERS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL/include/*.h")
    if (MSVC)
      set(EXTRA_SDL_CMAKE_ARGS -DSDL_LIBC=ON)
    endif()
    ExternalProject_Add(SDL2
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/SDL2/"
      INSTALL_COMMAND cmake -E copy ${SDL_HEADERS} "${CMAKE_CURRENT_BINARY_DIR}/SDL2/src/SDL2-build/include"
      CMAKE_ARGS -DBUILD_SHARED_LIBS=OFF -DSDL_STATIC_PIC=ON -DSDL_STATIC=ON ${EXTRA_SDL_CMAKE_ARGS}
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )
    set(SDL2_FOUND TRUE)
    set(SDL2_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/SDL2/src/SDL2-build/include")
    if(MSVC)
      set(SDL2_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/SDL2/src/SDL2-build/$<CONFIGURATION>/SDL2$<$<CONFIG:Debug>:d>.lib")
      set(SDL2main_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/SDL2/src/SDL2-build/$<CONFIGURATION>/SDL2main$<$<CONFIG:Debug>:d>.lib")
    else()
      set(SDL2_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/SDL2/src/SDL2-build/libSDL2.a")
      set(SDL2main_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/SDL2/src/SDL2-build/libSDL2main.a")
    endif()
  endif()
endmacro()

macro(vendored_sdl_image)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_SDL_IMAGE)
    set(USE_VENDORED_SDL_IMAGE)
    vendored_sdl()
    vendored_jpeg()
    vendored_png()

    ExternalProject_Add(SDL2_image
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/SDL2_image/"
      INSTALL_COMMAND ""
      CMAKE_ARGS -DSDL2IMAGE_VENDORED=OFF -DBUILD_SHARED_LIBS=OFF -DSDL2IMAGE_DEPS_SHARED=OFF -DSDL2IMAGE_SAMPLES=OFF
      -DSDL2IMAGE_BACKEND_STB=OFF
      CMAKE_CACHE_ARGS -DSDL2_FOUND:STRING=1 -DSDL2_INCLUDE_DIR:PATH=${SDL2_INCLUDE_DIR}
      -DSDL2_LIBRARY:FILEPATH=${SDL2_LIBRARY} -DSDL2_MAIN_LIBRARY:FILEPATH=${SDL2main_LIBRARY}
      -DSDL2_DIR:PATH="${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL"
      -DSDL2IMAGE_BACKEND_IMAGEIO:BOOL=OFF
      -DSDL2IMAGE_JPG_VENDORED:STRING=ON
      -DJPEG_FOUND:STRING=1 -DJPEG_INCLUDE_DIR:PATH=${JPEG_INCLUDE_DIR} -DJPEG_LIBRARY:FILEPATH=${JPEG_LIBRARIES}
      -DSDL2IMAGE_PNG_VENDORED:STRING=OFF
      -DPNG_FOUND:STRING=1 -DPNG_INCLUDE_DIR:PATH=${PNG_INCLUDE_DIR} -DPNG_PNG_INCLUDE_DIR:PATH=${PNG_INCLUDE_DIR} -DPNG_LIBRARY:FILEPATH=${PNG_LIBRARY}
      -DZLIB_FOUND:STRING=1 -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR} -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARIES}
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )

    add_dependencies(SDL2_image SDL2 jpeg png)

    set(SDL2_IMAGE_FOUND TRUE)
    set(SDL2_IMAGE_INCLUDE_DIRS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_image/")
    if(MSVC)
      set(SDL2_IMAGE_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/SDL2_image/src/SDL2_image-build/$<CONFIGURATION>/SDL2_image-static$<$<CONFIG:Debug>:d>.lib" ${JPEG_LIBRARIES})
    else()
      set(SDL2_IMAGE_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/SDL2_image/src/SDL2_image-build/libSDL2_image.a" ${JPEG_LIBRARIES})
    endif()
  endif()
endmacro()

macro(vendored_sdl_mixer)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_SDL_MIXER)
    set(USE_VENDORED_SDL_MIXER true)
    vendored_sdl()

    ExternalProject_Add(SDL2_mixer
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_mixer"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer/"
      INSTALL_COMMAND ""
      CMAKE_ARGS -DSDL2MIXER_VENDORED=ON -DBUILD_SHARED_LIBS=OFF -DSDL2MIXER_DEPS_SHARED=OFF
      -DSDL2MIXER_SAMPLES=OFF
      -DSDL2MIXER_FLAC=OFF
      -DSDL2MIXER_OPUS=OFF
      -DSDL2MIXER_MOD=OFF
      -DSDL2MIXER_MIDI=ON
      -DSDL2MIXER_VORBIS=VORBISFILE -DSDL2MIXER_VORBIS_VORBISFILE_SHARED=OFF
      CMAKE_CACHE_ARGS -DSDL2_FOUND:STRING=1 -DSDL2_INCLUDE_DIR:PATH=${SDL2_INCLUDE_DIR}
      -DSDL2_LIBRARY:FILEPATH=${SDL2_LIBRARY} -DSDL2_MAIN_LIBRARY:FILEPATH=${SDL2main_LIBRARY}
      -DSDL2_DIR:PATH="${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL"
      -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
    )
    add_dependencies(SDL2_mixer SDL2)
    set(SDL2_MIXER_FOUND TRUE)
    set(SDL2_MIXER_INCLUDE_DIRS "${CMAKE_CURRENT_MACRO_LIST_DIR}/SDL_mixer/include")
    if(MSVC)
      set(SDL2_MIXER_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer/src/SDL2_mixer-build/$<CONFIGURATION>/SDL2_mixer-static$<$<CONFIG:Debug>:d>.lib")
    else()
      set(SDL2_MIXER_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/SDL2_mixer/src/SDL2_mixer-build/libSDL2_mixer.a")
    endif()
  endif()
endmacro()

macro(vendored_unsf)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_UNSF)
    set(USE_VENDORED_UNSF true)

    ExternalProject_Add(unsf
      SOURCE_DIR "${CMAKE_CURRENT_MACRO_LIST_DIR}/unsf"
      PREFIX "${CMAKE_CURRENT_BINARY_DIR}/unsf/"
      CMAKE_CACHE_ARGS -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
      INSTALL_COMMAND ""
    )
  endif()
endmacro()

macro(vendored_freepats)
  include(ExternalProject)
  if(NOT DEFINED USE_VENDORED_FREEPATS)
    set(USE_VENDORED_FREEPATS true)

    ExternalProject_Add(freepats
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/freepats-download"
      SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/freepats-source"
      URL "https://freepats.zenvoid.org/freepats-20060219.tar.xz"
      CONFIGURE_COMMAND ""
      BUILD_COMMAND cmake -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/freepats-source" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/freepats"
      INSTALL_COMMAND "")
  endif()
endmacro()
