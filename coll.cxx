#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <ostream>
#include <initializer_list>

using std::cout;
using std::endl;

#define null NULL
#define KB 1024
#define MB KB*KB
#define GB KB*KB*KB

#define eval(x) time_t start = clock();\
    x;\
    time_t end = clock();\
    printf("Time elapsed: %d micros.\n", (end - start));

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;
typedef uint32_t uint;
typedef int8_t byte;

class String;

struct Error
{
	static void fatal(const char *what);
	static void error(String &what);
	static void assert(bool flag, const char *what);
};

void Error::fatal(const char *what)
{
	printf("Fatal error: %s\n", what);
	exit(EXIT_FAILURE);
}

void Error::assert(bool flag, const char *what)
{
	if (flag == false) {
		printf("Assertion failed: %s\n", what);
		exit(EXIT_FAILURE);
	}
}

template<class T>
class Stack
{
	struct Node {T data; Node *next;};
	Node *head = nullptr;
	int count = 0;
public:
	Stack();
	Stack(const Stack &arg);
	~Stack();

	void operator=(const Stack &rhs);
	
	void push(const T &elem);
	const T& peek() const;
	T pop();
	
	void reverse();
	void clear();
	
	int size() const;
	bool empty() const;
	bool notEmpty() const;
};

template<class T>
Stack<T>::Stack() {}

template<class T>
Stack<T>::~Stack()
{
	clear();
}

template<class T>
Stack<T>::Stack(const Stack<T> &arg)
{
	for (auto it = arg.head; it != nullptr; it = it->next)
		push(it->data);
	reverse();
}

template<class T>
void Stack<T>::operator=(const Stack<T> &rhs)
{
	if (this == &rhs) return;
	else clear();
	for (auto it = rhs.head; it != nullptr; it = it->next)
		push(it->data);
	reverse();
}

template<class T>
void Stack<T>::push(const T &elem)
{
	Node *node = new Node;
	node->data = elem;
	node->next = head;
	head = node;
	count++;
}

template<class T>
T Stack<T>::pop()
{
	if (empty()) Error::fatal("Stack underflow.");
	T retval = head->data;
	Node *node = head;
	head = head->next;
	delete node;
	count--;
	return retval;
}

template<class T>
const T& Stack<T>::peek() const
{
	if (empty()) Error::fatal("Stack underflow.");
	return head->data;
}

template<class T>
void Stack<T>::clear()
{
	Node *curr = head;
	while (curr != nullptr) {
		Node *next = curr->next;
		delete curr;
		curr = next;
	}
	this->count = 0;
	this->head = nullptr;
}

template<class T>
void Stack<T>::reverse()
{
	Node *prev = nullptr, *curr = head;
	while (curr != nullptr) {
		Node *next = curr->next;
		curr->next = prev;
		prev = curr;
		curr = next;
	}
	head = prev;
}

template<class T>
int Stack<T>::size() const
{
	return count;
}

template<class T>
bool Stack<T>::empty() const
{
	return size() == 0;
}

template<class T>
class Queue
{
	struct Node {T data; Node *next;};
	Node *tail = nullptr;
	Node *head = nullptr;
	int count = 0;
public:
	Queue();
	Queue(const Queue &arg);
	virtual ~Queue();

	void operator=(const Queue &rhs);
	
	void enqueue(const T &elem);
	T dequeue();
	const T& peek() const;
	
	void clear();
	
	int size() const;
	bool empty() const;
};

template<class T>
Queue<T>::Queue() {}

template<class T>
Queue<T>::~Queue()
{
	clear();
}

template<class T>
Queue<T>::Queue(const Queue<T> &arg)
{
	for (auto it = arg.head; it != nullptr; it = it->next)
		enqueue(it->data);
}

template<class T>
void Queue<T>::operator=(const Queue<T> &rhs)
{
	if (this == &rhs) return;
	else clear();
	for (auto it = rhs.head; it != nullptr; it = it->next)
		enqueue(it->data);
}

template<class T>
void Queue<T>::enqueue(const T &elem)
{
	Node *node = new Node;
	node->data = elem;
	if (empty()) {
		head = tail = node;
	} else {
		tail->next = node;
		tail = node;
	}
	count++;
}

template<class T>
T Queue<T>::dequeue()
{
	if (empty()) Error::fatal("Queue underflow.");
	T retval = head->data;
	Node *node = head;
	head = head->next;
	delete node;
	count--;
	return retval;
}

template<class T>
const T& Queue<T>::peek() const
{
	if (empty()) Error::fatal("Queue underflow.");
	return head->data;
}

template<class T>
void Queue<T>::clear()
{
	Node *curr = head;
	while (curr != nullptr) {
		Node *next = curr->next;
		delete curr;
		curr = next;
	}
	head = nullptr;
	tail = nullptr;
	count = 0;
}

template<class T>
int Queue<T>::size() const
{
	return count;
}

template<class T>
bool Queue<T>::empty() const
{
	return size() == 0;
}

template<class T>
class List
{
	T *data = nullptr;
	int count = 0;
	int total = 0;
	
public:
	List(int sz = 2);
	List(int sz, const T &val);
	List(std::initializer_list<T> list);
	List(const List &arg);
	~List();
	
