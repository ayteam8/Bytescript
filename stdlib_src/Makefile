CC = gcc
CFLAGS = -I./ -I../ -I../src -I../dist -I../false_windows -ldl -lm -D_GNU_SOURCE -fPIC -rdynamic

SRC := $(wildcard */*.c)
OBJS := $(SRC:.c=.o)
DLL := $(SRC:%.c=../dist/stdlib/%.so)

OBJ := $(MATH_OBJ:%.o=../build/%.o)
TARGET := $(DLL)

.PRECIOUS: $(OBJ)

main : $(TARGET)

all: CFLAGS += -O3 -march=native
all: main

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

debug : CFLAGS += -O0 -g
debug : main

release : CFLAGS += -O3
release : main

../dist/stdlib/%.so: ../build/%.o ../dist/stdlib/%_map.bs | ../dist/stdlib
	$(CC) $(CFLAGS) -shared ../build/$(notdir $<) -o ../dist/stdlib/$(notdir $@)

../dist/stdlib/%.bs: ../dist/stdlib
	cp $*.bs ../dist/stdlib

../build/%.o: %.c | ../build
	$(CC) $(CFLAGS) -c $< -o ../build/$(notdir $@)

../build:
	mkdir -p ../build

../dist/stdlib:
	mkdir -p ../dist/stdlib



