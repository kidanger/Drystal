NATIVE_CMAKE_BUILD_TYPE ?= Debug
WEB_CMAKE_BUILD_TYPE ?= Release
BUILD_DIRS = $(dir $(wildcard ./build-*/))
SUBDIRSCLEAN = $(addsuffix .clean,$(BUILD_DIRS))
EMSCRIPTEN = /usr/lib/emscripten/

CMAKE_FLAGS ?=

# from the Makefile of neovim
BUILD_TYPE ?= $(shell (type ninja > /dev/null 2>&1 && echo "Ninja") || \
    echo "Unix Makefiles")
ifeq (,$(BUILD_TOOL))
  ifeq (Ninja,$(BUILD_TYPE))
    ifneq ($(shell cmake --help 2>/dev/null | grep Ninja),)
      BUILD_TOOL := ninja
    else
      # User's version of CMake doesn't support Ninja
      BUILD_TOOL = $(MAKE)
      BUILD_TYPE := Unix Makefiles
    endif
  else
    BUILD_TOOL = $(MAKE)
  endif
endif

all: native

native:
	mkdir -p build-native-$(NATIVE_CMAKE_BUILD_TYPE)
	cd build-native-$(NATIVE_CMAKE_BUILD_TYPE) && cmake .. -G '$(BUILD_TYPE)' -DCMAKE_BUILD_TYPE=$(NATIVE_CMAKE_BUILD_TYPE) $(CMAKE_FLAGS)
	+$(BUILD_TOOL) -C build-native-$(NATIVE_CMAKE_BUILD_TYPE)

install-native: native
	+$(BUILD_TOOL) -C build-native-$(NATIVE_CMAKE_BUILD_TYPE) install

install: install-native

web:
	mkdir -p build-web-$(WEB_CMAKE_BUILD_TYPE)
	cd build-web-$(WEB_CMAKE_BUILD_TYPE) && cmake .. -G '$(BUILD_TYPE)' -DCMAKE_BUILD_TYPE=$(WEB_CMAKE_BUILD_TYPE) -DCMAKE_TOOLCHAIN_FILE=$(EMSCRIPTEN)/cmake/Modules/Platform/Emscripten.cmake $(CMAKE_FLAGS)
	+$(BUILD_TOOL) -C build-web-$(WEB_CMAKE_BUILD_TYPE)

clean: $(SUBDIRSCLEAN)

$(SUBDIRSCLEAN):
	$(BUILD_TOOL) -C $(basename $@) clean || true

distclean: clean
	rm -rf $(BUILD_DIRS)

.PHONY: clean distclean $(SUBDIRSCLEAN) install native web install-native
