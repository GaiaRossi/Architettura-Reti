import java.net.*;

//scopo esercizio
//creare un server che, data una stringa inviata dal client
//ritorni il numero di caratteri in quella stringa

public class RemoteDGramStrlenServer {
    //java RemoteDGramStrlenServer portaServer
    public static void main(String[] args) {
        //controllo argomenti
        if(args.length != 1){
            System.err.println("Uso: java RemoteDGramStrlenServer portaServer");
            System.exit(1);
        }
        try {
            //creazione della socket
            DatagramSocket datagramSocket = new DatagramSocket(Integer.parseInt(args[0]));

            //creazione del buffer e del pacchetto per ricevere la richiesta
            byte reqBuff[] = new byte[2048];
            DatagramPacket reqPacket = new DatagramPacket(reqBuff, reqBuff.length);

            while(true){
                //ricezione della richiesta
                datagramSocket.receive(reqPacket);

                //lettura del pacchetto ricevuto
                String richiesta = new String(reqPacket.getData(), 0, reqPacket.getLength(), "UTF-8");

                //conto dei caratteri
                String risposta = Integer.valueOf(richiesta.length()).toString();
                byte resBuf[] = risposta.getBytes("UTF-8");
                
                //creo il pacchetto di risposta
                DatagramPacket resPacket = new DatagramPacket(resBuf, resBuf.length, reqPacket.getAddress(), reqPacket.getPort());

                //invio pacchetto
                datagramSocket.send(resPacket);
            }

            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(2);
        }
    }
}
