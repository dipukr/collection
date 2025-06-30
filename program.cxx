int lis(int a[], int n)
{
	int lis[n];
	lis[0] = 1;
	for (int i = 1; i < n; i++) {
		lis[i] = 1;
		for (int j = 0; j < i; j++)
			if (a[i] > a[j] && lis[i] <= lis[j])
				lis[i] = lis[j] + 1;
	}
	return *max_element(lis, lis+n);
}

string lcs(const string &x, const string &y)
{
	int m = x.length();
	int n = y.length();

	int opt[m+1][n+1] = {0};

	for (int i = m-1; i >= 0; i--)
		for (int j = n-1; j >= 0; j--)
			if (x[i] == y[j])
				opt[i][j] = opt[i+1][j+1] + 1;
			else
				opt[i][j] = max(opt[i+1][j], opt[i][j+1]);

	string result;
	int i = 0;
	int j = 0;
	
	while (i < m && j < n) {
		if (x[i] == y[j]) {
			result += x[i];
			i++;
			j++;
		}
		else if (opt[i+1][j] >= opt[i][j+1]) i++;
		else j++;
	}

	return result;
}

int prefixEvaluation(string s)
{
	stack<int> st;
	for (int i = s.length()-1; i >= 0; i--) {
		if (s[i] >= '0' && s[i] <= '9')
			st.push(s[i]-'0');
		else {
			int a = st.top();
			st.pop();
			int b = st.top();
			st.pop();
			switch (s[i]) {
			case '+': st.push(a+b); break;
			case '-': st.push(a-b); break;
			case '*': st.push(a*b); break;
			case '/': st.push(a/b); break;
			case '^': st.push(pow(a,b)); break;
			}
		}
	}
	return st.top();
}

int postfixEvaluation(string s)
{
	stack<int> st;
	for (int i = 0; i < s.length(); i++) {
		if (s[i] >= '0' && s[i] <= '9')
			st.push(s[i]-'0');
		else {
			int a = st.top();
			st.pop();
			int b = st.top();
			st.pop();
			switch (s[i]) {
			case '+': st.push(a+b); break;
			case '-': st.push(a-b); break;
			case '*': st.push(a*b); break;
			case '/': st.push(a/b); break;
			case '^': st.push(pow(a,b)); break;
			}
		}
	}
	return st.top();
}

int precedence(char ch)
{
	if (ch == '^') return 3;
	else if (ch == '*' || ch == '/') return 2;
	else if (ch == '+' || ch == '-') return 1;
	else return -1;
}

string infixToPostfix(string s)
{
	stack<char> st;
	string result;
	for (int i = 0; i < s.length(); i++) {
		char c = s[i];
		if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
			result += c;
		else if (c == '(')
			st.push(c);
		else if (c == ')') {
			while (!st.empty() && st.top() != '(') {
				result += st.top();
				st.pop();
			}
			if (!st.empty()) st.pop();
		} else { // op
			while (!st.empty() && precedence(st.top()) < precedence(c)) {
				result += st.top();
				st.pop();
			}
			st.push(c);
		}
	}
	while (!st.empty()) {
		result += st.top();
		st.pop();
	}
	return result;
}

string infixToPrefix(string s)
{
	reverse(s.begin(), s.end());
	for(int i = 0; i < s.length(); i++)
		if (s[i] == '(') s[i] = ')';
		else if (s[i] == ')') s[i] = '(';
	string result = infixToPostfix(s);
	reverse(result.begin(), result.end());
	return result;
}

bool redundantParenthesis(string s)
{
	stack<char> st;
	bool answer = false;
	for (char ch: s) {
		if (ch == '(') st.push(ch);
		else if (ch == '+' or ch == '-' or ch == '*' or ch == '/') st.push(ch);
		else if (ch == ')') {
			if (st.top() == '(') answer = true;
			while (st.top() == '+' or st.top() == '-' or st.top() == '*' or st.top() == '/')
				st.pop();
			st.pop();
		}
	}
	return answer;
}

bool threesum(int a[], int n, int target)
{
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)
			for (int k = j + 1; k < n; k++)
				if (a[i] + a[j] + a[k] == target)
					return true;
	return false;
}

bool threesum2(int a[], int n, int target)
{
	sort(a, a+n);
	for (int i = 0; i < n; i++) {
		int low = i + 1;
		int high = n - 1;
		while (low < high) {
			int curr = a[i] + a[low] + a[high];
			if (curr == target) return true;
			if (curr < target) low++;
			else high--;
		}
	}
	return false;
}

int strstr(const string &haystack, const string &needle)
{
	if (needle.size() == 0) return 0;
	if (needle.size() > haystack.size()) return -1;
	for (int i = 0; i <= haystack.size()-needle.size(); i++) {
		bool found = true;
		for (int j = 0; j < needle.size(); j++) {
			if (haystack[i + j] != needle[j]) {
				found = false;
				break;
			}
		}
		if (found) return i;
	}
	return -1;
}

void heapify(vector<double> &A, int N, int index)
{
	int max = index;
	int lhs = 2 * index + 1;
	int rhs = 2 * index + 2;
	if (lhs < N && A[lhs] > A[max]) max = lhs;
	if (rhs < N && A[rhs] > A[max]) max = rhs;
	if (max != index) {
		swap(A[i], A[max]);
		heapify(A, N, max);
	}
}

void heapsort(vector<double> &A, int N)
{
	for (int i = n / 2 - 1; i >= 0; i--)
		heapify(A, N, i);
	for (int i = N - 1; i >= 0; i--) {
		swap(A[0], A[i]);
		heapify(A, i, 0);
	}
}

string removeDup(string s)
{
	if (s.length() == 0) return "";
	char ch = s[0];
	string ros = removeDup(s.substr(1));
	if (ch == ros[0]) return ros;
	return ch+ros;
}