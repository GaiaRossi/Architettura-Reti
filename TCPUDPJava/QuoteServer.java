import java.net.DatagramPacket;
import java.net.DatagramSocket;

public class QuoteServer{

    //java QuoteServer porta
    public static void main(String[] args) {
        String[] quotations = { 
            "Adoro i piani ben riusciti",
            "Quel tappeto dava veramente un tono all'ambiente",
            "Se ci riprovi ti stacco un braccio",
            "Questo è un colpo di genio, Leonard",
            "I fagioli comunque erano uno schifo"
        };

        try {
            DatagramSocket datagramSocket = new DatagramSocket(Integer.parseInt(args[0]));

            int index = 0;

            while(true){
                byte[] reqBuf = new byte[2048];
                DatagramPacket reqPacket = new DatagramPacket(reqBuf, reqBuf.length);
                datagramSocket.receive(reqPacket);
                String richiesta = new String(reqPacket.getData(), 0, reqPacket.getLength(), "UTF-8");

                if(richiesta.equals("QUOTE")){
                    String quote = quotations[index % quotations.length];

                    byte[] quoteBuf = quote.getBytes("UTF-8");

                    DatagramPacket quotePacket = new DatagramPacket(quoteBuf, quoteBuf.length, reqPacket.getAddress(), reqPacket.getPort());
                    datagramSocket.send(quotePacket);
                }
                index++; 
            }


        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
        
    }
}