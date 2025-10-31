interface Animal {
	void speak();
}
class Cat implements Animal {
	@Override
	public void speak() {
		System.out.println("Meow...");
	}
}
class Dog implements Animal {
	@Override
	public void speak() {
		System.out.println("Bark...");
	}
}
public class Poly {
	public static void main(String[] args) {
		Animal a = new Dog();
		a.speak();
		long l = (Long)a;
		a = new Cat();
		a.speak();
	}
}