public class Main {
    public static void main(String[] args) {
        BST tree = new BST();

        // Creating a balanced BST
        int[] keys = {50, 30, 70, 20, 40, 60, 80};
        for (int key : keys)
            tree.insert(key);

        tree.prettyPrint2D();
    }
}