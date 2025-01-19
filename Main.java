import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.*;
import java.util.stream.IntStream;

public class Main {
	public static void main(final String[] args) throws Exception {
		Files.lines(Path.of("Main.java"))
			.forEach(System.out::println);
		IntStream.of(10, 20, 30, 40, 50)
			.filter(elem -> elem > 20)
			.forEach(System.out::println);
	}
}
