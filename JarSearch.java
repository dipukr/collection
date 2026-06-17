import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipFile;

void main(String[] args) {
    if (args.length < 2) {
        IO.println("Usage: java JarSearch.java <projectDir> <needle> [--no-maven]");
        IO.println("Example: java JarSearch.java TReDS javax/xml/parsers");
        return;
    }

    String projectDir = args[0];
    String needle = args[1];
    boolean runMaven = !Arrays.asList(args).contains("--no-maven");

    File depsDir = new File(projectDir, "target/deps");

    // Step 1: mvn dependency:copy-dependencies
    if (runMaven) {
        IO.println("Running Maven copy-dependencies...");
        int exitCode = runMavenCopy(projectDir);
        if (exitCode != 0) {
            IO.println("Maven failed with exit code " + exitCode);
            return;
        }
        IO.println("Maven finished.");
    }

    if (!depsDir.isDirectory()) {
        IO.println("No deps folder at " + depsDir.getPath());
        return;
    }

    // Step 2: scan each jar for the needle
    scanJars(depsDir, needle);
}

/// Runs the Maven goal in the given project directory, streaming its output.
int runMavenCopy(String projectDir) {
    var pb = new ProcessBuilder("mvn", "dependency:copy-dependencies", "-DoutputDirectory=target/deps");
    pb.directory(new File(projectDir));
    pb.inheritIO(); // show Maven's output directly in this console

    try {
        Process process = pb.start();
        return process.waitFor();
    } catch (IOException e) {
        IO.println("ERROR: could not start Maven ('" + MAVEN + "'). "
                + "Check it is on PATH or set the full path in the MAVEN constant.");
        return -1;
    } catch (InterruptedException e) {
        Thread.currentThread().interrupt();
        IO.println("Maven was interrupted.");
        return -1;
    }
}

/// Lists *.jar files and prints those containing a .class entry matching the needle.
void scanJars(File depsDir, String needle) {
    File[] jars = depsDir.listFiles((dir, name) -> name.endsWith(".jar"));
    if (jars == null || jars.length == 0) {
        IO.println("No jars found in " + depsDir.getPath());
        return;
    }

    Arrays.sort(jars); // stable, alphabetical order
    IO.println("Scanning " + jars.length + " jar(s) for '" + needle + "'...");

    int count = 0;
    for (File jar : jars) {
        if (jarContains(jar, needle)) {
            IO.println(jar.getName());
            count++;
        }
    }

    IO.println("Done. " + count + " jar(s) matched.");
}

boolean jarContains(File jar, String needle) {
    try (ZipFile zf = new ZipFile(jar)) {
        Enumeration<? extends ZipEntry> entries = zf.entries();
        while (entries.hasMoreElements()) {
            String name = entries.nextElement().getName();
            // mimic: grep ".class" then grep -F <needle>
            if (name.endsWith(".class") && name.contains(needle)) {
                return true; // one hit per jar is enough
            }
        }
    } catch (ZipException e) {
        IO.println("Skipping unreadable jar: " + jar.getName());
    } catch (IOException e) {
        IO.println("Error reading " + jar.getName() + ": " + e.getMessage());
    }
    return false;
}