public class Mat {

	public double[][] data;

	public Mat(int rows, int cols) {
		this.data = new double[rows][cols];
	}
	
	public Mat(double[][] data) {
		this.data = data;
	}

	public void init(double val) {
		for (int i = 0; i < rows(); i++)
			for (int j = 0; j < cols(); j++)
				data[i][j] = val;
	}

	public void randomize() {
		for (int i = 0; i < rows(); i++)
			for (int j = 0; j < cols(); j++)
				data[i][j] = Math.random();
	}
	
	public int rows() {
		return data.length;
	}
	
	public int cols() {
		return data[0].length;
	}
	
	public String shape() {
		return String.format("(%d, %d)", rows(), cols());
	}

	public void draw() {
		for (int row = 0; row < data.length; row++) {
			for (int col = 0; col < data[0].length; col++) {
				int code = data[row][col] == 0.0 ? 32 : 9608;
				System.out.printf("%s%c%c", "\u001B[32m", code,code);
			}
			System.out.println();
		}
	}
	
	public String toString() {
		var text = new StringBuilder();
		for (int i = 0; i < rows(); i++) {
			text.append("[");
			for (int j = 0; j < cols(); j++) {
				text.append(String.format("%.2f", data[i][j]));
				if (j < cols() - 1)
					text.append(" ");
			}
			text.append("]\n");
		} 
		return text.toString();
	}
	
	public static Mat of(double[][] data) {
		return new Mat(data);
	}
}