	void operator=(const List &rhs);
	bool operator==(const List &rhs);

	List operator+(const T &rhs);
	List operator+(const List &rhs);
	List operator-(const T &rhs);
	List operator-(const List &rhs);
	
	void operator+=(const T &rhs);
	void operator+=(const List &rhs);
	void operator-=(const T &rhs);
	void operator-=(const List &rhs);

	List& operator<<(const T &rhs);

	T& operator[](int index);
	const T& operator[](int index) const;

	void add(const T &elem);
	void append(const T &elem);
	void addLast(const T &elem);
	void add(const List &array);
	void erase(const T &elem);
	void erase(const List &array);
	void insert(int index, const T &elem);
	void remove(int index);
	void removeLast();
	bool contains(const T &elem);
	void reserve(int sz);
	void clear();

	int size() const;
	bool empty() const;
};

template<class T>
List<T>::List(int sz)
{
	this->count = 0;
	this->total = sz;
	this->data = new T[sz];
}

template<class T>
List<T>::List(int sz, const T &val)
{
	this->count = sz;
	this->total = sz;
	this->data = new T[sz];
	for (int it(0); it < count; it++)
		this->data[it] = val;
}

template<class T>
List<T>::List(std::initializer_list<T> list)
{
	for (auto elem: list)
		add(elem);
}

template<class T>
List<T>::List(const List &arg)
{
	operator=(arg);
}

template<class T>
List<T>::~List()
{
	if (data != nullptr)
		delete[] data;
}

template<class T>
void List<T>::operator=(const List<T> &rhs)
{
	if (this == &rhs) return;
	this->count = rhs.count;
	this->total = rhs.total;
	delete[] this->data;
	this->data = new T[total];
	for (int it(0); it < count; it++)
		this->data[it] = rhs.data[it];
}

template<class T>
bool List<T>::operator==(const List<T> &rhs)
{
	if (this->count != rhs.count) return false;
	for (int it(0); it < count; it++)
		if (this->data[it] != rhs.data[it])
			return false;
	return true;
}

template<class T>
List<T> List<T>::operator+(const T &rhs)
{
	List<T> array(*this);
	array += rhs;
	return array;
}

template<class T>
List<T> List<T>::operator+(const List<T> &rhs)
{
	List<T> array(*this);
	array += rhs;
	return array;
}

template<class T>
List<T> List<T>::operator-(const T &rhs)
{
	List<T> array(*this);
	array -= rhs;
	return array;
}

template<class T>
List<T> List<T>::operator-(const List<T> &rhs)
{
	List<T> array(*this);
	array -= rhs;
	return array;
}

template<class T>
void List<T>::operator+=(const T &rhs)
{
	add(rhs);
}

template<class T>
void List<T>::operator+=(const List<T> &rhs)
{
	add(rhs);
}

template<class T>
void List<T>::operator-=(const T &rhs)
{
	erase(rhs);
}

template<class T>
void List<T>::operator-=(const List<T> &rhs)
{
	erase(rhs);
}

template<class T>
List<T>& List<T>::operator<<(const T &rhs)
{
	add(rhs);
	return *this;
}

template<class T>
void List<T>::add(const T &elem)
{
	if (count == total)
		reserve(total * 2);
	data[count] = elem;
	count += 1;
}

template<class T>
void List<T>::append(const T &elem)
{
	if (count == total)
		reserve(total * 2);
	data[count] = elem;
	count += 1;
}

template<class T>
void List<T>::addLast(const T &elem)
{
	if (count == total)
		reserve(total * 2);
	data[count] = elem;
	count += 1;
}

template<class T>
void List<T>::add(const List<T> &array)
{
	for (int it(0); it < array.size(); it++)
		add(array[it]);
}

template<class T>
void List<T>::erase(const T &elem)
{
	for (int it(0); it < count; it++)
		if (data[it] == elem)
			erase(it);
}

template<class T>
void List<T>::erase(const List<T> &array)
{
	for (int it(0); it < array.size(); it++)
		erase(array[it]);
}

template<class T>
void List<T>::insert(int index, const T &elem)
{
	if (index >= count) Error::fatal("List index out of bound.");
	if (count == total) reserve(total * 2);
	for (int it = count; it > index; it--)
		data[it] = data[it - 1];
	data[index] = elem;
	count++;
}

template<class T>
void List<T>::remove(int index)
{
	if (index >= count) Error::fatal("List index out of bound1.");
	count--;
	for (int it = index; it < count; it++)
		data[it] = data[it + 1];
}

template<class T>
void List<T>::removeLast()
{
	if (empty()) Error::fatal("List is empty.");
	count -= 1;
}

template<class T>
bool List<T>::contains(const T &elem)
{
	for (int it(0); it < count; it++)
		if (data[it] == elem)
			return true;
	return false;
}

template<class T>
void List<T>::reserve(int sz)
{
	T *temp = data;
	data = new T[sz];
	for (int it(0); it < count; it++)
		data[it] = temp[it];
	total = sz;
	delete[] temp;
}

