bin: btree

btree:
	g++ disk.cpp btree.cpp -o btree 

all: bin

clean:
	rm -f btree *.bin

test: bin
	rm -f data.bin
	./btree && ./btree
