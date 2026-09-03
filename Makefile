# ==============================================================
#  Mini-Golf — Cross-platform Makefile (macOS + Windows/MinGW)
# ==============================================================

TARGET  := golf
SRC     := $(wildcard src/*.c)
OBJ     := $(SRC:src/%.c=build/%.o)
CC      := gcc
CFLAGS  := -std=c99 -Wall -Wextra -g

# --------------------------------------------------------------
# Detect the OS
#   - On Windows, the OS env var is set to "Windows_NT" by default.
#   - Everywhere else (macOS, Linux) we ask uname.
# --------------------------------------------------------------
ifeq ($(OS),Windows_NT)
    PLATFORM := WINDOWS
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        PLATFORM := MACOS
    else
        PLATFORM := LINUX
    endif
endif

# --------------------------------------------------------------
# Platform-specific flags
# --------------------------------------------------------------
ifeq ($(PLATFORM),MACOS)
    # Homebrew raylib via pkg-config (Apple Silicon or Intel, either works)
    CFLAGS  += $(shell pkg-config --cflags raylib)
    LDFLAGS := $(shell pkg-config --libs raylib) -lm
endif

ifeq ($(PLATFORM),WINDOWS)
    # Assumes raylib installed via MSYS2/MinGW (pacman -S mingw-w64-x86_64-raylib)
    # pkg-config ships with that package too, so this stays consistent with macOS.
    CFLAGS  += $(shell pkg-config --cflags raylib)
    LDFLAGS := $(shell pkg-config --libs raylib) -lopengl32 -lgdi32 -lwinmm
    TARGET  := $(TARGET).exe
    MKDIR   := if not exist build mkdir build
    RM      := if exist build rmdir /s /q build & if exist $(TARGET) del /q $(TARGET)
    EXE     := $(subst /,\,$(TARGET))
else
    MKDIR   := mkdir -p build
    RM      := rm -rf build $(TARGET)
    EXE     := ./$(TARGET)
endif

ifeq ($(PLATFORM),LINUX)
    CFLAGS  += $(shell pkg-config --cflags raylib)
    LDFLAGS := $(shell pkg-config --libs raylib) -lm
endif

# --------------------------------------------------------------
# Rules
# --------------------------------------------------------------
.PHONY: all clean run info

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	$(MKDIR)

run: $(TARGET)
	$(EXE)

clean:
	$(RM)

info:
	@echo "Platform : $(PLATFORM)"
	@echo "CC       : $(CC)"
	@echo "CFLAGS   : $(CFLAGS)"
	@echo "LDFLAGS  : $(LDFLAGS)"