template<class T>
void List<T>::clear()
{
	if (data != nullptr) delete[] data;
	count = 0;
	total = 0;
}

template<class T>
T& List<T>::operator[](int index)
{
	if (index >= count) Error::fatal("List index out of bound2.");
	return data[index];
}

template<class T>
const T& List<T>::operator[](int index) const
{
	if (index >= count) Error::fatal("List index out of bound3.");
	return data[index];
}

template<class T>
int List<T>::size() const
{
	return count;
}

template<class T>
bool List<T>::empty() const
{
	return size() == 0;
}

template<class T>
class Deque
{
	struct Node {T data; Node *next;};	
	Node *head = nullptr;
	Node *tail = nullptr;
	int count = 0;
public:
	Deque();
	Deque(const Deque &arg);
	~Deque();
	
	void operator=(const Deque &rhs);
	
	void operator+=(const T &rhs);
	void operator+=(const Deque &rhs);
	void operator-=(const T &rhs);
	void operator-=(const Deque &rhs);
	
	void addFirst(const T &elem);
	void addLast(const T &elem);
	
	T removeFirst();
	T removeLast();
	
	const T& first() const;
	const T& last() const;
	
	bool contains(const T &elem);
	void erase(const T &elem);
	void clear();
	void reverse();
	
	int size() const;
	bool empty() const;
};

template<class T>
Deque<T>::Deque() {}

template<class T>
Deque<T>::Deque(const Deque<T> &arg)
{
	operator=(arg);
}

template<class T>
Deque<T>::~Deque()
{
	clear();
}

template<class T>
void Deque<T>::operator=(const Deque<T> &rhs)
{
	if (this == &rhs) return;
	else clear();
	for (auto it = rhs.head; it != nullptr; it = it->next)
		addLast(it->data);
}

template<class T>
void Deque<T>::operator+=(const T &rhs)
{
	addLast(rhs);
}

template<class T>
void Deque<T>::operator+=(const Deque<T> &rhs)
{
	for (auto it = rhs.head; it != nullptr; it = it->next)
		addLast(it->data);
}

template<class T>
bool Deque<T>::contains(const T &elem)
{
	for (auto iter = head; iter != nullptr; iter = iter->next)
		if (iter->data == elem)
			return true;
	return false;
}

template<class T>
void Deque<T>::erase(const T &elem)
{
	if (empty()) Error::fatal("Deque is empty.");
	if (head->data == elem) {
		removeFirst();
		return;
	}
	Node *curr = head;
	while (curr->next != nullptr) {
		if (curr->next->data == elem) {
			Node *temp = curr->next;
			curr->next = curr->next->next;
			delete temp;
			count--;
			break;
		} else curr = curr->next;
	}
}

template<class T>
void Deque<T>::addFirst(const T &elem)
{
	Node *node = new Node;
	node->data = elem;
	node->next = nullptr;
	if (empty()) {
		head = tail = node;
	} else {
		node->next = head;
		head = node;
	}
	count++;
}

template<class T>
void Deque<T>::addLast(const T &elem)
{
	Node *node = new Node;
	node->data = elem;
	node->next = nullptr;
	if (empty()) {
		head = tail = node;
	} else {
		tail->next = node;
		tail = node;
	}
	count++;
}

template<class T>
T Deque<T>::removeFirst()
{
	if (empty()) Error::fatal("Deque is empty.");
	T retval = head->data;
	Node *node = head;
	head = head->next;
	if (head == nullptr)
		tail = nullptr;
	delete node;
	count--;
	return retval;
}

template<class T>
T Deque<T>::removeLast()
{
	if (empty()) Error::fatal("Deque is empty.");
	T retval = tail->data;
	erase(retval);
	return retval;
}

template<class T>
void Deque<T>::clear()
{
	while (!empty())
		removeFirst();
}

template<class T>
const T& Deque<T>::first() const
{
	if (empty()) Error::fatal("Deque is empty.");
	return head->data;
}

template<class T>
const T& Deque<T>::last() const
{
	if (empty()) Error::fatal("Deque is empty.");
	return tail->data;
}

template<class T>
int Deque<T>::size() const
{
	return count;
}

template<class T>
bool Deque<T>::empty() const
{
	return size() == 0;
}

template<class T>
class Set
{
	struct TreeNode
	{
		T data;
		int height = 0;
		int factor = 0;
		TreeNode *left = nullptr;
		TreeNode *right = nullptr;
		TreeNode(T data) : data(data) {}
	};
	
	TreeNode *root = nullptr;
	int count = 0;

	bool contains(TreeNode *node, const T &data) const
	{
		if (node == nullptr) return false;
		if (data < node->data) return contains(node->left, data);
		else if (data > node->data) return contains(node->right, data);
		else return true;
	}

	void update(TreeNode *node)
	{
		int lh = (node->left == nullptr) ? -1 : node->left->height;
		int rh = (node->right == nullptr) ? -1 : node->right->height;
		node->height = (lh > rh ? lh : rh) + 1;
		node->factor = rh - lh;
	}

