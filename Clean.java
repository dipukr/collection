import java.io.File;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Optional;

public class Clean {
	public static void main(String[] args) {
		File dir = new File(".");
		File[] files = dir.listFiles();
		for (File file: files) {
			String fileName = file.getName();
			if (fileName.endsWith(".class") || 
				fileName.endsWith(".out") ||
				fileName.endsWith(".jar") ||
				fileName.endsWith(".o")) {
				file.delete();
			}
			if (file.isFile() && file.canExecute()) {
				String filePath = file.getAbsolutePath();
				Optional<String> first = null;
				try {
					first = Files.lines(Path.of(filePath)).findFirst();
					if (first.isPresent() && first.get().startsWith("#!"));
					else file.delete();
				} catch (Exception e) {
					file.delete();
				}
			}
		}
	}
}