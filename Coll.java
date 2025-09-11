import java.io.File;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;

public class Coll {
	
	private Comparator<File> cmp = Comparator.comparing(File::isDirectory);
	private Set<Integer> levels = new TreeSet<>();
	
	public void draw(String fileName, int level, boolean last) {
		for (int i = 0; i < level * 2; i++) {
			if (i % 2 == 0) {
				int a = (int) i / 2;
				if (levels.contains(a)) System.out.print(" ");
				else System.out.printf("%c", '\u2502');
			} else System.out.print("   ");
		}
		if (last) System.out.printf("%c%c%c %s\n", '\u2514','\u2500','\u2500', fileName);
		else System.out.printf("%c%c%c %s\n", '\u251c','\u2500','\u2500', fileName);
	}
	
	public void treeView(File root, int level) {
		File[] files = root.listFiles();
		Arrays.sort(files, cmp);
		for (int i = 0; i < files.length; i++) {
			File file = files[i];
			if (file.isDirectory()) {
				if (i == (files.length - 1))
					draw(file.getName(), level, true);
				else draw(file.getName(), level, false);
				treeView(file, level + 1);
			} else {
				if (i == (files.length - 1))
					draw(file.getName(), level, true);
				else draw(file.getName(), level, false);
			}
			if (i == (files.length - 1)) levels.add(level);
		}
	}
	
	public static void main(String[] args) {
		File root = new File("/home/dkumar/DSA");
		var tree = new Coll();
		tree.treeView(root, 0);
	}
}