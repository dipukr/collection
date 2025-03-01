import java.net.Socket;
import java.net.ServerSocket;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.Reader;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStreamReader;
import java.util.Scanner;

public class Server {

	private int port;

	public Server(int port) {
		this.port = port;
	}

	public void start() {
		try {
			ServerSocket socket = new ServerSocket(port);
			while (true) {
				System.out.printf("Listening at port %d.\n", 6789);
				Socket clientSocket = socket.accept();
				Scanner inFromClient = new Scanner(clientSocket.getInputStream());
				DataOutputStream outToClient = new DataOutputStream(clientSocket.getOutputStream());
				String message = inFromClient.next();
				String response = message.toUpperCase();
				System.out.printf("Received message: %s\n", message);
				outToClient.writeBytes(response+"\n");
				System.out.printf("Sent message: %s\n", response);
			}
		} catch (Exception e) {
			System.out.printf("ERROR: %s.\n", e.getMessage());
			System.exit(0);
		}
	}

	public static void main(String[] args) throws Exception {
		Server server = new Server(6789);
		server.start();
	}
}