	TreeNode* balance(TreeNode *root)
	{
		if (root->factor == -2) {
			if (root->left->factor <= 0)
	    		return leftLeftCase(root);
	    	return leftRightCase(root);
		} else if (root->factor == +2) {
			if (root->right->factor >= 0)
				return rightRightCase(root);
			return rightLeftCase(root);	
		}
		return root;
	}

	TreeNode* insert(TreeNode *root, const T &data)
	{
		if (root == nullptr)
			return new TreeNode(data);
		if (data > root->data)
			root->right = insert(root->right, data);
		else root->left = insert(root->left, data);
		update(root);
		return balance(root);
	}

	TreeNode* erase(TreeNode *root, const T &data)
	{
		if (root == nullptr) return root;
		if (data < root->data)
			root->left = erase(root->left, data);
		else if (data > root->data) 
			root->right = erase(root->right, data);
		else {
			if (root->left == nullptr and root->right == nullptr) {
				delete root;
				return nullptr;
			} else if (root->left == nullptr) {
				delete root;
				return root->right;
			} else if (root->right == nullptr) {
				delete root;
				return root->left;
			} else {
				TreeNode *minNode = findMin(root->right);
				root->data = minNode->data;
				root->right = erase(root->right, minNode->data);
			}
		}
		update(root);
		return balance(root);
	}

	TreeNode* leftLeftCase(TreeNode *node)
	{
		return rightRotation(node);
	}

	TreeNode* rightRightCase(TreeNode *node)
	{
		return leftRotation(node);
	}
	
	TreeNode* leftRightCase(TreeNode *node)
	{
		node->left = leftRotation(node->left);
		return rightRotation(node);
	}

	TreeNode* rightLeftCase(TreeNode *node)
	{
		node->right = rightRotation(node->right);
		return leftRotation(node);
	}

	TreeNode* leftRotation(TreeNode *node)
	{
		TreeNode *parent = node->right;
		node->right = parent->left;
		parent->left = node;
		update(node);
		update(parent);
		return parent;
	}

	TreeNode* rightRotation(TreeNode *node)
	{
		TreeNode *parent = node->left;
		node->left = parent->right;
		parent->right = node;
		update(node);
		update(parent);
		return parent;
	}

	TreeNode* findMin(TreeNode *root) const
	{
		while (root->left != nullptr)
			root = root->left;
		return root;
	}

	TreeNode* findMax(TreeNode *root) const
	{
		while (root->right != nullptr)
			root = root->right;
		return root;	
	}
public:
	Set();
	Set(const Set &arg);
	~Set();
	
	void operator=(const Set &rhs);

	Set operator+(const T &rhs);
	Set operator+(const Set &rhs);
	Set operator-(const T &rhs);
	Set operator-(const Set &rhs);
	
	void operator+=(const T &rhs);
	void operator+=(const Set &rhs);
	void operator-=(const T &rhs);
	void operator-=(const Set &rhs);

	bool contains(const T &data) const;
	void insert(const T &data);
	void erase(const T &data);

	T findMin() const;
	T findMax() const;

	void clear();
	List<T> traverse() const;

	int size() const;
	int height() const;
	bool empty() const;
};

template<class T>
Set<T>::Set() {}

template<class T>
Set<T>::~Set<T>()
{
	if (!empty())
		clear();
}

template<class T>
Set<T>::Set(const Set &arg)
{
	if (this == &arg) return;
	Queue<TreeNode*> queue;
	queue.enqueue(arg.root);
	while (queue.notEmpty()) {
		TreeNode *node = queue.dequeue();
		insert(node->data);
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
	}
}

template<class T>
void Set<T>::operator=(const Set<T> &rhs)
{
	if (this == &rhs) return;
	clear();
	Queue<TreeNode*> queue;
	queue.enqueue(rhs.root);
	while (queue.notEmpty()) {
		TreeNode *node = queue.dequeue();
		insert(node->data);
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
	}
}

template<class T>
Set<T> Set<T>::operator+(const T &rhs)
{
	Set<T> set(*this);
	set += rhs;
	return set;
}

template<class T>
Set<T> Set<T>::operator+(const Set<T> &rhs)
{
	Set<T> set(*this);
	set += rhs;
	return set;
}

template<class T>
Set<T> Set<T>::operator-(const T &rhs)
{
	Set<T> set(*this);
	set -= rhs;
	return set;
}

template<class T>
Set<T> Set<T>::operator-(const Set<T> &rhs)
{
	Set<T> set(*this);
	set -= rhs;
	return set;
}

template<class T>
void Set<T>::operator+=(const T &rhs)
{
	insert(rhs);
}

template<class T>
void Set<T>::operator+=(const Set<T> &rhs)
{
	Queue<TreeNode*> queue;
	queue.enqueue(rhs.root);
	while (queue.notEmpty()) {
		TreeNode *node = queue.dequeue();
		insert(node->data);
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
	}
}

template<class T>
void Set<T>::operator-=(const T &rhs)
{
	erase(rhs);
}

template<class T>
void Set<T>::operator-=(const Set<T> &rhs)
{
	Queue<TreeNode*> queue;
	queue.enqueue(rhs.root);
	while (queue.notEmpty()) {
		TreeNode *node = queue.dequeue();
		erase(node->data);
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
	}
}

