import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;

public class Add {

	private final File file;

	public Add(File file) {
		this.file = file;
	}

	public void add(String text) {
		try (var raf = new RandomAccessFile(this.file, "rw")) {
			long length = file.length();
			raf.seek(length - 1);
			char lastCh = (char) raf.read();
			if (lastCh != '\n') raf.write((int) '\n');
			raf.writeBytes(String.format("%s\n", text));
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