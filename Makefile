.PHONY: all src

all: src

src:
	$(MAKE) -C src/main/c
	
test:
	$(MAKE) -C test/main/c
	
clean:
	$(MAKE) -C src/main/c clean
	$(MAKE) -C test/main/c clean
