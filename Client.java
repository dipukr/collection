import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;

public class Client {
	public static void main(String argv[]) throws Exception {
		Socket socket = new Socket("2401:4900:8840:c714:70df:43b6:9ded:8104", 6789);
		BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in)); 
		DataOutputStream outToServer = new DataOutputStream(socket.getOutputStream());	
		BufferedReader inFromServer = new BufferedReader(new InputStreamReader(socket.getInputStream()));
		System.out.print("Enter message: ");
		String message = inFromUser.readLine(); 
		outToServer.writeBytes(message + '\n'); 
		message = inFromServer.readLine(); 
		System.out.println("From server: " + message); 
		socket.close();
	}
} 
