ts: client.o main.o
	g++ -o ts client.o main.o

client.o: client.cpp client.h 
	g++ -c client.cpp client.h 

main.o: main.cpp client.h 
	g++ -c main.cpp client.h 

clean: 
	rm -f  *.o *.gch  ts  *.log  *.xls