import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;

public class Main {
	public static void main(final String[] args) throws Exception {
		
		Files.lines(Path.of("Main.java")).forEach(System.out::println);
	}
}