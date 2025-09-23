import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;

class Node {
	int data;
	Node next;
	Node(int data) {
		this.data = data;
	}
}



void print(Node head) {
	for (Node curr = head; curr != null; curr = curr.next)
		System.out.print(curr.data+"->");
}


void main(String[] args) {
	Node head = new Node(100);
	head.next = new Node(200);
	print(head);
}