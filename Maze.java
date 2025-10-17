import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.io.File;
import java.util.Arrays;
import java.util.Collections;
import java.util.Deque;
import java.util.LinkedList;
import java.util.stream.Stream;

import javax.imageio.ImageIO;
import javax.swing.JFrame;
import javax.swing.JPanel;

public class Maze extends JPanel {
	
	private static final Color wallColor = new Color(39,40,34);
	private static final long serialVersionUID = 1;
	private static final int margin = 50;
	private static final int GS = 40;
	
	private static Image apple = null;
	private static Image mouse = null;
	
	record Point(int x, int y) {}
	
	private int[][] maze;
	private int rows;
	private int cols;
	
	public Maze(int[][] maze) throws Exception {
		setVisible(true);
		this.maze = maze;
		this.rows = maze.length;
		this.cols = maze[0].length;
		int W = GS * cols + margin;
		int H = GS * rows + margin;
		setPreferredSize(new Dimension(W, H));
		apple = ImageIO.read(new File("resources/apple.jpg"));
		mouse = ImageIO.read(new File("resources/mouse.png"));
	}

	public boolean isValid(int row, int col) {
		if (row >= 0 && row < rows && col >= 0 && col < cols)
			return maze[row][col] == 0 || maze[row][col] == 9;
		return false;
	}
	
	public boolean isGoal(int row, int col) {
		return maze[row][col] == 9;
	}
	
	public boolean hasPath(int row, int col, Deque<Point> path) {
		if (isValid(row, col)) {
			if (isGoal(row, col)) return true;
			maze[row][col] = 2;
			if (hasPath(row, col - 1, path)) {
				path.addLast(new Point(row, col - 1));
				return true;
			}
			if (hasPath(row, col + 1, path)) {
				path.addLast(new Point(row, col + 1));
				return true;
			}
			if (hasPath(row - 1, col, path)) {
				path.addLast(new Point(row - 1, col));
				return true;
			}
			if (hasPath(row + 1, col, path)) {
				path.addLast(new Point(row + 1, col));
				return true;
			}
			maze[row][col] = 0;
		}
		return false;
	}
	
	@Override
	public void repaint(long millis) {
		try {
			Thread.sleep(millis);
		} catch (InterruptedException e) {}
		this.repaint();
	}
	
	@Override
	public void paint(Graphics g) {
		var gc = (Graphics2D) g;
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				int x = j * GS + margin / 2;
				int y = i * GS + margin / 2;
				if (maze[i][j] == 0 || maze[i][j] == 2) {
					gc.setColor(Color.white);
					gc.fillRect(x, y, GS, GS);
				} else if (maze[i][j] == 1) {
					gc.setColor(wallColor);
					gc.fillRect(x, y, GS, GS);
				} else if (maze[i][j] == 9) {
					gc.drawImage(apple, x, y, GS, GS, this);
				}
				gc.setColor(Color.black);
				gc.drawRect(x, y, GS, GS);
			}
		}
	}
	
	public void draw() {
		for (int row = 0; row < rows; row++) {
			for (int col = 0; col < cols; col++) {
				int code = maze[row][col] == 0 ? 9608: 32;
				System.out.printf("%s%c%c", "\u001B[32m", code, code);
			}
			System.out.println();
		}
	}
	
	public void dump() {
		Stream.of(maze)
			.map(Arrays::toString)
			.forEach(System.out::println);
	}

	public static void main(String[] args) throws Exception {
		int[][] data = {
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
			{1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
			{1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
			{1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
			{1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1},
			{1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,1},
			{1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
			{1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
			{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
			{1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1},
			{1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
			{1,1,1,1,1,0,0,0,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1},
			{1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1},
			{1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1},
			{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,0,1},
			{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,9,1},
			{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
		};
		
		var frame = new JFrame();
		var maze = new Maze(data);
		maze.draw();
		
		frame.add(maze, BorderLayout.CENTER);
		frame.pack();
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		var path = new LinkedList<Point>();
		if (maze.hasPath(1, 1, path))
			path.addLast(new Point(1, 1));
		Collections.reverse(path);
		System.out.println(path);
		while (!path.isEmpty()) {
			Graphics2D gc = (Graphics2D) frame.getGraphics();
			Point point = path.removeLast();
			int x = point.y * GS + margin / 2;
			int y = point.x * GS + margin / 2;
			Thread.sleep(500);
			gc.drawImage(mouse, x, y, GS, GS, frame);
		}
	}
}