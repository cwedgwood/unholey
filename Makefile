CFLAGS+=-Wall

default: unholey
	./unholey

clean:
	rm -f *~ *.o unholey

.PHONY: default clean
