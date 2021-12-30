.PHONY: all src src-build release

all: src-build src

release: src-build
	$(MAKE) -C src/main/c release
	
src:
	$(MAKE) -C src/main/c
	
src-build:
	$(MAKE) -C src/build/c
	
test:
	$(MAKE) -C test/main/c
	
clean:
	$(MAKE) -C src/build/c clean
	$(MAKE) -C src/main/c clean
	$(MAKE) -C src/test/c clean
