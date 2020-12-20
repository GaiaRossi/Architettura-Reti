client: rgrepClient.c
	gcc -o client rgrepClient.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2

server: rgrepServer.c
	gcc -o server rgrepServer.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2

server_rxb : rgrep_rxb_server.c utils.o rxb.o
	gcc -o server_rxb rgrep_rxb_server.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

client_rxb : rgrep_rxb_client.c utils.o rxb.o
	gcc -o client_rxb rgrep_rxb_client.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

utils.o: utils.c
	gcc -c utils.c

rxb.o: rxb.c
	gcc -c rxb.c

clean: server client
	rm server client
