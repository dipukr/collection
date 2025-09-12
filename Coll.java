import java.io.File;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Set;
import java.util.TreeSet;

public class Coll {
	public static void main(String[] args) throws Exception {
		Server server = new Server(8780);
		server.start();
	}
}