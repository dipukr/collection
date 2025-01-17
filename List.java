import java.io.File;
import java.util.LinkedList;
import java.util.TreeSet;

public class List {
	public static void main(final String[] args) throws Exception {
		File root = null;
		var root2 = root.getAbsolutePath();
		if (args.length == 0) root = new File(".");
		else if (args.length == 1) root = new File(args[0]);
		else return;
		var dirPaths = new TreeSet<String>();
		var filePaths = new TreeSet<String>();
		var queue = new LinkedList<File>();
		queue.offer(root);
		while (!queue.isEmpty()) {
			File file = queue.poll();
			if (file.isDirectory()) {
				dirPaths.add(file.getCanonicalPath() + "/");
				File[] files = file.listFiles();
				if (files != null)
					for (File elem: files)
						queue.offer(elem);
			} else if (file.isFile()) {
				String filePath = file.getCanonicalPath();
				filePaths.add(filePath);
			}
		}
		dirPaths.stream().forEach(System.out::println);
		filePaths.stream().forEach(System.out::println);
	}
}
