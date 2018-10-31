#include <iostream>
using namespace std;

template <class T>
class Node {
	private:
		T data;
		Node* next;

	public:
		Node(char characters = '\0', Node* n = nullptr) {
			data = characters;
			next = n;
		}

		T getData() {
			return data;
		}

		Node* getNext() {
			return next;
		}

		void setData(T c) {
			data = c;
		}

		void setNext(Node* n) {
			if (next != '\0') {
				delete next;
			}
			next = n;
		}

};

template <class T>
class Stack {
	private:
		Node<T>* first;
		int size;

	public:
		Stack<T>() {
			first = nullptr;
			size = 0;
		}

		void push(T item) {
			if (size != 0) {
				first = new Node<T>(item, first);
			}
			else {
				first = new Node<T>(item);
			}
			size++;
		}

		T pop() {
			T c = (*first).getData();
			Node<T>* cur = first;
			first = first->getNext();
			delete cur;
			size--;
			return c;
		}

		T peek() {
			return (*first).getData();
		}

		void clear() {
			Node<T>* prev = nullptr;
			Node<T>* cur = first;
			while (cur != nullptr) {
				prev = cur;
				cur = cur->getNext();
				delete prev;
			}
			size = 0;
		}

		int length() {
			return size;
		}

		bool isEmpty() {
			return size == 0;
		}

		void print() {
			if (!isEmpty()) {
				Node<T>* cur;
				for (cur = first; cur->getNext() != '\0'; cur = cur->getNext()) {
					cout << cur->getData() << endl;
				}
				cout << cur->getData() << endl;
			}
		}
};
