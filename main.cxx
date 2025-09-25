#include <iostream>
#include <coll>

void Subsets(List<List<int>> &subsets, List<int> &subset, List<int> &data, int i)
{
	if (i == data.size()) {
		List<int> list(subset);
		subsets.add(list);
		return;
	}
	subset.addLast(data[i]);
	Subsets(subsets, subset, data, i + 1);
	subset.removeLast();
	Subsets(subsets, subset, data, i + 1);
}

int main(int argc, const char **argv)
{ 
	List<int> list{1,2,3,4};
	List<List<int>> subsets;
	List<int> subset;
	Subsets(subsets, subset, list, 0);
	cout << subsets << endl;
	cout << String::format("hello%d%s\n", 10, "world").cstr();
}