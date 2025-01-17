public class Run {
	public static void main(final String[] args) throws Exception {
		List<String> lines = Files.lines(Path.of("cmd"))
			.toList();
		String cmd = lines.get(0);
		Process runtime = System.getRuntime();
		
	}
}