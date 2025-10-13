import java.nio.file.Path;
import java.util.function.Function;
import java.nio.file.Files;

public class Main {
	public static void main(String[] args) throws Exception {
		var mapper = (String arg) -> arg.toLowerCase();
		Files.lines(Path.of("Main.java"))
			.map(mapper)
			.forEach(System.out::println);
		mapper.andThen(after);
	}
}