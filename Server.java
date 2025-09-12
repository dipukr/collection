import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class Server {
	
	private static final DateFormat formatter = new SimpleDateFormat("dd-MM-yyyy HH:mm:ss");
	private int port;

	public Server(int port) {
		this.port = port;
	}

	public void start() {
		ExecutorService executorService = Executors.newFixedThreadPool(4);
		try (ServerSocket serverSocket = new ServerSocket(port);
			FileWriter fileWriter = new FileWriter("/tmp/log/server.log", true)) {
			while (true) {
				Socket clientSocket = serverSocket.accept();
				logger(fileWriter, clientSocket);
				executorService.execute(() -> handle(clientSocket));
			}
		} catch (Exception e) {
			System.out.printf("ERROR: %s.\n", e.getMessage());
		}
	}
	
	public void logger(FileWriter fileWriter, Socket clientSocket) throws Exception {
		InetAddress inetAddr = clientSocket.getInetAddress();
		String hostAddr = inetAddr.getHostAddress();
		StringBuilder msg = new StringBuilder();
		msg.append('[');
		msg.append(formatter.format(new Date()));
		msg.append("] ");
		msg.append(hostAddr);
		msg.append('\n');
		fileWriter.write(msg.toString());
		fileWriter.flush();
	}

	public void handle(Socket socket) {
		try {
			InputStream inputStream = socket.getInputStream();
			InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
			BufferedReader reader = new BufferedReader(inputStreamReader);
			OutputStream outputStream = socket.getOutputStream();
			DataOutputStream writer = new DataOutputStream(outputStream);
			String line = reader.readLine();
			writer.writeBytes(line.toUpperCase() + '\n');
		} catch (Exception e) {
			System.out.printf("ERROR: %s\n", e.getMessage());
		}
	}
}