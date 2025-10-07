#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>


struct Error
{
	static void error(const char *what);
	static void assert(bool flag, const char *what);
};

void Error::error(const char *what)
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

class String
{
	char *chars = nullptr;
	uint capacity = 0;
	uint count = 0;	
public:
	String(int size);
	String(int size, char c);
	String(const char *str = "");
	String(const String &arg);
	
	virtual ~String();

	void operator=(const String &rhs);
	void operator+=(const String &rhs);
	String operator+(const String &rhs);
	char& operator[](uint index);
	char operator[](uint index) const;
	
	void clear();
	void resize(uint sz);
	int findFirst(char c) const;
	int findLast(char c) const;
	String substr(uint start, uint end) const;

	uint size() const {return count;}
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

String::String(int sz)
{
	this->count = 0;
	this->capacity = sz;
	this->chars = new char[capacity];
	this->chars[0] = 0;
}

String::String(int size, char c)
{
	this->count = size;
	this->capacity = size + 1;
	this->chars = new char[capacity];
	this->chars[count] = 0;
	memset(this->chars, c, count);
}

String::String(const char* str)
{
	this->count = strlen(str);
	this->capacity = count + 1;
	this->chars = new char[capacity];
	strcpy(this->chars, str);
}

String::String(const String &arg)
{
	this->count = arg.size();
	this->capacity = count + 1;
	this->chars = new char[capacity];
	strcpy(this->chars, arg.chars);
}

String::~String()
{
	if (this->chars != nullptr)
		delete[] this->chars;
}

void String::operator=(const String &rhs)
{
	if (this != &rhs) {
		delete[] chars;
		this->count = rhs.count;
		this->capacity = rhs.capacity;
		this->chars = new char[capacity];
		strcpy(this->chars, rhs.chars);
	}
}

void String::operator+=(const String &rhs)
{
	uint total = this->count + rhs.count;
	if (total >= this->capacity) {
		this->capacity = total * 2;
		char *ptr = chars;
		this->chars  = new char[this->capacity];
		strcpy(this->chars, ptr);
		delete[] ptr;
	}
	strcpy(this->chars + count, rhs.chars);
	this->count = total;
}

String String::operator+(const String &rhs)
{
	String result(*this);
	result += rhs;
	return result;
}

char& String::operator[](uint index)
{
	if (index >= this->count) Error::error("String index out of bound");
	return this->chars[index];
}

char String::operator[](uint index) const
{
	if (index >= this->count) Error::error("String index out of bound");
	return this->chars[index];
}

void String::clear()
{
	this->count = 0;
	this->capacity = 0;
	this->chars[0] = 0;
}

void String::resize(uint sz)
{
	this->capacity = sz;
	if (sz < this->count) this->count = sz;
	char *ptr = this->chars;
	this->chars = new char[sz];
	memset(this->chars, 0, sz);
	memcpy(this->chars, ptr, this->count);
}

int String::findFirst(char c) const
{
	if (c == 0) return String::NO_POS;
	char *ptr = strchr(this->chars, c);
	return ptr != nullptr ? ptr - this->chars : String::NO_POS;
}

int String::findLast(char c) const
{
	if (c == 0) return String::NO_POS;
	char *ptr = strrchr(this->chars, c);
	return ptr != nullptr ? ptr - this->chars: String::NO_POS;
}

String String::substr(uint start, uint end) const
{
	if (start >= this->count or end >= this->count) Error::error("String index out of bound");
	uint len = end - start;
	String str(len, 0);
	memcpy(str.chars, this->chars + start, len);
	return str;
}

String String::format(const char *fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, 256, fmt, args);
	va_end(args);
	return buf;
}

template<class T>
class List
{
	struct Node {T data; Node *next;};
	Node *tail = nullptr;
	Node *head = nullptr;
	uint count = 0;
public:
	List();
	List(const List &arg);
	virtual ~List();

	void operator=(const List &rhs);
	void enqueue(const T &elem);
	T dequeue();
	const T& peek() const;
	
	void clear();
	
	uint size() const;
	bool empty() const;
};

template<class T>
List<T>::List() {}

template<class T>
List<T>::~List()
{
	clear();
}

template<class T>
List<T>::List(const List<T> &arg)
{
	for (auto iter = arg.head; iter != nullptr; iter = iter->next)
		enqueue(iter->data);
}

template<class T>
void List<T>::operator=(const List<T> &rhs)
{
	if (this == &rhs) return;
	else clear();
	for (auto iter = rhs.head; iter != nullptr; iter = iter->next)
		enqueue(iter->data);
}

template<class T>
void List<T>::enqueue(const T &elem)
{
	Node *node = new Node;
	node->data = elem;
	if (empty()) {
		this->head = node;
		this->tail = node;
	} else {
		this->tail->next = node;
		this->tail = node;
	}
	this->count += 1;
}

template<class T>
T List<T>::dequeue()
{
	if (empty()) Error::error("List underflow");
	T retval = head->data;
	Node *node = head;
	head = head->next;
	delete node;
	count--;
	return retval;
}

template<class T>
const T& List<T>::peek() const
{
	if (empty()) Error::error("List underflow");
	return head->data;
}

template<class T>
void List<T>::clear()
{
	Node *curr = head;
	while (curr != nullptr) {
		Node *next = curr->next;
		delete curr;
		curr = next;
	}
	this->head = nullptr;
	this->tail = nullptr;
	this->count = 0;
}

template<class T>
uint List<T>::size() const
{
	return count;
}

template<class T>
bool List<T>::empty() const
{
	return size() == 0;
}


class Path
{
	String path;
public:
	Path() {}
	Path(const String &path);
	String getPath() const {return path;}
};

Path::Path(const String &path)
{
	if (!path.empty() and path[0] == '/')
		this->path = path;
	else {
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) == nullptr)
			Error::error("getcwd");
		this->path = String(cwd) + "/" + path;
	}
}


class File
{
	Path path;
public:
	File(const String &path);
	File(const Path &path);
	bool exists() const;
	bool isFile() const;
	bool isDirectory() const;
};

File::File(const String &path) {this->path = Path(path);}
File::File(const Path &path) {this->path = path;}
bool File::isFile() const {
	struct stat st;
	if (stat(path.getPath().cstr(), &st) != 0) return false;
		return S_ISREG(st.st_mode);
}
bool File::isDirectory() const {
	struct stat st;
	if (stat(path.getPath().cstr(), &st) != 0) return false;
		return S_ISDIR(st.st_mode);
}
bool File::exists() const {
	struct stat st;
	return (stat(path.getPath().cstr(), &st) == 0);
}


int main(int argc, const char **argv)
{
	File file("/home/dkumar/collection/Coll.java0");
	if (file.isFile()) printf("file\n");
	else printf("directory\n");
	if (file.exists())
		printf("exists\n");
	else printf("not exists\n");
	return EXIT_SUCCESS;
}