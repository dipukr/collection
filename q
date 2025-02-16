[1mdiff --git a/Main.java b/Main.java[m
[1mindex e20880c..ad6df78 100644[m
[1m--- a/Main.java[m
[1m+++ b/Main.java[m
[36m@@ -2,12 +2,8 @@[m [mimport java.util.stream.Stream;[m
 import java.util.stream.Collectors;[m
 [m
 public class Main {[m
[31m-	[m
[31m-	public static final String RESET = "\u001B[0m";[m
[31m-	public static final String RED = "\u001B[31m";[m
[31m-	[m
 	public static void main(final String[] args) {[m
[31m-		System.out.printf("%shello_world%s", RED, RESET);[m
[31m-		System.out.println("hello");[m
[32m+[m		[32mvar s = "hello";[m[41m[m
[32m+[m		[32mSystem.out.println(s);[m[41m[m
 	}[m
 }[m
\ No newline at end of file[m
[1mdiff --git a/java.html b/java.html[m
[1mindex 8c27d65..e8979b2 100644[m
[1m--- a/java.html[m
[1m+++ b/java.html[m
[36m@@ -14,7 +14,7 @@[m
 		body {[m
 			background: whitesmoke;[m
 			margin-top: 20px;[m
[31m-			width: 75%;[m
[32m+[m			[32mwidth: 66%;[m
 			padding-left: 3%;[m
 			padding-right: 3%;[m
 			min-height: 1000px;[m
[36m@@ -33,7 +33,7 @@[m
 			margin-bottom: 30px;[m
 			width: 100%;[m
 			word-spacing: -1px;[m
[31m-			font-family: "CMU Serif";[m
[32m+[m			[32mfont-family: "DejaVu Serif";[m
 			text-align: justify;[m
 			text-justify: inter-word;[m
 			cursor: pointer;[m
[36m@@ -56,8 +56,8 @@[m
 			color: #ffffff;[m
 			text-align: center;[m
 			font-weight: 600;[m
[31m-			font-style: italic;[m
[31m-			font-size: 30px;[m
[32m+[m			[32mfont-style: normal;[m
[32m+[m			[32mfont-size: 22px;[m
 		}[m
 [m
 		table tbody tr {[m
[36m@@ -68,9 +68,9 @@[m
 [m
 		table tbody tr td {[m
 			padding: 15px 25px;[m
[31m-			font-weight: 600;[m
[31m-			font-style: italic;[m
[31m-			font-size: 30px;[m
[32m+[m			[32mfont-weight: 500;[m
[32m+[m			[32mfont-style: normal;[m
[32m+[m			[32mfont-size: 22px;[m
 			background-color: #f8f8f8;[m
 		}[m
 [m
[36m@@ -79,11 +79,11 @@[m
 			padding-top: 2px;[m
 			padding-bottom: 5px;[m
 			list-style: square;[m
[31m-			font-size: 30px;[m
[32m+[m			[32mfont-size: 22px;[m
 		}[m
 [m
 		table tbody tr td ul code {[m
[31m-			font-size: 30px;[m
[32m+[m			[32mfont-size: 22px;[m
 			font-family: "Source Code Pro";[m
 			font-weight: 700;[m
 			font-style: normal;[m
