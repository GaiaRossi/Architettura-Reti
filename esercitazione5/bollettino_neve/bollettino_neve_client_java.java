import java.io.*;
import java.net.*;

public class bollettino_neve_client_java{
    public static void main(String[] args) {
        /* java bollettino_neve_clint_java hostname porta */
        /* controllo argomenti */
        if(args.length != 2){
            System.err.println("Uso: java bollettino_neve_clint_java hostname porta");
            System.exit(1);
        }

        /* creo socket e stream */
        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

            String input, daInviare;
            System.out.println("Inserisci la localita da vedere:");
            while(!(input = tastiera.readLine()).equals("fine")){
                daInviare = input;
                System.out.println("Inserisci il numero di localita da vedere:");
                input = tastiera.readLine();
                /* concatenazione stringhe */
                daInviare = daInviare + "\n" + input + "\n";
                netOut.write(daInviare, 0, daInviare.length());
                netOut.flush();

                /* leggo risposta */
                String risposta;
                while(!(risposta = netIn.readLine()).equals("finerichiesta")){
                    System.out.println(risposta);
                }
                System.out.println("Inserisci la localita da vedere:");
            }
            socket.close();
            
        } catch (Exception e) {
            System.out.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}