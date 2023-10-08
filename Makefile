# Dependency configurations
DEPENDENCIES	?= libgtk-4-dev clang-tidy bear blueprint-compiler
GTKFLAGS   		!= pkg-config --cflags gtk4
GTKLIBS    		!= pkg-config --libs gtk4
GLIB			!= pkg-config --variable=glib_compile_resources gio-2.0
BLUEPRINT		:= blueprint-compiler compile

# Compile options
CFLAGS			?= -std=c17 -lc
INCLUDES		?= -Iinc -Ilib/str
SOURCES 		?= build/*.c src/*.c lib/str/str.c
BUILD_DIR		?= build/
TARGET			?= nordi
OUT				:= $(BUILD_DIR)$(TARGET)
RESOURCE_DIR	?= res/
GLIBFLAGS		?= --sourcedir=$(BUILD_DIR) --generate-source
RESOURCE		?= $(RESOURCE_DIR)$(TARGET).gresource.xml
TESTS			?= lib/str/str.c test/*.c test/munit/munit.c
TARGETTEST		?= $(TARGET)-unittest
OUTTEST			:= $(OUT)-unittest

# Choose compiler
ifneq ($(shell which gcc),)
CC 				:= gcc
else
CC 				:= clang
endif

# Add bear to compiler in order to build compilation commands
ifneq ($(shell which bear),)
CC 				:= bear -- $(CC)
endif

# Linting options
TIDYFLAGS		?= --quiet -p $(BUILD_DIR)

# Install options
ifneq ($(SUDO_USER),)
USER 			:= $(SUDO_USER)
endif

INSTALL_DIR		?= /usr/local/bin/
APP_DIR			?= /home/$(USER)/.local/share/applications/
ICON_DIR		?= /usr/share/icons/

# Package options
DPKG			?= dpkg-deb --build
PACKAGE_DIR		?= $(BUILD_DIR)package/$(TARGET)/
CONTROL_DIR		?= $(PACKAGE_DIR)DEBIAN/
DESKTOP_DIR		?= $(PACKAGE_DIR)usr/share/applications/

# Make options
.DEFAULT_GOAL 	:= build

ifneq ($(tput colors),2)
	COLSTART	?= \033[1;44m
	COLRELEASE	?= \033[1;45m
	COLDEBUG	?= \033[1;43m
	COLEND		?= \033[0m
endif

# display all targets in this Makefile
.PHONY: info
info:
	@echo "make setup		- Installs all package dependencies for $(TARGET)"
	@echo "make build		- Compiles $(TARGET) binary for release mode"
	@echo "make build-debug	- Compiles $(TARGET) binary with debug symbols"
	@echo "make check		- Runs clang-tidy on the binary"
	@echo "make test		- Runs the unit tests"
	@echo "make install		- Installs built $(TARGET) locally"
	@echo "make package		- Builds the $(TARGET).deb Debian package "
	@echo "make clean		- Cleans the build directory"
	@echo "make all		- Same as 'make clean build check test'"
	@echo "make all-debug		- Same as 'make clean build-debug check test'"

# make all
.PHONY: all
all: clean build check test

# make all, debug version
.PHONY: all-debug
all-debug: clean build-debug check test

# install dependencies
.PHONY: setup
setup:
	@echo "$(COLSTART)installing dependencies$(COLEND)"
	@apt-get install $(DEPENDENCIES)
	@git submodule update --init --recursive

# build binary in debug
.PHONY: build-debug
build-debug: CFLAGS += -g -w
build-debug: $(TARGET)
	@echo "$(COLDEBUG) DEBUG $(COLEND) binary at $(BUILD_DIR)$(TARGET)"

# build binary in release
.PHONY: build
build: CFLAGS += -O2 -Wall -Wextra
build: $(TARGET)
	@echo "$(COLRELEASE) RELEASE $(COLEND) binary at $(BUILD_DIR)$(TARGET)"

# linting
.PHONY: check
check:
	@echo "$(COLSTART)checking source formats$(COLEND)"
	@clang-tidy $(TIDYFLAGS) $(SOURCES)

# unit testing
.PHONY: test
test: CFLAGS += -w
test: $(TARGETTEST)
	@echo "$(COLSTART)running unit tests$(COLEND)"
	@$(OUTTEST)

# install locally
.PHONY: install
install:
	@echo "$(COLSTART)installing binary on $(INSTALL_DIR)$(TARGET)$(COLEND)"
	@cp $(OUT) $(INSTALL_DIR)
	@chmod +x $(INSTALL_DIR)$(TARGET)
	@echo "$(COLSTART)installing icon on $(ICON_DIR)$(COLEND)"
	@cp -r $(RESOURCE_DIR)hicolor $(ICON_DIR)
	@echo "$(COLSTART)installing .desktop on $(APP_DIR)$(TARGET).desktop$(COLEND)"
	@cp $(RESOURCE_DIR)$(TARGET).desktop $(APP_DIR)
	@chmod +x $(APP_DIR)$(TARGET).desktop
	@gtk-update-icon-cache -f -t $(ICON_DIR)hicolor

# create debian package
.PHONY: package
package: build
	@echo "$(COLSTART)creating install package$(INSTALL_DIR)$(TARGET)$(COLEND)"
	@mkdir -p $(PACKAGE_DIR)$(INSTALL_DIR) $(PACKAGE_DIR)$(ICON_DIR) $(CONTROL_DIR) $(DESKTOP_DIR)
	@cp $(OUT) $(PACKAGE_DIR)$(INSTALL_DIR)
	@cp -r $(RESOURCE_DIR)hicolor $(PACKAGE_DIR)$(ICON_DIR)
	@cp $(RESOURCE_DIR)control $(CONTROL_DIR)
	@cp $(RESOURCE_DIR)$(TARGET).desktop $(DESKTOP_DIR)
	@$(DPKG) $(PACKAGE_DIR)
	@echo "$(COLSTART)nordi.deb checksums$(COLEND)"
	@echo "SHA256: $(shell sha256sum $(BUILD_DIR)package/$(TARGET).deb)"
	@echo "SHA512: $(shell sha512sum $(BUILD_DIR)package/$(TARGET).deb)"

# clear build directory
.PHONY: clean
clean:
	@echo "$(COLSTART)cleaning $(TARGET) build files$(COLEND)"
	-@rm -rv $(BUILD_DIR)

# Unit tests binary
$(TARGETTEST):
	@echo "$(COLSTART)building $(TARGETTEST)$(COLEND)"
	-@mkdir -pv $(BUILD_DIR)
	@$(CC) $(GTKFLAGS) $(CFLAGS) $(TESTS) $(INCLUDES) $(GTKLIBS) -o $(OUTTEST)

# Binary target file
$(TARGET): resources.c
	@echo "$(COLSTART)building $(TARGET)$(COLEND)"
	-@mkdir -pv $(BUILD_DIR)
	@$(CC) $(GTKFLAGS) $(CFLAGS) $(SOURCES) $(INCLUDES) $(GTKLIBS) -o $(OUT)

# Resource file
resources.c:
	@echo "$(COLSTART)building resources$(COLEND)"
	-@mkdir -pv $(BUILD_DIR)
	@$(BLUEPRINT) $(RESOURCE_DIR)$(TARGET).blp > $(BUILD_DIR)$(TARGET).ui
	@echo "generated blueprint $(BUILD_DIR)$(TARGET).ui"
	@$(GLIB) $(RESOURCE) $(GLIBFLAGS) --target=$(BUILD_DIR)$@
	@echo "compiled resources to $(BUILD_DIR)$@"
