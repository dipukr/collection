import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Image;
import java.io.File;
import java.util.Arrays;
import java.util.Deque;
import java.util.LinkedList;
import java.util.stream.Stream;

import javax.imageio.ImageIO;
import javax.swing.JFrame;
import javax.swing.JPanel;

public class Maze extends JPanel {
	private static final Color WALL_COLOR = new Color(39,40,34);
	private static final String GREEN = "\u001B[32m";
	private static Image APPLE_IMG = null;
	private static Image MOUSE_IMG = null;
	private static final int MARGIN = 50;
	private static final int GS = 40;
	
	record Cell(int x, int y) {}
	
	private int[][] maze;
	private int rows;
	private int cols;
	
	public Maze(int[][] maze) throws Exception {
		setVisible(true);
		this.maze = maze;
		this.rows = maze.length;
		this.cols = maze[0].length;
		int W = GS * cols + MARGIN;
		int H = GS * rows + MARGIN;
		setPreferredSize(new Dimension(W, H));
		APPLE_IMG = ImageIO.read(new File("resources/apple.png"));
		MOUSE_IMG = ImageIO.read(new File("resources/mouse.png"));
	}

	public boolean isValid(int row, int col) {
		if (row >= 0 && row < rows && col >= 0 && col < cols)
			return maze[row][col] == 0 || maze[row][col] == 9;
		return false;
	}
	
	public boolean isGoal(int row, int col) {
		return maze[row][col] == 9;
	}
	
	public boolean hasPath(int row, int col, Deque<Cell> path) {
		if (isValid(row, col)) {
			if (isGoal(row, col)) return true;
			maze[row][col] = 2;
			if (hasPath(row, col - 1, path)) {
				path.addLast(new Cell(row, col - 1));
				return true;
			}
			if (hasPath(row, col + 1, path)) {
				path.addLast(new Cell(row, col + 1));
				return true;
			}
			if (hasPath(row - 1, col, path)) {
				path.addLast(new Cell(row - 1, col));
				return true;
			}
			if (hasPath(row + 1, col, path)) {
				path.addLast(new Cell(row + 1, col));
				return true;
			}
			maze[row][col] = 0;
		}
		return false;
	}
	
	public void draw() {
		for (int row = 0; row < rows; row++) {
			for (int col = 0; col < cols; col++) {
				int code = maze[row][col] == 0 ? 9608: 32;
				System.out.printf("%s%c%c", GREEN, code,code);
			}
			System.out.println();
		}
	}
	
	public void dump() {
		Stream.of(maze)
			.map(Arrays::toString)
			.forEach(System.out::println);
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
				int x = j * GS + MARGIN / 2;
				int y = i * GS + MARGIN / 2;
				if (maze[i][j] == 0 || maze[i][j] == 2) {
					gc.setColor(Color.white);
					gc.fillRect(x, y, GS, GS);
				} else if (maze[i][j] == 1) {
					gc.setColor(WALL_COLOR);
					gc.fillRect(x, y, GS, GS);
				} else if (maze[i][j] == 9) {
					gc.drawImage(APPLE_IMG, x, y, GS, GS, this);
				}
				gc.setColor(Color.black);
				gc.drawRect(x, y, GS, GS);
			}
		}
	}

	public static void main(String[] args) throws Exception {
		int data[][] = {
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
		
		frame.add(maze, BorderLayout.CENTER);
		frame.pack();
		frame.setLocationRelativeTo(null);
		frame.setVisible(true);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		var path = new LinkedList<Cell>();
		if (maze.hasPath(1, 1, path))
			path.addLast(new Cell(1, 1));
		
		//System.out.println(path);
		
		while (path.isEmpty() == false) {
			var gc = (Graphics2D) frame.getGraphics();
			Cell cell = path.removeLast();
			int x = cell.y * GS + MARGIN / 2;
			int y = cell.x * GS + MARGIN / 2;
			Thread.sleep(500);
			gc.drawImage(MOUSE_IMG, x, y, GS, GS, frame);
		}
	}
}