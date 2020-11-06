import java.net.*;
import java.io.*;

public class RemoteHeadClient{
    
    //java RemoteHeadClient hostname porta file
    public static void main(String[] args) {
        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));
            BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));

            netOut.write(args[2]);
            netOut.newLine();
            netOut.flush();

            int linee_lette = 0;

            String linea_letta;
            while((linea_letta = netIn.readLine()) != null && linee_lette < 5){
                System.out.println(linea_letta);
                linee_lette++;
            }

            socket.close();

        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}