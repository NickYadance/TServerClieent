yzmond: server.o main.o
	g++ -o yzmond server.o main.o -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient

server.o: server.cpp server.h
	g++ -c server.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient

main.o: main.cpp server.h
	g++ -c main.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -f *.o tserver log