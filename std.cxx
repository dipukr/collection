void merge(std::queue<int> &a, std::queue<int> &b, std::queue<int> &r)
{
	while (!a.empty() && !b.empty()) {
		if (a.peek() < b.peek())
			r.push(a.pop());
		else r.push(b.pop());
	}
	while (!a.empty()) r.push(a.pop());
	while (!b.empty()) r.push(b.pop());
}

void merge(std::list<int> &a, std::list<int> &b, std::list<int> &result)
{
	auto fst = std::begin(a);
	auto snd = std::begin(b);
	while (fst != std::end(a) && snd != std::end(b))
		if (*fst < *snd)
			result.push_back(*fst), fst++;
		else
			result.push_back(*snd), snd++;
	while (fst != std::end(a))
		result.push_back(*fst), fst++;
	while (snd != std::end(b))
		result.push_back(*snd), snd++;
}

void merge(list<int> &a, list<int> &b, list<int> &r)
{
	while (!a.empty() && !b.empty()) {
		if (a.front() < b.front())
			r.push_back(a.front()), a.pop_front();
		else
			r.push_back(b.front()), b.pop_front();
		while (!a.empty())
			r.push_back(a.front()), a.pop_front();
		while (!b.empty())
			r.push_back(b.front()), b.pop_front();
	}
}

void increment(std::list<int> &a, std::list<int> &r)
{
	std::reverse(std::begin(a), std::end(a));
	int carry = 0;
	auto i = a.begin();
	while (i != a.end()) {
		int sum = (i == a.begin()) ? *i + 1: *i + carry;
		carry = sum / 10;
		r.push_back(sum % 10);
		i++;
	}
	if (carry)
		r.push_back(1);
	r.reverse();
	return r;
}

int binary_to_decimal(std::list<int> &a)
{
	std::reverse(std::begin(a), std::end(a));
	auto iter = std::begin(a);
	int f = 1;
	int r = 0;
	while (iter != std::end(a)) {
		int d = *iter;
		r += d * f;
		f = f * 2;
		iter++;
	}
	return r;
}

void add(std::list<int> &a, std::list<int> &b, std::list<int> &r)
{
	std::reverse(std::begin(a), std::end(a));
	std::reverse(std::begin(b), std::end(b));
	auto p = std::begin(a);
	auto q = std::begin(b);
	int carry = 0;
	while (p != std::end(a) || q != std::end(b)) {
		int sum = (p == std::end(a) ? 0 : *p) + (q == std::end(b) ? 0 : *q) + carry;
		carry = sum >= 10 ? 1 : 0;
		sum = sum % 10;
		result.push_back(sum);
		if (p != std::end(a)) p++;
		if (q != std::end(b)) q++;
	}
	if (carry > 0)
		result.push_back(carry);
	std::reverse(std::begin(r), std::end(r));
}

void multiply(std::list<int> &a, std::list<int> &b, std::list<int> &r) 
{
	std::reverse(std::begin(a), std::end(a));
	std::reverse(std::begin(a), std::end(a));
	auto p = std::begin(a);
	auto q = std::begin(b);
	int carry = 0;
	while (p != std::end(a) || q != std::end(b)) {
		int sum = 0;
		carry = sum / 10;
		int digit = sum % 10;
		result.push_back(digit);
		if (p != std::end(a)) p++;
		if (q != std::end(b)) q++;
	}
	if (carry > 0)
		result.push_back(carry);
	std::reverse(std::begin(r), std::end(r));
}

void add(std::list<std::list<int>> &nums, std::list<int> &result)
{
	for (std::list<int> &elem: nums)
		std::reverse(std::begin(elem), std::end(elem));
	std::reverse(std::begin(result), std::end(result));
}

void add(std::list<std::list<int>> &nums, std::list<int> &result)
{
	for (std::list<int> &elem: nums)
		std::reverse(std::begin(elem), std::end(elem));
	std::reverse(std::begin(result), std::end(result));
}

void multiply(std::list<int> &a, std::list<int> &b, std::list<int> &r)
{
	std::reverse(std::begin(a), std::end(a));
	std::reverse(std::begin(a), std::end(a));
    int len = a.size() + b.size() + 1;
    while (len--)
        r.push_back(0);
    auto q = S.begin();
	auto r = R.begin();
    while (q != S.end()) {
        auto s = r;
		auto carry = 0;
		auto p = F.begin();
        while (p != F.end()) {
            int mul = (*p) * (*q) + carry;
            *s += mul % 10;
            carry = mul / 10 + *s / 10;
            p++;
			s++;
        }
        if (carry > 0)
            *s += carry;
        r++;
		q++;
    }
    result.reverse();
    return result;
}

void tokenize(const std::string &text, const std::string &delim, std::vector<std::string> &result)
{
	int start = 0;
	int end = text.find(delim);
	while (end != -1) {
		result.push_back(text.substr(start, end - start));
		start = end + delim.size();
		end = text.find(delim, start);
	}
	result.push_back(text.substr(start, end - start));
}

void timer(uint32_t interval)
{
	uint32_t intervalMS = interval * 1000000;
	clock_t start = clock();
	while (true) {
		clock_t end = clock();
		clock_t elapsed_time = end - start;
		if (elapsed_time >= intervalMS) {
			std::cout << "Ticking...." << std::endl;
			start = end;
		}
	}
}

int main(int argc, const char **argv)
{
	std::cout << "Welcome to C++ Programming" << std::endl;
	return EXIT_SUCCESS;
}