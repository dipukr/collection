public class Error {
	public static void fatal(String messge) {
		System.out.println("Fatal error: " + messge);
		System.exit(1);
	}
}