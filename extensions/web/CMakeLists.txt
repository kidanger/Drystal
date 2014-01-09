add_library(web SHARED main.cpp)
install(TARGETS web DESTINATION /usr/share/drystal)
