import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

//scopo esercizio
//creare un client che chieda a RemoteDGramStrlenServer
//i caratteri in una stringa

public class RemoteDGramStrlenClient {
    //java RemoteDGramStrlen nodoServer portaServer
    public static void main(String[] args) {
        //controllo argomenti
        if(args.length != 2){
            System.err.println("Uso: java RemoteDGramStrlen nodoServer portaServer");
            System.exit(1);
        }

        try {
            //creazione socket
            DatagramSocket datagramSocket = new DatagramSocket();

            //creazione stream lettura tastiera
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));

            //lettura da tastiera
            String richiesta;
            while(!(richiesta = tastiera.readLine()).equals("fine")){
                byte reqBuff[] = richiesta.getBytes("UTF-8");
                DatagramPacket reqPacket = new DatagramPacket(reqBuff, reqBuff.length, InetAddress.getByName(args[0]), Integer.parseInt(args[1]));

                datagramSocket.send(reqPacket);

                //preparazione per la risposta
                byte resBuff[] = new byte[2048];
                DatagramPacket resPacket = new DatagramPacket(resBuff, resBuff.length);
                
                //ricezione messaggio risposta, forse
                datagramSocket.receive(resPacket);

                //lettura della risposta
                String risposta = new String(resPacket.getData(), "UTF-8");
                System.out.println(risposta);
            }

            datagramSocket.close();
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
