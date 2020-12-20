server : rdisponibilita_vini_server.c
	gcc -o server rdisponibilita_vini_server.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -lunistring

client : rdisponibilita_vini_client.c
	gcc -o client rdisponibilita_vini_client.c -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -lunistring

server_rxb : rdisponibilita_vini_rxb_server.c rxb.o utils.o
	gcc -o server_rxb rdisponibilita_vini_rxb_server.c rxb.o utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -lunistring

client_rxb : rdisponibilita_vini_rxb_client.c rxb.o utils.o
	gcc -o client_rxb rdisponibilita_vini_rxb_client.c rxb.o utils.o -std=c17 -g3 -Wall -Werror -pedantic -D_FORTIFY_SOURCE=2 -lunistring

rxb.o : rxb.c
	gcc -c rxb.c

utils.o : utils.c
	gcc -c utils.c

clean : 
	rm client server