template<class T>
void Set<T>::insert(const T &data)
{
	if (!contains(data)) {
		root = insert(root, data);
		count++;
	}
}

template<class T>
bool Set<T>::contains(const T &data) const
{
	return contains(root, data);
}

template<class T>
void Set<T>::erase(const T &data)
{
	if (contains(data)) {
		root = erase(root, data);
		count--;
	}
}

template<class T>
T Set<T>::findMin() const
{
	if (empty()) Error::fatal("Tree is empty.");
	return findMin(root)->data;	
}

template<class T>
T Set<T>::findMax() const
{
	if (empty()) Error::fatal("Tree is empty.");
	return findMax(root)->data;	
}

template<class T>
void Set<T>::clear()
{
	Queue<TreeNode*> queue;
	queue.enqueue(root);
	while (queue.notEmpty()) {
		TreeNode *node = queue.dequeue();
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
		delete node;
	}
	this->root = nullptr;
	this->count = 0;
}

template<class T>
List<T> Set<T>::traverse() const
{
	List<T> array(size());
	Queue<TreeNode*> queue;
	queue.enqueue(root);
	while (queue.notEmpty()) {
		TreeNode *node = queue.dequeue();
		array.add(node->data);
		if (node->left != nullptr) queue.enqueue(node->left);
		if (node->right != nullptr) queue.enqueue(node->right);
	}
	return array;
}

template<class T>
int Set<T>::size() const
{
	return count;
}

template<class T>
int Set<T>::height() const
{
	return root == nullptr ? -1 : root->height;
}

template<class T>
bool Set<T>::empty() const
{
	return size() == 0;
}

class String
{
	char *chars = nullptr;
	int total = 0;
	int count = 0;	
public:
	String(char c);
	String(int size);
	String(int size, char c);
	String(const char *str = "");
	String(const String &arg);
	
	virtual ~String();

	void operator=(const String &rhs);
	void operator+=(const String &rhs);

	String operator+(const String &rhs);

	char& operator[](int index);
	char operator[](int index) const;
	
	String substr(int start, int end) const;
	String sub(int start, int sz) const;
	String rightPart(int start) const;
	String leftPart(int end) const;

	int findFirst(char c) const;
	int findLast(char c) const;
	
	void resize(int sz);
	void clear();

	int size() const {return count;}
	bool empty() const {return size() == 0;}
	const char* cstr() const {return chars;}

	static const int NO_POS = -1;
	static String format(const char *fmt, ...);

	bool operator==(const String &rhs) const {return strcmp(this->chars, rhs.chars) == 0;}
	bool operator!=(const String &rhs) const {return strcmp(this->chars, rhs.chars) != 0;}
	bool operator< (const String &rhs) const {return strcmp(this->chars, rhs.chars) <  0;}
	bool operator<=(const String &rhs) const {return strcmp(this->chars, rhs.chars) <= 0;}
	bool operator> (const String &rhs) const {return strcmp(this->chars, rhs.chars) >  0;}
	bool operator>=(const String &rhs) const {return strcmp(this->chars, rhs.chars) >= 0;}
};

String::String(char c)
{
	count = 1;
	total = count + 1;
	chars = new char[total];
	chars[0] = c;
	chars[1] = 0;
}

String::String(int sz)
{
	count = 0;
	total = sz;
	chars = new char[total];
	chars[0] = 0;
}

String::String(int size, char c)
{
	count = size;
	total = size + 1;
	chars = new char[total];
	memset(chars, c, count);
	chars[count] = 0;
}

String::String(const char* str)
{
	count = strlen(str);
	total = count + 1;
	chars = new char[total];
	strcpy(chars, str);
}

String::String(const String &arg)
{
	count = arg.size();
	total = count + 1;
	chars = new char[total];
	strcpy(chars, arg.chars);
}

String::~String()
{
	if (chars != nullptr)
		delete[] chars;
}

void String::operator=(const String &rhs)
{
	if (this != &rhs) {
		delete[] chars;
		count = rhs.count;
		total = rhs.total;
		chars = new char[total];
		strcpy(chars, rhs.chars);
	}
}

void String::operator+=(const String &rhs)
{
	int ncount = count + rhs.count;
	if (ncount >= total) {
		total = ncount * 2;
		char *ptr = chars;
		chars  = new char[total];
		strcpy(chars, ptr);
		delete[] ptr;
	}
	strcpy(chars + count, rhs.chars);
	count = ncount;
}

String String::operator+(const String &rhs)
{
	String result(*this);
	result += rhs;
	return result;
}

char& String::operator[](int index)
{
	if (index >= count) Error::fatal("String index out of bound.");
	return chars[index];
}

char String::operator[](int index) const
{
	if (index >= count) Error::fatal("String index out of bound.");
	return chars[index];
}

void String::clear()
{
	count = 0;
	total = 0;
	chars[0] = 0;
}

void String::resize(int sz)
{
	total = sz;
	if (sz < count) count = sz;
	char *pt = chars;
	chars = new char[sz];
	memset(chars, 0, sz);
	memcpy(chars, pt, count);
}

