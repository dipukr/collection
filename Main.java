import java.util.HashSet;
public class Main {
	public void draw(int[][] data) {

	}
    public void drawMat(int[][] m) {
        if (m == null || m.length == 0) {
            System.out.println("[]");
            return;
        }
        int rows = m.length;
        int cols = 0;
        for (int r = 0; r < rows; r++) cols = Math.max(cols, m[r].length);

        // compute max width per column (treat missing entries as empty)
        int[] colWidth = new int[cols];
        for (int c = 0; c < cols; c++) {
            for (int r = 0; r < rows; r++) {
                if (c < m[r].length) {
                    colWidth[c] = Math.max(colWidth[c], String.valueOf(m[r][c]).length());
                }
            }
        }

        String leftTop  = "\u23A1"; // ⎡
        String leftMid  = "\u23A2"; // ⎢
        String leftBot  = "\u23A3"; // ⎣
        String rightTop = "\u23A4"; // ⎤
        String rightMid = "\u23A5"; // ⎥
        String rightBot = "\u23A6"; // ⎦

        for (int r = 0; r < rows; r++) {
            String L = (r == 0) ? leftTop : (r == rows - 1) ? leftBot : leftMid;
            String R = (r == 0) ? rightTop : (r == rows - 1) ? rightBot : rightMid;
            var sb = new StringBuilder();
            sb.append(L);
            for (int c = 0; c < cols; c++) {
                String cell = (c < m[r].length) ? String.valueOf(m[r][c]) : "";
                sb.append(String.format("%" + colWidth[c] + "s", cell));
                if (c < cols - 1) sb.append(" ");
            }
            sb.append(R);
            System.out.println(sb.toString());
        }
	}

	public int max(int[] data) {
		int answer = 0;
		int subans = 0;
		for (int i = 0; i < data.length; i++) {
			if (data[i] == 1) {
				subans += 1;
			} else {
				answer = Math.max(answer, subans);
				subans = 0;
			}
		}
		return answer;
	}
	public int max(int[] data, int i) {
		
		return 0;
	}
	public int max(int[][] data) {
		int answer = 0;
		
		return answer;
	}
	public static void main(String[] args) throws Exception {
		System.out.println("hellow");
		var main = new Main();
		int[] data = {0,1,1,0,1,1,1,0};
		int[][] mat = {{0,1,1,0,1},{1,1,0,1,0},{0,1,1,1,0},
				{1,1,1,1,0},{1,1,1,1,1},{0,0,0,0,0}};
		System.out.println(main.max(data));
		var set = new HashSet<Integer>();
		set.add(100);
		set.add(200);
		set.add(300);
		var rand = new java.util.Random();
		for(int i=0;i<mat.length;i++)
			for(int j=0;j<mat[0].length;j++)
				mat[i][j]=rand.nextInt(3478);
		//System.out.println(set);
		main.drawMat(mat);
	}
}