package main;

import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.net.Socket;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

public class DiscoveryClient {
	private String serverHost;
	private int serverPort;
	private ScheduledExecutorService heartbeatExecutor;
	private Map<ServiceInstance, ScheduledFuture<?>> heartbeats;

	public DiscoveryClient(String serverHost, int serverPort) {
		this.serverHost = serverHost;
		this.serverPort = serverPort;
		this.heartbeats = new ConcurrentHashMap<>();
		this.heartbeatExecutor = Executors.newScheduledThreadPool(2);
	}

	public void registerService(ServiceInstance instance, long heartbeatInterval) {
		sendMessage(new Message(MessageType.REGISTER, instance));
		// Start heartbeat
		ScheduledFuture<?> heartbeatTask = 
				heartbeatExecutor.scheduleAtFixedRate(() -> sendHeartbeat(instance),
				heartbeatInterval, heartbeatInterval, TimeUnit.MILLISECONDS);
		heartbeats.put(instance, heartbeatTask);
		System.out.println("Registered service: " + instance);
	}

	public void deregisterService(ServiceInstance instance) {
		ScheduledFuture<?> task = heartbeats.remove(instance);
		if (task != null) {
			task.cancel(false);
		}
		sendMessage(new Message(MessageType.DEREGISTER, instance));
		System.out.println("Deregistered service: " + instance);
	}

	public void sendHeartbeat(ServiceInstance instance) {
		try {
			sendMessage(new Message(MessageType.HEARTBEAT, instance));
		} catch (Exception e) {
			System.err.println("Heartbeat failed for " + instance + ": " + e.getMessage());
		}
	}

	@SuppressWarnings("unchecked")
	public List<ServiceInstance> discoverService(String serviceId) {
		Message response = sendMessage(new Message(MessageType.DISCOVER, serviceId));

		if (response != null && response.getPayload() instanceof List) {
			return (List<ServiceInstance>) response.getPayload();
		}
		return Collections.emptyList();
	}

	public List<String> listServices() {
		Message response = sendMessage(new Message(MessageType.LIST_SERVICES, (Object) null));
		if (response != null && response.getPayload() instanceof List)
			return (List<String>) response.getPayload();
		return Collections.emptyList();
	}

	public Message sendMessage(Message message) {
		try (Socket socket = new Socket(serverHost, serverPort);
				ObjectOutputStream out = new ObjectOutputStream(socket.getOutputStream());
				ObjectInputStream in = new ObjectInputStream(socket.getInputStream())) {
			out.writeObject(message);
			out.flush();
			return (Message) in.readObject();
		} catch (Exception e) {
			System.err.println("Failed to send message: " + e.getMessage());
			return null;
		}
	}

	public void shutdown() {
		heartbeats.values().forEach(task -> task.cancel(false));
		heartbeatExecutor.shutdown();
	}
}
