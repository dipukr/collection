import std.stdio;
class Animal {
	this() {
		writeln("Constructor called");
	}
	~this() {
		writeln("Destructor called");
	}
}
class Point {
	int x;
	int y;
	this(int x, int y) {
		this.x = x;
		this.y = y;
	}
}
void main() {
	Animal a=new Animal();
	Point p = new Point(10,20);
	p.x = 23;
	writeln(p.x,p.y);
}

class Animal {
	val name;
	val age;
	val sal;

	this(name, age, sal) {
		this.name = name;
		this.age = age;
		this.sal = sal;
	}
}