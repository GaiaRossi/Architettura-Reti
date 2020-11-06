import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;

public class QuoteClient {
    //java QuoteClient nodoServer portaServer
    public static void main(String[] args) {

        try {
            byte[] reqBuf = new String("QUOTE").getBytes("UTF-8");

            DatagramSocket datagramSocket = new DatagramSocket();
            DatagramPacket reqPacket = new DatagramPacket(reqBuf, 0, reqBuf.length, InetAddress.getByName(args[0]), Integer.parseInt(args[1]));

            datagramSocket.send(reqPacket);

            byte[] recvBuf = new byte[2048];
            DatagramPacket recvPacket = new DatagramPacket(recvBuf, recvBuf.length);
            datagramSocket.receive(recvPacket);

            String citazione = new String(recvPacket.getData(), 0, recvPacket.getLength(), "UTF-8");
            System.out.println(citazione);

            datagramSocket.close();
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }

    }
}
