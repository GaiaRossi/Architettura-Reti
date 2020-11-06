import java.io.*;
import java.net.*;

public class RemoteHeadServer {
    
    //java RemoteHeadServer porta
    public static void main(String[] args) {
        try {
            ServerSocket ss = new ServerSocket(Integer.parseInt(args[0]));

            while(true){
                Socket socket = ss.accept();

                BufferedReader netIn = new BufferedReader(new InputStreamReader(socket.getInputStream(), "UTF-8"));
                BufferedWriter netOut = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream(), "UTF-8"));

                String fileName = netIn.readLine();

                File file = new File(fileName);

                if(file.exists()){
                    FileReader fileReader = new FileReader(file);
                    BufferedReader bufferedFile = new BufferedReader(fileReader);
                    int linee_inviate = 0;
                    while(linee_inviate < 5){
                        String line = bufferedFile.readLine();
                        netOut.write(line);
                        netOut.newLine();
                        netOut.flush();
                        linee_inviate++;
                    }
                }

                socket.close();

            }
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
    
}
