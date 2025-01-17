import java.nio.file.Files;
import java.nio.file.Path;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.File;
import java.util.List;
import java.util.Queue;
import java.util.ArrayList;
import java.util.Map;
import java.util.Scanner;
import java.util.HashMap;
import java.util.LinkedList;

public class Search {

	private Map<String, List<String>> filePaths = new HashMap<>();
	private List<String> dirs = new ArrayList<>();

	public Search(List<String> dirs) {
		for (String dir: dirs) {
			File root = new File(dir);
			walk(root);
		}
	}

	public List<String> search(String fileName) {
		return filePaths.get(fileName);
	}

	public void walk(final File root) {
		Queue<File> queue = new LinkedList<>();
		queue.offer(root);
		while (queue.isEmpty() == false) {
			File file = queue.poll();
			String filePath = file.getAbsolutePath();
			if (file.isDirectory()) {
				File[] files = file.listFiles();
				if (files == null) continue;
				for (File elem: files)
					queue.offer(elem);
			} else {
				String fileName = file.getName();
				if (filePaths.containsKey(fileName))
					filePaths.get(fileName).add(filePath);	
				else {
					List<String> paths = new ArrayList<>();
					paths.add(filePath);
					filePaths.put(fileName, paths);
				}
			}
		}
	}
}
