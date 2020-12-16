import java.net.*;
import java.io.*;

public class rstrlen_td_client_java{
    public static void main(String[] args) {
        /**controllo argomenti */
        if(args.length != 2){
            System.err.println("Uso: java rstrlen_td_client_java server porta");
            System.exit(1);
        }

        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));

            String input, output;
            System.out.println("Inserisci una stringa:");
            while(!(input = tastiera.readLine()).equals("fine")){
                netOut.write(input, 0, input.length());
                netOut.newLine();
                netOut.flush();

                while(!(output = netIn.readLine()).equals("fine")){
                    System.out.println(output);
                }

                System.out.println("Inserisci una stringa:");
            }
            socket.close();

        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}