import javax.swing.*;
import java.awt.*;

class Node {
    int key;
    Node left, right;

    Node(int key) { this.key = key; }
}

class BST {
    Node root;
    public void insert(int key) { root = insertRec(root, key); }
    private Node insertRec(Node node, int key) {
        if (node == null) return new Node(key);
        if (key < node.key) node.left = insertRec(node.left, key);
        else if (key > node.key) node.right = insertRec(node.right, key);
        return node;
    }
}

class TreePanel extends JPanel {
    private final BST bst;
    private final int nodeSize = 30;
    private final int levelGap = 50;
    private final int siblingGap = 20;

    TreePanel(BST bst) { this.bst = bst; }

    @Override
    protected void paintComponent(Graphics g) {
        super.paintComponent(g);
        if (bst.root != null) {
            Graphics2D g2 = (Graphics2D) g;
            int panelWidth = getWidth();
            drawNode(g2, bst.root, panelWidth / 2, 30, panelWidth / 4);
        }
    }

    private void drawNode(Graphics2D g, Node node, int x, int y, int hGap) {
        g.setColor(Color.BLACK);
        g.fillOval(x - nodeSize/2, y - nodeSize/2, nodeSize, nodeSize);
        g.setColor(Color.WHITE);
        String text = String.valueOf(node.key);
        FontMetrics fm = g.getFontMetrics();
        int tx = x - fm.stringWidth(text)/2;
        int ty = y + fm.getAscent()/2 - 2;
        g.drawString(text, tx, ty);

        if (node.left != null) {
            int nx = x - hGap;
            int ny = y + levelGap;
            g.setColor(Color.BLACK);
            g.drawLine(x, y + nodeSize/2, nx, ny - nodeSize/2);
            drawNode(g, node.left, nx, ny, hGap / 2);
        }
        if (node.right != null) {
            int nx = x + hGap;
            int ny = y + levelGap;
            g.setColor(Color.BLACK);
            g.drawLine(x, y + nodeSize/2, nx, ny - nodeSize/2);
            drawNode(g, node.right, nx, ny, hGap / 2);
        }
    }
}

public class PrettyPrintBST extends JFrame {
    public PrettyPrintBST(BST bst) {
        setTitle("BST Pretty Print (Swing)");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        add(new TreePanel(bst));
        setSize(800, 600);
        setLocationRelativeTo(null);
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            BST tree = new BST();
            for (int key : new int[]{50, 30, 70, 20, 40, 60, 80})
                tree.insert(key);
            new PrettyPrintBST(tree).setVisible(true);
        });
    }
}