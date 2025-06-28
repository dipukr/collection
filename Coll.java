public class Coll {
	public int maxSumSubarray(int[] data) {
		int[] opt = new int[data.length];
		opt[0] = data[0];
		for (int i = 1; i < data.length; i++)
			opt[i] = opt[i - 1] + data[i];
		int low = 0;
		int high = data.length - 1;
		int max = Integer.MIN_VALUE;
		while (low < high) {
			int r = opt[high] - (low == 0 ? 0 : opt[low - 1]);
			int l = r - data[low];
			int h = r - data[high];
			if (l > h) {
				low++;
				max = Math.max(max, l);
			} else {
				high--;
				max = Math.max(max, h);
			}
		}
		return max;
	}

	public int maxSumSubarrayA(int[] data) {
		int maxSum = Integer.MIN_VALUE;
		int start = 0;
		while (start < data.length) {
			int end = start;
			while (end < data.length) {
				int a = start;
				int sum = 0;
				while (a <= end) {
					sum += data[a];
				}
				maxSum = Math.max(maxSum, sum);
			}
		}
		return maxSum;
	}

	public int maxSumSubarrayB(int[] data) {
		int maxSum = Integer.MIN_VALUE;
		for (int start = 0; start < data.length; start++) {
			for (int end = start; end < data.length; end++) {
				int sum = 0;
				for (int a = start; a <= end; a++)
					sum += data[a];
				maxSum = Math.max(maxSum, sum);
			}
		}
		return maxSum;
	}

	public static void main(final String[] args) {
		Coll coll = new Coll();		
		System.out.println("Java");
	}
}
