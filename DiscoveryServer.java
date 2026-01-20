package main;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

public class DiscoveryServer {
	private ConcurrentHashMap<String, Set<ServiceInstance>> registry;
	private ScheduledExecutorService healthCheckExecutor;
	private ExecutorService clientHandlerPool;
	private ServerSocket serverSocket;
	private volatile boolean running;
	private long instanceTimeout;
	private int port;

	public DiscoveryServer(int port, long instanceTimeout) {
		this.port = port;
		this.registry = new ConcurrentHashMap<>();
		this.healthCheckExecutor = Executors.newSingleThreadScheduledExecutor();
		this.clientHandlerPool = Executors.newCachedThreadPool();
		this.instanceTimeout = instanceTimeout;
	}

	public void start() throws IOException {
		serverSocket = new ServerSocket(port);
		running = true;

		System.out.println("Discovery Server started on port " + port);

		// Start health check
		healthCheckExecutor.scheduleAtFixedRate(this::performHealthCheck, 5000, 5000, TimeUnit.MILLISECONDS);

		// Accept client connections
		while (running) {
			try {
				Socket clientSocket = serverSocket.accept();
				clientHandlerPool.execute(() -> handleClient(clientSocket));
			} catch (IOException e) {
				if (running) {
					e.printStackTrace();
				}
			}
		}
	}

	public void handleClient(Socket socket) {
		try (var istream = new ObjectInputStream(socket.getInputStream());
			var ostream = new ObjectOutputStream(socket.getOutputStream())) {
			Message message = (Message) istream.readObject();
			Message response = processMessage(message);
			ostream.writeObject(response);
			ostream.flush();
		} catch (Exception e) {
			e.printStackTrace();
		} finally {
			try {
				socket.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public Message processMessage(Message message) {
		return switch (message.getType()) {
			case REGISTER -> {
				register(message.getInstance());
				yield new Message(MessageType.RESPONSE, "OK");
			}
			case DEREGISTER -> {
				deregister(message.getInstance());
				yield new Message(MessageType.RESPONSE, "OK");
			}
			case HEARTBEAT -> {
				heartbeat(message.getInstance());
				yield new Message(MessageType.RESPONSE, "OK");
			}
			case DISCOVER -> {
				List<ServiceInstance> instances = getInstances(message.getServiceId());
				yield new Message(MessageType.RESPONSE, (Serializable) instances);
			}
			case LIST_SERVICES -> {
				List<String> services = new ArrayList<>(registry.keySet());
				yield new Message(MessageType.RESPONSE, (Serializable) services);
			}
			default -> new Message(MessageType.RESPONSE, "UNKNOWN");
		};
	}

	public void register(ServiceInstance instance) {
		registry.computeIfAbsent(instance.getServiceId(), 
				k -> ConcurrentHashMap.newKeySet()).add(instance);
		System.out.println("Registered: " + instance);
	}

	public void deregister(ServiceInstance instance) {
		Set<ServiceInstance> instances = registry.get(instance.getServiceId());
		if (instances != null) {
			instances.remove(instance);
			if (instances.isEmpty()) {
				registry.remove(instance.getServiceId());
			}
			System.out.println("Deregistered: " + instance);
		}
	}

	public void heartbeat(ServiceInstance instance) {
		Set<ServiceInstance> instances = registry.get(instance.getServiceId());
		if (instances != null) {
			instances.stream().filter(i -> i.equals(instance))
				.findFirst()
				.ifPresent(ServiceInstance::updateHeartbeat);
		}
	}

	public List<ServiceInstance> getInstances(String serviceId) {
		return registry.containsKey(serviceId) ? 
			new ArrayList<>(registry.get(serviceId)) : 
				Collections.emptyList();
	}

	public void performHealthCheck() {
		registry.forEach((serviceId, instances) -> {
			instances.removeIf(instance -> {
				boolean unhealthy = !instance.isHealthy(instanceTimeout);
				if (unhealthy) {
					System.out.println("Removing unhealthy instance: " + instance);
				}
				return unhealthy;
			});
			if (instances.isEmpty()) {
				registry.remove(serviceId);
			}
		});
	}

	public void stop() {
		running = false;
		try {
			if (serverSocket != null)
				serverSocket.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		clientHandlerPool.shutdown();
		healthCheckExecutor.shutdown();
	}
}