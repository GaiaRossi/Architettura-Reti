import java.io.*;
import java.net.*;

public class coffee_machines_client_java{
    public static void main(String[] args) {
        /**controllo argomenti */
        /**coffee_machines server porta */
        if(args.length != 2){
            System.err.println("Uso: java coffee_machines_client_java server porta");
            System.exit(1);
        }

        try {
            Socket socket = new Socket(args[0], Integer.parseInt(args[1]));
            InputStream netIn = socket.getInputStream();
            OutputStream netOut = socket.getOutputStream();
            BufferedReader tastiera = new BufferedReader(new InputStreamReader(System.in));
            
            byte len[] = new byte[2];
            byte ack[] = new byte[4];

            System.out.println("Inserisci lo username:");
            String username = tastiera.readLine();

            if(username.equals("fine")){
                len[0] = 0;
                len[1] = 0;

                netOut.write(len);

                socket.close();
                System.exit(1);
            }

            System.out.println("Inserisci la password:");
            String password = tastiera.readLine();

            if(password.equals("fine")){
                len[0] = 0;
                len[1] = 0;

                netOut.write(len);

                socket.close();
                System.exit(1);
            }
            
            /**invio lunghezza e username al server */
            byte username_utf8[] = username.getBytes("UTF-8");
            int username_len = username.length();
            
            len[0] = Integer.valueOf((username_len & 0xFF00) >> 8).byteValue();
            len[1] = Integer.valueOf(username_len & 0xFF).byteValue();

            netOut.write(len);
            netOut.write(username_utf8);
            netOut.flush();

            /**invio lunghezza e password al server */
            byte password_utf8[] = password.getBytes("UTF-8");
            int password_len = password.length();

            len[0] = Integer.valueOf((password_len & 0xFF00) >> 8).byteValue();
            len[1] = Integer.valueOf(password_len & 0xFF).byteValue();
            
            netOut.write(len);
            netOut.write(password_utf8);
            netOut.flush();

            /**verifico ack */
            netIn.read(ack);
            if(ack[2] != 79 || ack[3] != 75){
                System.err.println("Non autenticato");
            }
            else{
                System.out.println("Autenticato");
            }

            while(true){
                System.out.println("Inserisci la categoria:");
                String categoria = tastiera.readLine();

                if(categoria.equals("fine")){
                    len[0] = 0;
                    len[1] = 0;

                    netOut.write(len);

                    break;
                }

                /**invio categoria */
                byte categoria_utf8[] = categoria.getBytes("UTF-8");
                int categoria_len = categoria.length();

                len[0] = Integer.valueOf((categoria_len & 0xFF00) >> 8).byteValue();
                len[1] = Integer.valueOf(categoria_len& 0xFF).byteValue();
                
                netOut.write(len);
                netOut.write(categoria_utf8);
                netOut.flush();

                /**leggo risposta */
                netIn.read(len);

                int to_read = (len[0] << 8) | len[1];
                int nread;

                while (to_read > 0) {
                    byte[] buffer = new byte[4096];

                    int bufsize = buffer.length;
                    int sz = (to_read < bufsize) ? to_read : bufsize;

                    nread = netIn.read(buffer, 0, sz);
                    System.out.write(buffer, 0, nread);

                    to_read -= nread;
		        }
            }
            socket.close();
            
        } catch (Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(1);
        }
    }
}