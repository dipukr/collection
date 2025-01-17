import java.io.File;

public class Clean {
	public static void main(final String[] args) {
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
				file.delete();
			}
		}
	}
}