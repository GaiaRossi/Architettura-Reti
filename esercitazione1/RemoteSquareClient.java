import java.io.*;
import java.net.*;

//scopo esercizio
//creare un client che chieda ad un RemoteSquare server di
//calcolare il quadrato di un numero

public class RemoteSquareClient {
    //java RemoteSquareClient hostname porta
    public static void main(String[] args) {
        //controllo argomenti
        if(args.length != 2){
            System.err.println("Uso: java RemoteSquareClient hostname porta");
            System.exit(1);
        }
        //creazione socket
        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            //creazione stream
            BufferedReader tasiera = new BufferedReader(new InputStreamReader(System.in));

            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

            //leggo i numeri da inviare al server
            String lettoTastiera, ricevuto;
            
            while(!(lettoTastiera = tasiera.readLine()).equals("fine")){
                netOut.write(lettoTastiera);
                netOut.newLine();
                netOut.flush();
                
                //attendo risposta del server
                ricevuto = netIn.readLine();
                System.out.println(ricevuto);
            }
            
            //chiusura socket
            socket.close();
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
