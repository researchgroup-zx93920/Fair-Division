main: main.o helper.o output.o
	g++ main.o helper.o output.o -o main

main.o: main.cpp
	g++ -c main.cpp

helper.o: helper.cpp
	g++ -c helper.cpp

output.o: output.cpp
	g++ -c output.cpp

clean: 
	rm *.o main