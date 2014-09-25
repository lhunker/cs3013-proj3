LIB=-lpthread
CCPP=g++

all: addem

addem: addem.C mailboxs.C
	$(CCPP) addem.C mailboxs.C -o addem $(LIB)

clean:
	rm addem
