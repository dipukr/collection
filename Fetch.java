import java.io.File;
import java.util.List;
import java.util.Queue;
import java.util.ArrayList;
import java.util.Map;
import java.util.Scanner;
import java.util.HashMap;
import java.util.LinkedList;
import java.nio.file.Files;
import java.nio.file.Path;
import java.io.IOException;

public class Fetch {	
	public static void main(final String[] args) {
		if (args.length != 1) return;
		String fileName = args[0];
		List<String> dirs = List.of("/home/server/RESEARCH",
			"/home/server/sources", "/home/server/SAMAST");
		Search search = new Search(dirs);
		List<String> paths = search.search(fileName);
		if (paths == null) {
			System.out.printf("File '%s' not found.\n", fileName);
			return;
		}
		int code = 0;
		Runtime runtime = Runtime.getRuntime();
		if (paths.size() > 1) {
			int count = 0;
			for (String path: paths)
				System.out.printf("[%d][%s]\n", count++, path);
			System.out.print("Which one? ");
			try (Scanner scanner = new Scanner(System.in)) {
				String word = scanner.next();
				try {
					code = Integer.valueOf(word);
				} catch (NumberFormatException e) {
					System.out.printf("%c\n", 128544);
					System.exit(0);
				}
			}
			if (code < 0 || code >= paths.size()) {
				System.out.printf("%c\n", 128544);
				System.exit(0);
			}
		}
		try {
			String command = String.format("subl %s", paths.get(code));
			Process process = runtime.exec(command);
			process.waitFor();
		} catch (Exception e) {
			System.out.println("Fatal error....");
		}
	}
}
