CXXFLAGS=-g -pedantic -Wall
LDFLAGS=-lGL -lglut -lGLEW -lm
CC=g++ # Or make will try to link using cc

all: main

main: main.o renderer.o world.o stage.o geometry.o player.o hook.o


.PHONY: all

