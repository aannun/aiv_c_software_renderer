CC=clang
CFLAGS=-Wall -Wno-pragma-pack -ISDL2-2.0.9\include
LDFLAGS=-lSDL2
BINARY=rasterize
BINARY_TESTS=rasterize_tests

ifeq ($(OS),Windows_NT)
	BINARY:=$(BINARY).exe
	BINARY_TESTS:=$(BINARY_TESTS).exe
endif

rasterize: main.o aiv_math.o aiv_rasterizer.o parser.o
	$(CC) -o $(BINARY) $(LDFLAGS) $^

main.o: main.c
	$(CC) -c -o $@ $(CFLAGS) $^

aiv_math.o: aiv_math.c aiv_math.h
	$(CC) -c -o $@ $(CFLAGS) $<

aiv_rasterizer.o: aiv_rasterizer.c aiv_rasterizer.h
	$(CC) -c -o $@ $(CFLAGS) $<

aiv_renderer.o: aiv_renderer.c aiv_renderer.h
	$(CC) -c -o $@ $(CFLAGS) $<

parser.o: parser.c parser.h
	$(CC) -c -o $@ $(CFLAGS) $<

tests.o: tests.c
	$(CC) -c -o $@ $(CFLAGS) $^

test: tests.o tetris.o
	$(CC) -o $(BINARY_TESTS) $(LDFLAGS) $^
	$(BINARY_TESTS)