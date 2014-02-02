add_library(truetype SHARED main.cpp font.cpp parser.cpp)
install(TARGETS truetype DESTINATION /usr/share/drystal)
