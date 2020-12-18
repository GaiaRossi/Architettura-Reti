client: rgrepClient.c
	gcc -o client rgrepClient.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2

server: rgrepServer.c
	gcc -o server rgrepServer.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2

clean: server client
	rm server client
