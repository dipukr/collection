import java.util.*;

public class BST {
    public Node root;

    public void insert(int key) {
        root = insertRec(root, key);
    }

    public Node insertRec(Node node, int key) {
        if (node == null) return new Node(key);
        if (key < node.key)
            node.left = insertRec(node.left, key);
        else if (key > node.key)
            node.right = insertRec(node.right, key);
        return node;
    }

    public void prettyPrint() {
        printTree(root, "", true);
    }

    public void printTree(Node node, String prefix, boolean isTail) {
        if (node == null) return;

        System.out.println(prefix + (isTail ? "└── " : "├── ") + node.key);

        List<Node> children = new ArrayList<>();
        if (node.left != null) children.add(node.left);
        if (node.right != null) children.add(node.right);

        for (int i = 0; i < children.size(); i++) {
            boolean last = (i == children.size() - 1);
            printTree(children.get(i), prefix + (isTail ? "    " : "│   "), last);
        }
    }
}