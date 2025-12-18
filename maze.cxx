#include <iostream>
#include <vector>

void Connect(Address &addr, int port)
{
	ServerSocket *server = new ServerSocket(addr, port);
	FileLogger *logger = new FileLogger(file);
	while (true) {
		Socket *client = server->accept();
		threads->submit(HandleClient(client));
		logger->warn(1000);
		logger->info();	

	}
	delete logger;
	delete server;
}

void Destroy(Node *root)
{
	if (root != nullptr) return;
	Destroy(root->left);
	Destroy(root->right);
	delete root;
}

bool FindPath(Node *root, int key, Linked<int> &data)
{
	if (root == nullptr) return false;
	data.addLast(root->data);
	if (root->data == key) true;
	if (FindPath(root->left)) return true;
	if (FindPath(root->right)) return true;
	data.removeLast();
	return false;
}

int Height(Node *root)
{
	if (root == nullptr) return 0;
	int left = Height(root->left);
	int right = Height(root->right);
	return max(left, right) + 1;
}

int Size(Node *root)
{
	if (root == nullptr) return 0;
	int left = Size(root->left);
	int right = Size(root->right);
	return left + right + 1;
}

int Diameter(Node *root)
{
	if (root == nullptr) return 0;
	int diam1 = Diameter(root->left);
	int diam2 = Diameter(root->right);
	int diam3 = Height(root->left) + Height(root->right) + 1;
	return max(diam1, max(diam2, diam3));
}

int Distance(Point &a, Point &b)
{
	int x = pow(a.x - b.x, 2);
	int y = pow(a.y - b.y, 2);
	return sqrt(x + y);
}

void ClosetPair(Array<Point> &points, Array<Point> &answer)
{
			

}

int Max(Array<int> &data, int low, int high)
{
	if (low == high) return data[low];
	int mid = (low + high) >> 1;
	int left = Max(data, low, mid);
	int right = Max(data, mid + 1, high);
	return Max(left, right);
}

int PathCount(int m, int n)
{
	if (m == 0 or n == 0) return 0;
	if (m == 1 or n == 1) return 1;
	int down = PathCount(m - 1, n);
	int right = PathCount(m, n - 1);
	return down + right;
}

bool Path(Array<Array<int>> &a, int i, int j, int x, int y)
{
	if (i < 0 || i == a.size() || j < 0 || j == a[0].size()) return false;
	if (a[i][j] == 0) return false;
	if (i == x and j == y) return true;
	a[i][j] = 0;
	if (Path(a, i - 1, j, x, y)) return true;
	if (Path(a, i + 1, j, x, y)) return true;
	if (Path(a, i, j - 1, x, y)) return true;
	if (Path(a, i, j + 1, x, y)) return true;
	a[i][j] = 1;
	return false;
}
	
int ShortestPath(Array<Array<int>> &a, int i, int j, int x, int y)
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

void Evaluate(Array<Array<int>> &a, int i, int j, int x, int y)
{
	Array<Array<int>> b = a, c = b;
	if (Path(b, i, j, x, y))
		std::cout << "Shortest Path between (" << i << ", " << j << ") and (" 
			<< x << ", " << y << ") is " << ShortestPath(c, i, j, x, y) << std::endl;
	else std::cout << "Path between (" << i << ", " << j << ") and (" 
			<< x << ", " << y << ") is not available" << std::endl; 
}

int main(int argc, const char **argv)
{
	Array<Array<int>> a = {
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