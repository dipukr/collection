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
	var queue = new LinkedList<Integer>();
	queue.offer(100);
	queue.offer(200);
	queue.offer(300);
	while (!queue.isEmpty())
		System.out.println(queue.poll());
	System.out.println(new Random().nextInt(100));
}
