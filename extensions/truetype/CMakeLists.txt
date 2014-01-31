add_library(truetype SHARED main.cpp font.cpp)
install(TARGETS truetype DESTINATION /usr/share/drystal)
