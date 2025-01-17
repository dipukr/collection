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

public class Draw {
	public static void main(String[] args) {
		if (args.length == 1) {
			try {
				Files.lines(Path.of(args[0]))
					.map(line -> line.replaceAll("\t", "    "))
					.forEach(System.out::println);
			} catch (FileNotFoundException e) {
				System.out.printf("File '%s' not found.", args[0]);
			} catch (IOException e) {
				System.out.println("File IO error.");
			}
		}
	}
}
