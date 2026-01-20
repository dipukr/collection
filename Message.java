package main;

import java.io.Serializable;

public class Message implements Serializable {
	private ServiceInstance instance;
	private String serviceId;
	private MessageType type;
	private Object payload;
	
	public Message(MessageType type, ServiceInstance instance, String serviceId, Object payload) {
		this.type = type;
		this.instance = instance;
		this.serviceId = serviceId;
		this.payload = payload;
	}

	public Message(MessageType type, ServiceInstance instance) {
		this(type, instance, null, null);
	}

	public Message(MessageType type, String serviceId) {
		this(type, null, serviceId, null);
	}

	public Message(MessageType type, Object payload) {
		this(type, null, null, payload);
	}

	public MessageType getType() {
		return type;
	}

	public ServiceInstance getInstance() {
		return instance;
	}

	public String getServiceId() {
		return serviceId;
	}

	public Object getPayload() {
		return payload;
	}
}