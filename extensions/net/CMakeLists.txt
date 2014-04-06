if(EMSCRIPTEN)
    set(SOURCES main.cpp)
else()
    set(SOURCES main.cpp websocket.c)
endif()
add_library(net SHARED ${SOURCES})

if(NOT DEFINED EMSCRIPTEN)
    target_link_libraries(net resolv)
    target_link_libraries(net ssl)
endif()

install(TARGETS net DESTINATION /usr/share/drystal)
