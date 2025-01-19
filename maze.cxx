#include <iostream>
#include <vector>

int PathCount(int m, int n)
{
	if (m == 0 || n == 0) return 0;
	if (m == 1 && n == 1) return 1;
	int down = PathCount(m - 1, n);
	int right = PathCount(m, n - 1);
	return down + right;
}

bool Path(std::vector<std::vector<int>> &a, int i, int j, int x, int y)
{
	if (i < 0 || i == a.size() || j < 0 || j == a[0].size()) return false;
	if (a[i][j] == 0) return false;
	if (i == x and j == y) return true;
	a[i][j] = 0;
	if (Path(a, i-1, j, x, y)) return true;
	if (Path(a, i+1, j, x, y)) return true;
	if (Path(a, i, j-1, x, y)) return true;
	if (Path(a, i, j+1, x, y)) return true;
	a[i][j] = 1;
	return false;
}

int ShortestPath(std::vector<std::vector<int>> &a, int i, int j, int x, int y)
{
	if (i < 0 || i == a.size() || j < 0 || j == a[0].size() || a[i][j] == 0) return MAX_INT;
	if (i == x and j == y) return 0;
	a[i][j] = 0;
	int t = ShortestPath(a, i - 1, j, x, y) + 1;
	int b = ShortestPath(a, i + 1, j, x, y) + 1;
	int l = ShortestPath(a, i, j - 1, x, y) + 1;
	int r = ShortestPath(a, i, j + 1, x, y) + 1;
	a[i][j] = 1;
	return min(min(t, b), min(l, r));
}

void FloodFill(int a[][], int m, int n, int r, int c, int tofill, int prevfill)
{
	if (r < 0 || r >= m || c < 0 || c >= n) return;
	if (a[r][c] != prevfill) return;
	a[r][c] = tofill;
	FloodFill(a, m, n, r, c - 1, tofill, prevfill);
	FloodFill(a, m, n, r, c + 1, tofill, prevfill);
	FloodFill(a, m, n, r - 1, c, tofill, prevfill);
	FloodFill(a, m, n, r + 1, c, tofill, prevfill);
}

void Evaluate(std::vector<std::vector<int>> &a, int i, int j, int x, int y)
{
	std::vector<std::vector<int>> b = a, c = b;
	if (Path(b, i, j, x, y))
		std::cout << "Shortest Path between (" << i << ", " << j << ") and (" 
			<< x << ", " << y << ") is " << ShortestPath(c, i, j, x, y) << std::endl;
	else std::cout << "Path between (" << i << ", " << j << ") and (" 
			<< x << ", " << y << ") is not available" << std::endl; 
}

int main(const int argc, const char **argv)
{
	std::vector<std::vector<int>> a = {
		{1,1,1,1,1,0,0,1,1,1},
		{0,1,1,1,1,1,0,1,0,1},
		{0,0,1,0,1,1,1,0,0,1},
		{1,0,1,1,1,0,1,1,0,1},
		{0,0,0,1,0,0,0,1,0,1},
		{1,0,1,1,1,0,0,1,1,0},
		{0,0,0,0,1,0,0,1,0,1},
		{0,1,1,1,1,1,1,1,0,0},
		{1,1,1,1,1,0,0,1,0,1},
		{0,0,1,0,0,1,1,0,0,1}
	};
	Evaluate(a, 0, 0, 7, 1);
	Evaluate(a, 0, 0, 8, 0);
	Evaluate(a, 0, 0, 8, 8);
	Evaluate(a, 0, 0, 9, 9);
}
