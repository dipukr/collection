package main;

import java.io.Serializable;
import java.util.Map;
import java.util.Objects;

public class ServiceInstance implements Serializable {
	private String serviceId;
	private String host;
	private int port;
	private boolean secure;
	private long lastHeartbeat;
	private Map<String, String> metadata;

	public ServiceInstance(String serviceId, String host, int port,
			boolean secure, Map<String, String> metadata) {
		this.serviceId = serviceId;
		this.host = host;
		this.port = port;
		this.secure = secure;
		this.metadata = metadata;
		this.lastHeartbeat = System.currentTimeMillis();
	}

	public void updateHeartbeat() {
		this.lastHeartbeat = System.currentTimeMillis();
	}

	public boolean isHealthy(long timeoutMs) {
		return System.currentTimeMillis() - lastHeartbeat < timeoutMs;
	}

	public String getServiceId() {
		return serviceId;
	}

	public String getHost() {
		return host;
	}

	public int getPort() {
		return port;
	}

	public Map<String, String> getMetadata() {
		return metadata;
	}

	public boolean isSecure() {
		return secure;
	}

	public long getLastHeartbeat() {
		return lastHeartbeat;
	}

	@Override
	public boolean equals(Object o) {
		if (this == o)
			return true;
		if (o == null || getClass() != o.getClass())
			return false;
		ServiceInstance that = (ServiceInstance) o;
		return port == that.port &&
				Objects.equals(serviceId, that.serviceId) &&
				Objects.equals(host, that.host);
	}

	@Override
	public int hashCode() {
		return Objects.hash(serviceId, host, port);
	}

	@Override
	public String toString() {
		return String.format("ServiceInstance{serviceId=%s, host=%s, port=%d}",
				serviceId, host, port);
	}
}