int String::findFirst(char c) const
{
	if (c == 0) return String::NO_POS;
	char *ptr = strchr(chars, c);
	return ptr != nullptr ? ptr - chars : String::NO_POS;
}

int String::findLast(char c) const
{
	if (c == 0) return String::NO_POS;
	char *ptr = strrchr(chars, c);
	return ptr != nullptr ? ptr - chars: String::NO_POS;
}

String String::substr(int start, int end) const
{
	if (start >= count or end >= count) Error::fatal("String index out of bound.");
	int len = end - start;
	String str(len, 0);
	memcpy(str.chars, chars + start, len);
	return str;
}

String String::sub(int start, int sz) const
{
	if (start + sz > count) Error::fatal("String index out of bound.");
	String str(sz, 0);
	memcpy(str.chars, chars + start, sz);
	return str;
}

String String::leftPart(int end) const 
{
	if (end >= count) Error::fatal("String index out of bound.");
	return substr(0, end);
}

String String::rightPart(int start) const
{
	if (start >= count) Error::fatal("String index out of bound.");
	return substr(start, count - 1);
}

String String::format(const char *fmt, ...)
{
	char buf[256];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 256, fmt, args);
	va_end(args);
	return buf;
}

void Error::error(String &what)
{
	printf("ERROR: %s\n", what.cstr());
	exit(EXIT_FAILURE);
}

template<class T>
std::ostream& operator<<(std::ostream &lhs, const List<T> &rhs)
{
	lhs << "[";
	for (int i = 0; i < rhs.size(); i++) {
		if (i > 0) lhs << ", ";
		lhs << rhs[i];
	}
	lhs << "]";
	return lhs;
}

template<class T>
class BinaryHeap
{
	Array<T> data;

	void moveUP();
	void moveDown();
	void swap(int i, int j);
	bool hasParent(int i) const;
	bool hasLeft(int i) const;
	bool hasRight(int i) const;
	int left(int i) const;
	int right(int i) const;
	int parent(int i) const;
public:
	BinaryHeap();
	virtual ~BinaryHeap();

	void insert(const T &elem);
	const T& getMin() const;
	T deleteMin();
	
	int size() const;
	bool empty() const;
	bool notEmpty() const;
};


template<class T>
BinaryHeap<T>::BinaryHeap() {}

template<class T>
BinaryHeap<T>::~BinaryHeap() {}

template<class T>
void BinaryHeap<T>::insert(const T &elem)
{
	data.add(elem);
	moveUP();
}

template<class T>
T BinaryHeap<T>::deleteMin()
{
	T result = getMin();
	data[0] = data[data.size() - 1];
	data.remove(data.size() - 1);
	moveDown();
	return result;
}

template<class T>
const T& BinaryHeap<T>::getMin() const
{
	if (empty()) Error::fatal("Heap underflow.");
	return data[0];
}

template<class T>
void BinaryHeap<T>::moveUP()
{
	int index = data.size() - 1;
	while (hasParent(index) && data[index] < data[parent(index)]) {
		swap(index, parent(index));
		index = parent(index);
	}
}

template<class T>
void BinaryHeap<T>::moveDown()
{
	int index = 0;
	while (hasLeft(index)) {
		int lhs = left(index);
		int rhs = right(index);
		int smaller = lhs;
		if (hasRight(index) && data[rhs] < data[lhs])
			smaller = rhs;
		if (data[smaller] < data[index])
			swap(smaller, index);
		else break;
		index = smaller;
	}
}

template<class T>
void BinaryHeap<T>::swap(int i, int j)
{
	T temp = data[i];
	data[i] = data[j];
	data[j] = temp;
}

template<class T>
bool BinaryHeap<T>::hasParent(int i) const
{
	return i > 0;
}

template<class T>
bool BinaryHeap<T>::hasLeft(int i) const
{
	return left(i) < data.size();
}

template<class T>
bool BinaryHeap<T>::hasRight(int i) const
{
	return right(i) < data.size();
}

template<class T>
int BinaryHeap<T>::left(int i) const
{
	return 2 * i + 1;
}

template<class T>
int BinaryHeap<T>::right(int i) const
{
	return 2 * i + 2;
}

template<class T>
int BinaryHeap<T>::parent(int i) const
{
	return (i - 1) / 2;
}

template<class T>
int BinaryHeap<T>::size() const
{
	return data.size();
}

template<class T>
bool BinaryHeap<T>::empty() const
{
	return data.size() == 0;
}

template<class T>
bool BinaryHeap<T>::notEmpty() const
{
	return !empty();
}

int hash(int key) {return int(key);}
int hash(float key) {return int(key);}
int hash(double key) {return int(key);}
int hash(const String &str) {
	int h = 0;
	for (int it = 0; it < str.size(); it++)
		h = 31 * h + str[it];
	return (h & 0x7fffffff);
}

template<class K, class V>
class Map
{
	struct Node
	{
		K key;
		V val;
		Node *next;
		Node(K key, V val) {
			this->key = key;
			this->val = val;
			this->next = nullptr;
		}
	};
	Node** lists = nullptr;
	int listCount = 0;
	int count = 0;
public:
	Map();
	void put(const K &key, const V &val);
	bool hasKey(const K &key);
	void erase(const K &key);

