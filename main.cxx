#include <stdexcept>

template <typename T>
class List {
public:
    virtual void add(const T& element) = 0;
    virtual void remove(int index) = 0;
    virtual T get(int index) const = 0;
    virtual int size() const = 0;
    virtual ~List() {}
};

template <typename T>
class ArrayList : public List<T> {
private:
    T* data;
    int capacity;
    int length;

    void resize() {
        capacity *= 2;
        T* newData = new T[capacity];
        for (int i = 0; i < length; i++)
            newData[i] = data[i];
        delete[] data;
        data = newData;
    }

public:
    ArrayList(int initialCapacity = 10) {
        capacity = initialCapacity;
        length = 0;
        data = new T[capacity];
    }

    void add(const T& element) override {
        if (length == capacity) {
            resize();
        }
        data[length++] = element;
    }

    void remove(int index) override {
        if (index < 0 || index >= length) throw out_of_range("Index out of range");
        for (int i = index; i < length - 1; i++) {
            data[i] = data[i + 1];
        }
        length--;
    }

    T get(int index) const override {
        if (index < 0 || index >= length) throw out_of_range("Index out of range");
        return data[index];
    }

    int size() const override {
        return length;
    }

    ~ArrayList() {
        delete[] data;
    }
};

template <typename T>
class LinkedList : public List<T> {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int length;

public:
    LinkedList() : head(nullptr), tail(nullptr), length(0) {}

    void add(const T& element) override {
        Node* newNode = new Node(element);
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        length++;
    }

    void remove(int index) override {
        if (index < 0 || index >= length) throw out_of_range("Index out of range");

        Node* current = head;
        for (int i = 0; i < index; i++) current = current->next;

        if (current->prev) current->prev->next = current->next;
        if (current->next) current->next->prev = current->prev;
        if (current == head) head = current->next;
        if (current == tail) tail = current->prev;

        delete current;
        length--;
    }

    T get(int index) const override {
        if (index < 0 || index >= length) throw out_of_range("Index out of range");
        Node* current = head;
        for (int i = 0; i < index; i++) current = current->next;
        return current->data;
    }

    int size() const override {
        return length;
    }

    ~LinkedList() {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }
};


int main() {
	ArrayList<int> array;
	array.add(100);
	List<int> &list= array;
	std::cout << list.get(1) << std::endl;
	std::cout << "hello" << std::endl;
}