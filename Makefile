LIB=-lpthread
CCPP=g++

all: addem life

addem: addem.C mailboxs.C
	$(CCPP) addem.C mailboxs.C -o addem $(LIB)

life: life.C mailboxs.C
	$(CCPP) life.C mailboxs.C -o life $(LIB)

clean:
	-rm addem life 2> /dev/null
