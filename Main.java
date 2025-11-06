public class Main {
	public static int add(int a, int b) {
		return a + b;
	}
	public static void main(String[] args) {
		int r = add(100, 200);
		System.out.println(r);
		A obj = new A();
	}
}
class A {
	public A() {
		System.out.println("A constructed.");
	}
}