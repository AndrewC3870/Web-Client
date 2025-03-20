CC=g++ -std=c++11  -Wall -Wextra

client: client.cpp  helpers.cpp buffer.cpp
	$(CC) -o client client.cpp helpers.cpp buffer.cpp -Wall

run: client
	./client

clean:
	rm -f *.o client
