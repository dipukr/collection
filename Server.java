import java.net.Socket;
import java.net.ServerSocket;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;

public class Server {
	public static void main(String[] argv) throws Exception  {
		ServerSocket socket = new ServerSocket(6789);
		while (true) {
			System.out.printf("Listening at port %d\n", 6789);
			Socket connection = socket.accept();
			InputStream inputStream = connection.getInputStream();
			OutputStream outputStream = connection.getOutputStream();
			InputStreamReader isr = new InputStreamReader(inputStream);
			BufferedReader in = new BufferedReader(isr);
			DataOutputStream out = new DataOutputStream(outputStream);
			String message = in.readLine();
			System.out.printf("Received message: %s\n", message);
			message = message.toUpperCase()+'\n';
			out.writeBytes(message);
			System.out.printf("Sent message: %s", message);
		}
	}
}