import java.util.Map;
import java.util.HashMap;

public class Node {
	
	public String key;
	public Object val;
	public Node next;
	public Node prev;

	public Node(String key, Object val) {
		this.key = key;
		this.val = val;
	}
}