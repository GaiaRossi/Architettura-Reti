import java.io.*;
import java.net.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class ChatThread extends Thread{
    Socket socket;
    AtomicBoolean isRunning = new AtomicBoolean(false);

    public ChatThread(Socket socket){
        this.socket = socket;
    }

    public void run(){
        WritingThread wt = new WritingThread(socket);
        wt.start();
        ReadingThread rt = new ReadingThread(socket);
        rt.start();
    }
}

class WritingThread extends Thread{

    Socket socket;
    AtomicBoolean isRunning = new AtomicBoolean(false);

    public WritingThread(Socket socket){
        this.socket = socket;
    }

    public boolean isWorking(){
        return isRunning.get();
    }

    public void stopWriting(){
        isRunning.set(false);
    }

    public void run(){
        isRunning.set(true);
        System.out.println("Writing started on " + socket.getInetAddress().getHostAddress());
        System.out.println("Inserisci exit per uscire");
        try {
            BufferedWriter outNet = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));

            String messaggio;

            while(!((messaggio = tastiera.readLine()).equals("exit")) & isRunning.get()){
                try {
                    outNet.write(messaggio);
                    outNet.newLine();
                    outNet.flush();
                } catch (Exception e) {
                    tastiera.close();
                    socket.close();
                    isRunning.set(false);
                }
                
            }
            tastiera.close();
            socket.close();
            isRunning.set(false);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}

class ReadingThread extends Thread{
    Socket socket;
    AtomicBoolean isRunning = new AtomicBoolean(false);

    public ReadingThread(Socket socket){
        this.socket = socket;
    }

    public boolean isWorking(){
        return isRunning.get();
    }

    public void stopReading(){
        isRunning.set(false);
    }

    public void run(){
        isRunning.set(true);
        System.out.println("Reading started on " + socket.getInetAddress().getHostAddress());
        try {
            BufferedReader inNet = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));

            String messaggio;

            while(isRunning.get() & (messaggio = inNet.readLine()) != null){
                System.out.println(socket.getRemoteSocketAddress().toString() + ": " + messaggio);
            }

            isRunning.set(false);

        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
