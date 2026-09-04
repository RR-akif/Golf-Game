# ==============================================================
#  Mini-Golf - Cross-platform Makefile (macOS + Windows + Linux)
# ==============================================================

TARGET  := golf
CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -g
BUILD   := build

# --------------------------------------------------------------
# Sources: accept .c files in the project root and/or in src/
# --------------------------------------------------------------
SRC     := $(wildcard *.c) $(wildcard src/*.c)
OBJ     := $(patsubst %.c,$(BUILD)/%.o,$(SRC))
DEPS    := $(OBJ:.o=.d)
CFLAGS  += -MMD -MP

# --------------------------------------------------------------
# Detect the OS and the shell we are running under.
#   uname is present under macOS, Linux, MSYS2/MinGW and Git Bash.
#   If it is missing we are being run by cmd.exe (native mingw32-make).
# --------------------------------------------------------------
UNAME_S := $(shell uname -s 2>/dev/null)

ifeq ($(UNAME_S),)
    PLATFORM  := WINDOWS
    POSIX_SH  := 0
else ifeq ($(OS),Windows_NT)
    PLATFORM  := WINDOWS
    POSIX_SH  := 1
else ifeq ($(UNAME_S),Darwin)
    PLATFORM  := MACOS
    POSIX_SH  := 1
else
    PLATFORM  := LINUX
    POSIX_SH  := 1
endif

# --------------------------------------------------------------
# raylib location
#   1. A raylib/ folder shipped with the project wins for headers.
#   2. Libraries come from raylib/lib when that folder has a build for
#      this platform, otherwise from pkg-config / the system.
# --------------------------------------------------------------
LOCAL_INC := $(wildcard raylib/include/raylib.h)
LOCAL_LIB := $(wildcard raylib/lib/libraylib.a) $(wildcard raylib/lib/libraylib.dylib) $(wildcard raylib/lib/libraylib.so)

ifneq ($(LOCAL_INC),)
    CFLAGS   += -Iraylib/include
endif

# The bundled raylib/lib in this repo is a MinGW build, so only trust it
# on Windows; elsewhere prefer pkg-config and fall back to plain -lraylib.
ifeq ($(PLATFORM),WINDOWS)
    ifneq ($(LOCAL_LIB),)
        RAYLIB_LIBS := -Lraylib/lib -lraylib
    else
        RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)
        RAYLIB_LIBS := $(if $(RAYLIB_LIBS),$(RAYLIB_LIBS),-lraylib)
    endif
else
    PKG_CFLAGS  := $(shell pkg-config --cflags raylib 2>/dev/null)
    RAYLIB_LIBS := $(shell pkg-config --libs raylib 2>/dev/null)
    CFLAGS      += $(PKG_CFLAGS)
    ifeq ($(RAYLIB_LIBS),)
        ifneq ($(LOCAL_LIB),)
            RAYLIB_LIBS := -Lraylib/lib -lraylib
        else
            RAYLIB_LIBS := -lraylib
        endif
    endif
endif

# --------------------------------------------------------------
# Platform-specific link flags / shell commands
# --------------------------------------------------------------
ifeq ($(PLATFORM),MACOS)
    LDFLAGS := $(RAYLIB_LIBS) -lm \
               -framework CoreVideo -framework IOKit -framework Cocoa \
               -framework GLUT -framework OpenGL
endif

ifeq ($(PLATFORM),LINUX)
    LDFLAGS := $(RAYLIB_LIBS) -lm -lpthread -ldl -lrt -lX11
endif

ifeq ($(PLATFORM),WINDOWS)
    LDFLAGS := $(RAYLIB_LIBS) -lopengl32 -lgdi32 -lwinmm
    TARGET  := $(TARGET).exe
endif

ifeq ($(POSIX_SH),1)
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(BUILD) $(TARGET)
    EXE   = ./$(TARGET)
else
    # cmd.exe: backslashes, and "mkdir" fails loudly if the dir exists
    MKDIR = if not exist "$(subst /,\,$(1))" mkdir "$(subst /,\,$(1))"
    RMDIR = if exist $(BUILD) rmdir /s /q $(BUILD)
    RMDIR += & if exist $(subst /,\,$(TARGET)) del /q $(subst /,\,$(TARGET))
    EXE   = $(subst /,\,$(TARGET))
endif

# --------------------------------------------------------------
# Rules
# --------------------------------------------------------------
.PHONY: all clean run info

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

# One pattern rule covers both ./x.c -> build/x.o and src/x.c -> build/src/x.o
$(BUILD)/%.o: %.c
	@$(call MKDIR,$(dir $@))
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	$(EXE)

clean:
	$(RMDIR)

info:
	@echo "Platform : $(PLATFORM)"
	@echo "CC       : $(CC)"
	@echo "SRC      : $(SRC)"
	@echo "CFLAGS   : $(CFLAGS)"
	@echo "LDFLAGS  : $(LDFLAGS)"

-include $(DEPS)
