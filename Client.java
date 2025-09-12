import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.Socket;

public class Client {
	public static void main(String[] args) throws Exception {
		final int N = 1;
		for (int i = 0; i < N; i++) {
			Socket serverSocket = new Socket("localhost", 8780);
			InputStream inputStream = serverSocket.getInputStream();
			OutputStream outputStream = serverSocket.getOutputStream();
			InputStreamReader streamReader = new InputStreamReader(inputStream);
			BufferedReader fromServer = new BufferedReader(streamReader);
			DataOutputStream toServer = new DataOutputStream(outputStream);
			toServer.writeBytes("hello\n");
			String message = fromServer.readLine(); 
			System.out.println("from server: " + message); 
			serverSocket.close();
		}
	}
}