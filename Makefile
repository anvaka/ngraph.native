CFLAGS ?= -std=c++11 -g -Wall -Wextra -Wpedantic -Iinclude #-fopenmp

all: bin/layout \

clean: 
	rm -f obj/* bin/*

#---------------------------
# Object Files
#---------------------------
obj/layout.o: src/layout.cpp include/layout.h include/quadTree.h include/Random.h include/primitives.h
	g++ $(CFLAGS) -c -o obj/layout.o src/layout.cpp

obj/quadTree.o: src/quadTree.cpp include/quadTree.h include/Random.h include/primitives.h
	g++ $(CFLAGS) -c -o obj/quadTree.o src/quadTree.cpp

obj/main.o: src/main.cpp include/layout.h include/quadTree.h include/Random.h include/primitives.h
	g++ $(CFLAGS) -c -o obj/main.o src/main.cpp

#---------------------------
# Executables
#---------------------------
bin/layout: obj/layout.o obj/quadTree.o obj/main.o include/layout.h include/primitives.h include/quadTree.h include/Random.h
	g++ $(CFLAGS) -o bin/layout++ obj/quadTree.o obj/layout.o obj/main.o
