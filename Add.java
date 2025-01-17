package tools;

import java.io.File;
import java.io.RandomAccessFile;
import java.io.IOException;
import java.io.FileNotFoundException;

public class Add {

	private File file;

	public Add(File file) {
		this.file = file;
	}

	public void add(String text) {
		try {
			var file = new RandomAccessFile(this.file, "rw");
			long length = file.length();
			file.seek(length - 1);
			char lastCh = (char) file.read();
			if (lastCh != '\n')
				file.write((int) '\n');
			file.writeBytes(String.format("%s\n", text));
			file.close();
		} catch (FileNotFoundException e) {
			System.out.printf("File '%s' not found.", this.file.getName());
			System.exit(0);
		} catch (IOException e) {
			System.out.printf("%s: File IO Error.", this.file.getName());
			System.exit(0);
		} 
	}

	public static void main(final String[] args) {
		if (args.length != 1) return;
		File file = new File(args[0]);
		Add add = new Add(file);
		for (int i = 1; i < args.length; i++)
			add.add(args[i]);
	}
}