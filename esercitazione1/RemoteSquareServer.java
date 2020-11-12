//scopo esercizio
//creare un app distr che calcoli il quadrato di un numero

import java.io.BufferedReader;
import java.io.*;
import java.net.*;

//java RemoteSquareServer porta
public class RemoteSquareServer{
    public static void main(String[] args) {
        //controllo argomenti
        if(args.length != 1){
            System.err.println("Uso: java RemoteSquareServer porta");
            System.exit(1);
        }

        //creazione socket per ascoltare richieste
        try {
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));
            while(true){
                Socket socket = ss.accept();

                //creazione degli stream
                BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
                BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

                //lettura numero finché ce ne sono
                String letto;
                while((letto = netIn.readLine()) != null){
                    int numero = Integer.parseInt(letto);
                    numero = numero * numero;
                    String daInviare = Integer.valueOf(numero).toString();
                    netOut.write(daInviare);
                    netOut.newLine();
                    netOut.flush();
                }

                //chiusura socket
                socket.close();
            }
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}