class Person {
	String name;
	int age;
	int sal;	
	Person(String name, int age, int sal) {
		this.name = name;
		this.age = age;
		this.sal = sal;
	}
}

void main(String[] args) throws Exception
{
	Files.lines(Path.of("Main.java")).forEach(System.out::println);
	System.out.println(System.getenv("PATH"));
}