	V get(const K &key);
	V getOrDefault(const K &key, const V &val);
	
	int size() const;
	bool empty() const;
	bool notEmpty() const;
};

template<class K, class V>
Map<K, V>::Map()
{
	listCount = 8;
	lists = new Node*[listCount];
	for (int it(0); it < listCount; it++)
		lists[it] = nullptr;
}

template<class K, class V>
void Map<K, V>::put(const K &key, const V &val)
{
	int h = hash(key) % listCount;
	Node *node = new Node(key, val);
	if (lists[h] == nullptr) lists[h] = node;
	else node->next = lists[h], lists[h] = node;
	count++;
}

template<class K, class V>
bool Map<K, V>::hasKey(const K &key)
{
	int h = hash(key) % listCount;
	for (auto it = lists[h]; it != nullptr; it = it->next)
		if (it->key == key) return true;
	return false;
}

template<class K, class V>
void Map<K, V>::erase(const K &key)
{
	int h = hash(key) % listCount;
	if (lists[h] == nullptr) return;
	if (lists[h]->next == nullptr) {
		delete lists[h];
		lists[h] = nullptr;
		return;
	}
	Node *curr = lists[h], *prev = nullptr;
	while (curr != nullptr) {
		Node *next = curr->next;
		if (curr->key == key) {
			prev->next = curr->next;
			delete curr;
			count--;
			break;
		}
		curr = next;
		prev = curr;
	}
}

template<class K, class V>
V Map<K, V>::get(const K &key)
{
	int h = hash(key) % listCount;
	for (auto it = lists[h]; it != nullptr; it = it->next)
		if (it->key == key) return it->val;
	return V();
}

template<class K, class V>
V Map<K, V>::getOrDefault(const K &key, const V &val)
{
	int h = hash(key) % listCount;
	for (auto it = lists[h]; it != nullptr; it = it->next)
		if (it->key == key) return it->val;
	return val;
}

template<class K, class V>
int Map<K, V>::size() const
{
	return count;
}

template<class K, class V>
bool Map<K, V>::empty() const
{
	return size() == 0;
}

template<class K, class V>
bool Map<K, V>::notEmpty() const
{
	return !empty();
}

class file_reader
{
	FILE *stream = nullptr;
	size_t size = 0;

public:
	file_reader(const char *file);
	~file_reader();

	void seek(U64 loc);
	U64 file_size();
	void close();
	
	void read(byte &val);
	void read(ubyte  &val);
	void read(short &val);
	void read(ushort &val);
	void read(int &val);
	void read(uint &val);
	void read(long &val);
	void read(ulong &val);
	void read(float &val);
	void read(double &val);
	void read(String* &v);
	void read(Value &v);
	void read(Code* &v);
	void read(Class* &v);
	void read(char *dest, size_t sz);	
};

file_reader::file_reader(const char *file)
{
	stream = fopen(file, "rb");
	assert(stream !=  nullptr);
	fseek(stream, 0, SEEK_END);
	size = (U64) ftell(stream);
	fseek(stream, 0, SEEK_SET);
}

file_reader::~file_reader()
{
	fclose(stream);
}

