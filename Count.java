import java.io.File;
import java.util.ArrayList;
import java.util.LinkedList;
import java.nio.file.Files;
import java.nio.file.Path;
import java.io.IOException;
import java.util.function.*;

public class Count {

	public static long count(String filePath) {
		try {
			return Files.lines(Path.of(filePath)).count();
		} catch (Exception e) {
			return 0L;
		}
	}

	public static void main(final String[] args) throws Exception {
		File root = new File(".");
		Predicate<String> pred = null;
		if (args.length == 0) pred = arg -> true;
		else if (args.length == 1) 
			pred = arg -> arg.endsWith(args[0]);
		else return;
		var filePaths = new ArrayList<String>();
		var queue = new LinkedList<File>();
		queue.offer(root);
		while (!queue.isEmpty()) {
			File file = queue.poll();
			if (file.isDirectory()) {
				File[] files = file.listFiles();
				if (files != null)
					for (File elem: files)
						queue.offer(elem);
			} else if (file.isFile()) {
				String filePath = file.getCanonicalPath();
				if (pred.test(filePath))
					filePaths.add(filePath);
			}
		}
		long totalLines = filePaths.stream().mapToLong(Count::count).sum();
		System.out.println(totalLines);
	}
}
