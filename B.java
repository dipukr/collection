class Point {
	int x; int y;
	Point(int x, int y) {
		this.x = x;
		this.y = y;
	}
}

public class B {
	public static void main(String[] args) {
		Point a = new Point(10, 20);
		System.out.println(a.x);
		System.out.println(a.y);
	}
}