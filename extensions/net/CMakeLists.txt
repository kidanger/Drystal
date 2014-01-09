add_library(net SHARED main.cpp network.cpp)
install(TARGETS net DESTINATION /usr/share/drystal)
