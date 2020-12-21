server : coffee_machines_server.c utils.o
	gcc -o server coffee_machines_server.c utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

coffee_machines : coffee_machines_client.c utils.o
	gcc -o coffee_machines coffee_machines_client.c utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -DUSE_LIBUNISTRING=1 -lunistring

utils.o: utils.c
	gcc -c utils.c

clean : 
	rm coffee_machines server *.o