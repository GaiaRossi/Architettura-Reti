server_lpd : rps_lpd_server.c utils.o
	gcc -o server_lpd rps_lpd_server.c utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

client_lpd : rps_lpd_client.c utils.o
	gcc -o client_lpd rps_lpd_client.c utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

server_td : rps_td_server.c utils.o rxb.o
	gcc -o server_td rps_td_server.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

client_td : rps_td_client.c utils.o rxb.o
	gcc -o client_td rps_td_client.c utils.o rxb.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

utils.o: utils.c
	gcc -c utils.c

rxb.o: rxb.c
	gcc -c rxb.c

clean : 
	rm client server
