# LibXenon/devkitXenon application makefile
ifeq ($(strip $(DEVKITXENON)),)
$(error "Set DEVKITXENON to your devkitXenon installation")
endif

TARGET      := snake360
BUILD       := build
SOURCES     := source
INCLUDES    :=
CFLAGS      := -O2 -Wall -Wextra
CXXFLAGS    := $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS     :=
LIBS        := -lSDL -lxenon -lm

include $(DEVKITXENON)/rules
