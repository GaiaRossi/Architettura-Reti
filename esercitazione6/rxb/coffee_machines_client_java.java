import java.io.*;
import java.net.*;

public class coffee_machines_client_java{
    public static void main(String[] args) {
        /**controllo argomenti */
        if(args.length != 2){
            System.err.println("Uso: java coffee_machines_client_java server porta");
            System.exit(1);
        }

        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));

            System.out.println("Inserisci lo username:");
            String username = tastiera.readLine() + "\n";

            if(username.equals("fine")){
                socket.close();
                System.exit(1);
            }

            System.out.println("Inserisci la password:");
            String password = tastiera.readLine() + "\n";

            if(password.equals("fine")){
                socket.close();
                System.exit(1);
            }

            /**invio a server */
            netOut.write(username + password, 0, username.length() + password.length());
            netOut.flush();

            /**leggo ack */
            String ack = netIn.readLine();
            if(ack.equals("OK")){
                System.out.println("Autenticato");
            }
            else{
                System.err.println("Non autenticato");
                socket.close();
                System.exit(1);
            }

            /**leggo categorie */
            while(true){
                System.out.println("Inserisci la categoria:");
                String categoria = tastiera.readLine() + "\n";

                if(categoria.equals("fine\n")){
                    break;
                }

                netOut.write(categoria, 0, categoria.length());
                netOut.flush();

                String risposta;

                while(true){
                    risposta = netIn.readLine();

                    if(risposta.equals("fine_richiesta")){
                        break;
                    }

                    System.out.println(risposta);
                }

            }
            socket.close();
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}