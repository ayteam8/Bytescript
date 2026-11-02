CC = gcc
CFLAGS = -I./ -I../ -I./Values -I../false_windows -ldl -lm -D_GNU_SOURCE -rdynamic -fPIC
AR = ar
ARFLAGS = rcs
SRC := $(shell find . -name '*.c')
MAIN_SRC := $(wildcard *.c)
LIB_SRC := $(filter-out $(MAIN_SRC), $(SRC))
MAPS_SRC := $(wildcard Globals/maps/*.bs)

MAIN_OBJ := $(patsubst %.c, ../build/%.o, $(notdir $(MAIN_SRC)))
LIB_OBJ  := $(patsubst %.c, ../build/%.o, $(notdir $(LIB_SRC)))

OBJ    		:= $(MAIN_OBJ) $(LIB_OBJ)
MAPS   		:= $(MAPS_SRC:%.bs=../dist/stdlib/%.bs)
TARGET 		:= $(patsubst %.c, ../dist/%, $(MAIN_SRC))
C_API		:= ../dist/bytescript.h ../dist/Values/bytev.h
LIBRARIES := ../dist/libbytescript.a ../dist/bytescript.so

main : $(TARGET) $(MAPS) $(C_API)

libs: $(LIBRARIES)

all: CFLAGS += -O3 -march=native
all: main libs

o_o : CFLAGS += -O0 -march=native
o_o : main

o_one : CFLAGS += -O1 -march=native
o_one : main

o_two : CFLAGS += -O2 -march=native
o_two : main

o_three : CFLAGS += -O3 -march=native
o_three : main

byte : CFLAGS += -O2 -march=native
byte : main

release : CFLAGS += -O3
release : main

debug : CFLAGS += -O0 -g
debug : main

../dist/%: ../build/%.o $(LIB_OBJ)  | ../dist
	$(CC) $(CFLAGS) $^ -o $@

# it turn's linux doesn't need the .lib, xD
#../dist/bytescript.lib: ../dist/bytescript.dll | ../dist
#	dlltool -d bytescript.def -l $@

../dist/bytescript.so: $(LIB_OBJ) | ../dist
	$(CC) $(CFLAGS) $^ -shared -o $@

../build/%.o: ../build
	$(CC) $(CFLAGS) -c $(shell find . -name $*.c) -o $@

../dist/%.h: ../dist
	cp $*.h ../dist

../dist/stdlib/%.bs: ../dist/stdlib
	cp $*.bs ../dist/stdlib

../build :
	mkdir -p ../build

../dist :
	mkdir -p ../dist

../dist/stdlib:
	mkdir -p ../dist/stdlib

dis : CFLAGS += -O2
dis : clean all
	objdump -d ../build/dised.o > ../dist/dised.txt

../dist/libbytescript.a: $(LIB_OBJ)
	$(AR) $(ARFLAGS) ../dist/libbytescript.a $(LIB_OBJ)
