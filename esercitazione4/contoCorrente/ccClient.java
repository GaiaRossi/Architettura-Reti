import java.io.*;
import java.net.*;

public class ccClient{
    public static void main(String[] args) {
        //controllo argomenti
        if(args.length != 2){
            System.err.println("Uso: java ccClient hostname porta");
            System.exit(1);
        }

        try {
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
        
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));
        
        
            String input, risposta;
            while(!(input = tastiera.readLine()).equals("fine")){
                //invio la categoria
                netOut.write(input, 0, input.length());
                netOut.flush();

                //leggo risposta
                while(!((risposta = netIn.readLine()).equals("fine"))){
                    System.out.println(risposta);
                }
            }
        
            netOut.close();
            netIn.close();
            socket.close();
            tastiera.close();

        } catch (Exception e) {
            System.out.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}