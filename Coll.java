import java.io.File;

void log(int i) {
	System.out.println(i);
}

void main(String[] args) {
	File file = new File("file");
	System.out.println(file);
	log(200);
}