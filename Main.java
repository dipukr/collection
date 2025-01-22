import java.util.stream.Stream;
import java.util.stream.Collectors;

public class Main {
	
	public static int num(int n) {
		while (n > 26) 
			n = n / 26;
		return n;
	}

	public static char getChar(int n) {
		return (char)(n + 64);
	}

	public static String getLabel(int colnum) {
		colnum += 1;
		String label = "";
		int rem = 0;
		while (colnum > 26) {
			label += getChar(num(colnum));
			rem = colnum % 26;
			colnum /= 26;
		}
		label += getChar(rem);
		return label;
	}
	
	public static void main(final String[] args) throws Exception {
		Stream.iterate(0, i -> i + 1)
			.limit(1000)
			.forEach(elem -> System.out.printf("%d:%s\n", elem, getLabel(elem)));
	}
}