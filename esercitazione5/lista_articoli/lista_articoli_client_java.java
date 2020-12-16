import java.net.*;
import java.io.*;

public class lista_articoli_client_java{
    public static void main(String[] args) {
        /* java lista_articoli_client_java localhost porta */
        /**controllo parametri */
        if(args.length != 2){
            System.err.println("Uso: java lista_articoli_client_java localhost porta");
            System.exit(1);
        }

        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));

            String input, daInviare, risposta;

            System.out.println("Inserisci la email:");
            while(!(input = tastiera.readLine()).equals("fine")){
                daInviare = input + "\n";

                System.out.println("Inserisci la password:");
                input = tastiera.readLine();

                daInviare = daInviare + input + "\n";

                System.out.println("Inserisci la rivista:");
                input = tastiera.readLine();

                daInviare = daInviare + input + "\n";

                /**inviare al server */
                netOut.write(daInviare, 0, daInviare.length());
                netOut.flush();

                while(!(risposta = netIn.readLine()).equals("finerichiesta")){
                    System.out.println(risposta);
                }

                System.out.println("Inserisci la email:");
            }

            socket.close();
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}