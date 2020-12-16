server : lista_articoli_server.c utils.o rxb.o
	gcc -o server lista_articoli_server.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

client : lista_articoli_client.c utils.o rxb.o
	gcc -o client lista_articoli_client.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

utils.o: utils.c
	gcc -c utils.c

rxb.o: rxb.c
	gcc -c rxb.c

client_csx: lista_articoli_client_canonicals.c utils.o
	gcc -o client_csx lista_articoli_client_canonicals.c utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

server_csx: lista_articoli_server_canonicals.c utils.o
	gcc -o server_csx lista_articoli_server_canonicals.c utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

clean : 
	rm client server client_csx server_csx *.o
