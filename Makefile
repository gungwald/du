.PHONY: all src

all: src-build src

src:
	$(MAKE) -C src/main/c
	
src-build:
	$(MAKE) -C src/build/c
	
#test:
#	$(MAKE) -C test/main/c
	
clean:
	$(MAKE) -C src/build/c clean
	$(MAKE) -C src/main/c clean
#	$(MAKE) -C test/main/c clean
