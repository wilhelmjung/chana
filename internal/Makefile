bin:
	g++ disk.cpp btree.cpp

all: bin

clean:
	rm -f a.out *.bin

test: bin
	rm -f data.bin
	./a.out && ./a.out
