import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Scanner;

public class Client {

	private String host;
	private int port;

	public Client(String host, int port) {
		this.host = host;
		this.port = port;
	}
	
	public String connectSend(String message) {
		String response = null;
		try {
			Socket socket = new Socket(host, port);
			BufferedReader inFromServer = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			DataOutputStream outToServer = new DataOutputStream(socket.getOutputStream());
			outToServer.writeBytes(message+"\n");
			response = inFromServer.readLine();
			socket.close();
		} catch (Exception e) {
			System.out.printf("ERROR: %s.\n", e.getMessage());
			System.exit(0);
		}
		return response;
	}

	public static void main(String args[]) {
		// if (args.length != 1) return;
		final int N = 1000;
		// for (int i = 1; i <= N; i++) {
		//	Client client = new Client("127.0.0.1", 6789);
		//	System.out.println(client.connectSend(args[0]+i));
		//}
		Runnable task = () -> {
			Client client = new Client("127.0.0.1", 6789);
			System.out.println(client.connectSend(Thread.currentThread().getName()));
		};
		for (int a = 0; a < N; a++) {
			Thread thread = new Thread(task);
			thread.setName("thread_count_"+a);
			thread.start();
		}
		System.out.println("Terminated.");
	}
} 
