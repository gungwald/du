.PHONY: all du release

all: du

release:
	$(MAKE) -C du release
	
du:
	$(MAKE) -C du
	
test:
	$(MAKE) -C test
	
clean:
	$(MAKE) -C du clean
	$(MAKE) -C test clean

