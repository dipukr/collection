
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

public class Coll {
	public static void main(final String[] args) throws Exception {
		List<String> lines = Files.lines(
			Path.of("Coll.java"))
			.toList();
		lines.forEach(System.out::println);	
	}
}
