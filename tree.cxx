#include <iostream>
#include <vector>
#include <collib>

struct Node
{
	int data;
	Node *left;
	Node *right;
	Node(int data, Node *left=nullptr, Node *right=nullptr) {
		this->data = data;
		this->left = left;
		this->right = right;
	}
};

int find(int inorder[], int curr, int start, int end)
{
	for (int i = start; i <= end; i++)
		if (inorder[i] == curr)
			return i;
	return -1;
}

Node* BuildTree(int preorder[], int inorder[], int start, int end)
{
	static int i = 0;
	if (start > end) return nullptr;
	int curr = preorder[i++];
	Node *node = new Node(curr);
	if (start == end) return node;
	int pos = Find(inorder, curr, start, end);
	node->left = BuildTree(preorder, inorder, start, pos-1);
	node->right = BuildTree(preorder, inorder, pos+1, end);
}

int Diameter(Node *node)
{
	if (node == nullptr) return 0;
	int leftHeight = Height(node->left);
	int rightHeight = Height(node->right);
	int currDiameter = leftHeight + rightHeight + 1;
	int leftDiameter = Diameter(node->left);
	int rightDiameter = Diameter(node->right);
	return max(currDiameter, max(leftDiameter, rightDiameter));
}

int sumReplace(Node *node)
{
	if (node == nullptr) return 0;
	sumReplace(node->left);
	sumReplace(node->right);
	if (node->left) node->data += node->left->data;
	if (node->right) node->data += node->right->data;
}

void LevelOrder(Node *root)
{
	Queue<Node*> queue;
	queue.enqueue(root);
	while (queue.notEmpty()) {
		Node *node = queue.dequeue();
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
		VisitNode(node);
	}
}

void LevelOrder(Node *root)
{
	int level(1);
	Queue<Node*> queue;
	queue.enqueue(root);
	queue.enqueue(nullptr);
	while (!queue.empty()) {
		Node *node = queue.dequeue();
		if (node != nullptr) {
			if (node->left)
				queue.enqueue(node->left);
			if (node->right)
				queue.enqueue(node->right);
			cout << node->data << " ";
		} else if (!queue.empty()) {
			level++;
			cout << endl;
			queue.enqueue(nullptr);
		}
	}
}

int SumAtKthLevel(TreeNode *root, int k)
{
	int sum(0);
	int level(0);
	Queue<Node*> queue;
	queue.enqueue(root);
	queue.enqueue(nullptr);
	while (!queue.empty()) {
		Node *node = queue.dequeue();
		if (node != nullptr) {
			if (node->left)
				queue.enqueue(node->left);
			if (node->right)
				queue.enqueue(node->right);
			if (level == k)
				sum += node->data;
		} else if (!queue.empty()) {
			level++;
			queue.enqueue(nullptr);
		}
	}
	return sum;
}

bool Balanced(Node *node)
{
	if (node == nullptr) return true;
	if (!Balanced(node->left)) return false;
	if (!Balanced(node->right)) return false;
	return abs(Height(node->left) - Height(node->right)) <= 1;
}

bool balanced2(Node *node)
{
	if (node == nullptr) return true;
	return abs(height(node->left)-height(node->right)) <= 1 and balanced(node->left) and balanced(node->right);
}

bool balanced(Node *node, int *height)
{
	if (node == nullptr) return true;
	int lh = 0, rh = 0;
	if (!balanced(node->left, &lh)) return false;
	if (!balanced(node->right, &rh)) return false;
	*height = max(lh, rh) + 1;
	return abs(lh - rh) <= 1; 
}

int lowestCommonAncestor(Node *node, int n1, int n2)
{
	vector<int> path1, path2;
	if (!getPath(node, n1, path1) || !getPath(node, n2, path2)) return -1;
	int pc = 0;
	for (; pc < path1.size() and path2.size(); pc++)
		if (path1[pc] not_eq path2[pc])
			break;	
	return path1[pc - 1];
}

Node* lowestCommonAncestor2(Node *node, int n1, int n2)
{
	if (node == nullptr) return nullptr;
	if (node->data == n1 or node->data == n2) return node;
	Node *leftLca = lowestCommonAncestor2(node->left, n1, n2);
	Node *rightLca = lowestCommonAncestor2(node->right, n1, n2);
	if (leftLca and rightLca) return node;
	if (leftLca == nullptr) return rightLca;
	return leftLca;
}

void flatten(TreeNode *root)
{
	if (root == nullptr) return;
	if (root->left == nullptr || root->right == nullptr) return;
	if (node->left != nullptr) {
		flatten(node->left);
		Node *save = node->right;
		node->right = node->left;
		node->left = nullptr;
		Node *tail = node->right;
		while (tail->right != nullptr)
			tail = tail->right;
		tail->right = save;
	}
	flatten(node->right);
}