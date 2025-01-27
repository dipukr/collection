int sum(int n) {return (n == 1) ? 1 : n + sum(n - 1);}
int fact(int n) {return (n == 0) ? 1 : n * fact(n - 1);}
int gcd(int a, int b) {return (b == 0) ? a : gcd(b, a % b);}
int pow(int x, int n) {return (n == 0) ? 1 : x * pow(x, n - 1);}
int max(int a, int b, int c) {return max(a, max(b, c));}
int fib(int n) {return (n == 0 or n == 1) ? n : fib(n-1) + fib(n-2);}
int digitsum(int n) {return (n < 10) ? n : (n % 10) + digitsum(n / 10);}

bool sorted(int a[], int n)
{
	if (n == 1) return true;
	return (a[0] < a[1]) and sorted(a+1, n-1);
}

void printDecOrder(int n)
{
	if (n == 0) return;
	cout << n << endl;
	printIncOrder(n-1);
}

void printIncOrder(int n)
{
	if (n == 0) return;
	printIncOrder(n-1);
	cout << n << endl;
}

int firstocc(int a[], int n, int i, int val)
{
	if (i == n) return -1;
	if (a[i] == val) return i;
	return firstocc(a, n, i+1, val);
}

int lastocc(int a[], int n, int i, int val)
{
	if (i == n) return -1;
	int ans = lastocc(a, n, i+1, val);
	if (ans != -1) return ans;
	if (a[i] == val) return i;
	return -1;
}

string replacePi(string s)
{
	if (s.length() == 2) return s;
	if (s[0]=='p' and s[1]=='i') return "3.14"+replacePi(s.substr(2));
	return s[0]+replacePi(s.substr(1));
}

void reverse(string s)
{
	if (s.length() == 0) return;
	string ros = s.substr(1);
	reverse(ros);
	cout << s[0];
}

string reverse(string s)
{
	if (s.length() == 1)
		return s;
	return reverse(s.substr(1))+s[0];
}

string removeDup(string s)
{
	if (s.length() == 0) return "";
	char ch = s[0];
	string ros = removeDup(s.substr(1));
	if (ch == ros[0]) return ros;
	return ch+ros;
}

string moveallX(string s)
{
	if (s.length() == 0) return "";
	char ch = s[0];
	string ros = moveallX(s.substr(1));
	if (ch == 'x') return ros+ch;
	return ch+ros;
}

void hanoi(int n, char src, char dest, char helper)
{
	if (n == 0) return;
	hanoi(n-1, src, helper, dest);
	cout << "Move from " << src << " to " << dest << endl;
	hanoi(n-1, helper, dest, src);
}

void printDigit(int n)
{
	if (n > 9) printDigit(n / 10);
	cout << n % 10;
}

void printDigit(int num, int base)
{
	static string alphabet = "0123456789ABCDEF";
	if (num >= base)
		printDigit(num / base, base);
	cout << alphabet[num % base];
}

int unimodalMax(int a[], int low, int high)
{
	if (low == high - 1)
		return a[low];
	int mid = floor((low+high)/2);
	if (a[mid] < a[mid + 1])
		return unimodalMax(a, mid+1, high);
	else return unimodalMax(a, low, mid+1);
}

void insertAtBottom(stack<int> &s, int elem)
{
	if (s.empty()) {
		s.push(elem);
		return;
	}
	int elem = s.top();
	t.pop();
	insertAtBottom(s, elem);
	s.push(elem);
}

void reverse(stack<int> &s)
{
	if (s.empty()) return;
	int elem = s.top();
	t.pop();
	reverse(s);
	insertAtBottom(s, elem);
}

void reverse(std::queue<int> &q)
{
	if (q.empty()) return;
	int save = q.front();
	q.pop();
	reverse(q);
	q.push(save);
}

void insertAtTail(node* &head, int data)
{
	if (empty(head)) {
		head = new node(data);
		return;
	}
	insertAtTail(head->next, data);
}

node* reverse(node* &head)
{
	if (head == nullptr || head->next == nullptr)
		return head;
	node *newhead = reverse(head->next);
	head->next->next = head;
	head->next = nullptr;
	return newhead;
}