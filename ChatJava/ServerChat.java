import java.net.*;
import java.io.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class ServerChat{

    static final int PORT = 50050;

    public static void main(String[] args) {

        try {
            ServerSocket ss = new ServerSocket(PORT);

            while(true){
                Socket socket = ss.accept();
                ChatThread ct = new ChatThread(socket);
                ct.start();
            }
    
        } catch (Exception e) {
            e.printStackTrace();
            System.exit(1);
        }
    }
}