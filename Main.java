import java.io.File;
import java.io.FileWriter;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.FileTime;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.ThreadFactory;
sealed class A permits B {}
non-sealed class B extends A{}
public class Main {
	public static void main(String[] args) throws Exception {
		ThreadFactory factory = Thread.ofVirtual().factory();
		factory.newThread(() -> System.out.println(Thread.currentThread().getName())).start();
		Thread.sleep(1000);
	}
}