import java.util.ArrayList;
import java.util.List;
import java.nio.file.Files;
import java.nio.file.Path;
import java.io.FileWriter;

public class Notes {
	public static void main(final String[] args) throws Exception {
		String fileDir = "/home/dkumar/collection/";
		List<String> notedHtmlFileLines = 
				Files.lines(Path.of(fileDir + "noted.html"))
				.toList();
		long notedHtmlFileLineCount = notedHtmlFileLines.size();
		List<String> notedHtmlFileLinesH = notedHtmlFileLines.stream()
				.limit(notedHtmlFileLineCount - 3)
				.toList();
		List<String> notedHtmlFileLinesT = notedHtmlFileLines.stream()
				.skip(notedHtmlFileLineCount - 3)
				.toList();
		List<String> notedFileLines = 
				Files.lines(Path.of(fileDir + "noted"))
				.toList();
		List<String> finalFileLines = new ArrayList<>();
		finalFileLines.addAll(notedHtmlFileLinesH);
		for (String line: notedFileLines) {
			var html = new StringBuilder();
			html.append("<tr><td>");
			html.append(line);
			html.append("</td></tr>");
			finalFileLines.add(html.toString());
		}
		finalFileLines.addAll(notedHtmlFileLinesT);
		Files.write(Path.of(fileDir + "noted.html"), finalFileLines);
		var fw = new FileWriter(fileDir + "noted", false);
		fw.flush();
		fw.close();
	}
}