public class CircularLinkedList {
    private Node head = null;
    private Node tail = null;

    private static class Node {
        int data;
        Node next;

        Node(int data) {
            this.data = data;
        }
    }

    // Add to end
    public void append(int data) {
        Node newNode = new Node(data);
        if (head == null) {
            head = tail = newNode;
            tail.next = head;
        } else {
            tail.next = newNode;
            tail = newNode;
            tail.next = head;
        }
    }

    // Add to beginning
    public void prepend(int data) {
        Node newNode = new Node(data);
        if (head == null) {
            head = tail = newNode;
            tail.next = head;
        } else {
            newNode.next = head;
            head = newNode;
            tail.next = head;
        }
    }

    // Delete a node by value
    public void delete(int data) {
        if (head == null) return;

        Node current = head;
        Node previous = tail;

        do {
            if (current.data == data) {
                if (current == head) {
                    head = head.next;
                    tail.next = head;
                } else if (current == tail) {
                    tail = previous;
                    tail.next = head;
                } else {
                    previous.next = current.next;
                }
                return;
            }

            previous = current;
            current = current.next;
        } while (current != head);
    }

    // Print the list
    public void printList() {
        if (head == null) {
            System.out.println("List is empty");
            return;
        }

        Node current = head;
        do {
            System.out.print(current.data + " -> ");
            current = current.next;
        } while (current != head);
        System.out.println("(head)");
    }

    public static void main(String[] args) {
        CircularLinkedList cll = new CircularLinkedList();
        cll.append(10);
        cll.append(20);
        cll.append(30);
        cll.printList();  // 10 -> 20 -> 30 -> (head)

        cll.prepend(5);
        cll.printList();  // 5 -> 10 -> 20 -> 30 -> (head)

        cll.delete(20);
        cll.printList();  // 5 -> 10 -> 30 -> (head)

        cll.delete(5);
        cll.printList();  // 10 -> 30 -> (head)
    }
}
