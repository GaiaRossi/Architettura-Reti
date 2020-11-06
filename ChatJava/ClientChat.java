import java.net.*;
import java.io.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class ClientChat {
    public static void main(String[] args) {
        try {
            
            Socket socket = new Socket(InetAddress.getByName(null), 50050);
            ChatThread ct = new ChatThread(socket);
            ct.start();

        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}