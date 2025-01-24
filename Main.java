import java.util.stream.Stream;
import java.util.stream.Collectors;

public class Main {
	
	public static final String RESET = "\u001B[0m";
	public static final String RED = "\u001B[31m";
	
	public static void main(final String[] args) {
		System.out.printf("%shello_world%s", RED, RESET);
		System.out.println("hello");
	}
}