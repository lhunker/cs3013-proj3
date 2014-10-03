Lukas Hunker
Project 3

To Compile:
make all

To Run:
addem
	./addem threads range
life
	./life threads inputfile generations print input

Addem will add all numbers from 0 to range useing the number of worker threads specified. Threads and range must be positive integers. If threads > range threads will be set to equal range. Life will run the game of life useing threads number of threads. it takes generation 0 from inputfile and runs for a maximum of generations generations. It will terminate early if all organisms die or generations are the same from one to another.