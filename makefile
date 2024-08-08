CFLAGS=-Wall -Wextra -pedantic -std=c99 -O2
IMAGE=vm.hex

.PHONY: run clean

run: vm ${IMAGE}
	./vm ${IMAGE}

vm.hex: vm.fth
	gforth vm.fth

vm: vm.c

clean:
	git clean -dffx