void file_reader::read(S8  &v) {read((char*) &v, sizeof(v));}
void file_reader::read(U8  &v) {read((char*) &v, sizeof(v));}
void file_reader::read(S16 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(U16 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(S32 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(U32 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(S64 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(U64 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(F32 &v) {read((char*) &v, sizeof(v));}
void file_reader::read(F64 &v) {read((char*) &v, sizeof(v));}

void file_reader::read(value &v)
{
	read(v.type);
	switch (v.type) {
	case Data::I64: read(v.i64); break;
	case Data::F64: read(v.f64); break;
	case Data::STR: read(v.str);  break;
	case Data::FUN: read(v.fun); break;
	case Data::CLS: read(v.cls); break;
	default: assert(false);
	}
}

void file_reader::read(text* &v)
{
	char str[1024];
	U32 len(0);
	read(len);
	assert(len < 1024);
	read(str, len);
	str[len] = 0;
	v = new text(str);
}

void file_reader::read(code* &v)
{
	v = (code*) malloc(sizeof(code));
	v->is_native = 0;
	read(v->id);
	read(v->argc);
	read(v->varc);
	read(v->addr);
}



void FileReader::read(Class* &v)
{
	v = (Class*) malloc(sizeof(clas_s));
	read(v->id);
	read(v->data_count);
}

void file_reader::read(char *dest, U64 sz) 
{
	U64 rd = fread(dest, sz, 1, stream);
	assert(rd == 1);
}

void file_reader::seek(U64 pos)
{
	assert(pos < size);
	fseek(stream, pos, SEEK_SET);
}

int file_reader::file_size()
{
	return size;
}

void file_reader::close()
{
	fclose(stream);
}

mem_reader::mem_reader(U8 *src, U64 sz)
{
	mem = src;
	loc = mem;
	mem_total = sz;
	assert(mem != nullptr);
}

void mem_reader::read(S8 &val)
{
	val = *(S8*) loc;
	loc += sizeof(S8);
}

void mem_reader::read(U8 &val)
{
	val = *(U8*) loc;
	loc += sizeof(U8);
}

void mem_reader::read(S16 &val)
{
	val = *(S16*) loc;
	loc += sizeof(S16);
}

void mem_reader::read(U16 &val)
{
	val = *(U16*) loc;
	loc += sizeof(U16);
}

void mem_reader::read(S32 &val)
{
	val = *(S32*) loc;
	loc += sizeof(S32);
}

void mem_reader::read(U32 &val)
{
	val = *(U32*) loc;
	loc += sizeof(U32);
}

void mem_reader::read(S64 &val)
{
	val = *(S64*) loc;
	loc += sizeof(S64);
}

void mem_reader::read(U64 &val)
{
	val = *(U64*) loc;
	loc += sizeof(U64);
}

void mem_reader::read(F32 &val)
{
	val = *(F32*) loc;
	loc += sizeof(F32);
}

void mem_reader::read(F64 &val)
{
	val = *(F64*) loc;
	loc += sizeof(F64);
}

void mem_reader::read(char* &s)
{
	U32 sz = 0;
	read(sz);
	s = (char*) malloc(sz + 1);
	for (U32 a = 0; a < sz; a++)
		*(s + a) = loc[a];
	s[sz] = 0;
	loc += sz;
}

class FileWriter
{
	FILE *stream = nullptr;

public:
	FileWriter(const char *file);
	~FileWriter();
	void close();
	
	void write(S8  val);
	void write(U8  val);
	void write(S16 val);
	void write(U16 val);
	void write(S32 val);
	void write(U32 val);
	void write(S64 val);
	void write(U64 val);
	void write(F32 val);
	void write(F64 val);
	void write(String &v);
	void write(const char *src, U64 sz);
};

class MemoryWriter
{
	U8 *mem = nullptr;
	U8 *loc = nullptr;
	U64 mem_total = 0;

public:
	mem_writer(U64 sz);
	~mem_writer();

	U8* get_mem();
	U64 size();
	void reset();

	void write(S8  val);
	void write(U8  val);
	void write(S16 val);
	void write(U16 val);
	void write(S32 val);
	void write(U32 val);
	void write(S64 val);
	void write(U64 val);
	void write(F32 val);
	void write(F64 val);
	void write(char* s);

};

FileWriter::FileWriter(const char *filename)
{
	stream = fopen(filename, "wb");
	if (stream == nullptr)
		fatal("file open error");
}

FileWriter::~FileWriter()
{
	fclose(stream);
}



void FileWriter::write(S8  v) {write((char*) &v, sizeof(v));}
void FileWriter::write(U8  v) {write((char*) &v, sizeof(v));}
void FileWriter::write(S16 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(U16 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(S32 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(U32 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(S64 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(U64 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(F32 v) {write((char*) &v, sizeof(v));}
void FileWriter::write(F64 v) {write((char*) &v, sizeof(v));}

void FileWriter::write(text &v)
{
	U32 sz = v.size();
	write(sz);
	write((char*) v.cstr(), sz);
}

void FileWriter::write(value &v)
{

}

void FileWriter::write(code *v)
{

}

void FileWriter::write(clas_s *v)
{

}

void FileWriter::write(const char *src, U64 sz)
{
	U64 wr = fwrite(src, sz, 1, stream);
	assert(wr == 1);
}

void FileWriter::close()
{
	fclose(stream); 
}

MemWriter::MemWriter(U64 sz)
{
	mem = (U8*) malloc(sz);
	assert(mem != nullptr);
	loc = mem;
	mem_total = sz;
}

MemWriter::~MemWriter()
{
	free(mem);
	loc = nullptr;
	mem = nullptr;
}

U8* MemWriter::get_mem()
{
	return mem;
}

U64 MemWriter::size()
{
	return mem_total;
}

void MemWriter::write(S8 val)
{
	*(S8*) loc = val;
	loc += sizeof(S8);
}

void MemWriter::write(U8 val)
{
	*(U8*) loc = val;
	loc += sizeof(U8);
}

void MemWriter::write(S16 val)
{
	*(S16*) loc = val;
	loc += sizeof(S16);
}

void MemWriter::write(U16 val)
{
	*(U16*) loc = val;
	loc += sizeof(U16);
}

void MemWriter::write(S32 val)
{
	*(S32*) loc = val;
	loc += sizeof(S32);
}

void MemWriter::write(U32 val)
{
	*(U32*) loc = val;
	loc += sizeof(U32);
}

void MemWriter::write(S64 val)
{
	*(S64*) loc = val;
	loc += sizeof(S64);
}

void MemWriter::write(U64 val)
{
	*(U64*) loc = val;
	loc += sizeof(U64);
}

void MemWriter::write(F32 val)
{
	*(F32*) loc = val;
	loc += sizeof(F32);
}

void MemWriter::write(F64 val)
{
	*(F64*) loc = val;
	loc += sizeof(F64);
}

void MemWriter::write(char* s)
{
	if (s == nullptr) return;
	U32 sz = strlen(s);
	write(sz);
	for (U32 a = 0; a < sz; a++)
		*(loc + a) = s[a];
	loc += sz;
}