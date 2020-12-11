server_rxb : rconto_corrente_server_rxb.c utils.o rxb.o
	gcc -o server_rxb rconto_corrente_server_rxb.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

client_rxb : rconto_corrente_client_rxb.c utils.o rxb.o
	gcc -o client_rbx rconto_corrente_client_rxb.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

server : rconto_corrente_server.c
	gcc -o server rconto_corrente_server.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

client :  rconto_corrente_client.c
	gcc -o client rconto_corrente_client.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

utils.o: utils.c
	gcc -c utils.c

rxb.o: rxb.c
	gcc -c rxb.c

clean : 
	rm client server client_rxb server_rxb *.o
