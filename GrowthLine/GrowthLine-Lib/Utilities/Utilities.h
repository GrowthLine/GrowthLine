#ifndef UTILITIES_INCLUDE_
#define UTILITIES_INCLUDE_

// converts celsius to fahrenheit
float cToF(float c);

// Template for a doubly linked list
template <typename T>
class List {
    protected:
    
    // structure to a single node in the list
    struct Node {
        T value;
        Node* next;
        Node* previous;
    };

    /**
     * maximum number of readings to hold in list, adding new readings when
     * list is full, removes the oldest reading added
     * A value of -1 indicates that the list should have no limit (theoretically).
     */
    const int maxSize;  
    unsigned int size;
    Node* head;
    Node* tail;
    Node* cursor;

    public:
    // constructor for a list
    List() : List(-1) {

    }

    // constructor for  list provided maximum reading capcity
    List(const unsigned int& mS) : maxSize(mS), size(0) {
        head = new Node;
        tail = new Node;
        head->previous = nullptr;
        tail->next = nullptr;
        head->next = tail;
        tail->previous = head;
        cursor = nullptr;
    }

    // copy constructor
    List(const List &that) : maxSize(that.maxSize), size(0) {
        head = new Node;
        tail = new Node;
        head->previous = nullptr;
        tail->next = nullptr;
        cursor = head;
        head->next = tail;
        tail->previous = head;
        Node* thatCursor = that.head->next;
        while (thatCursor != that.tail) {
            add(thatCursor->value);
            thatCursor = thatCursor->next;
        }
        begin();
    };

    // Assignment operator
    List& operator= (const List that) {
        if (this != &that) {
            clear();
            Node* thatCursor = that.head->next;
            while (thatCursor != that.tail) {
                add(thatCursor->value);
                thatCursor = thatCursor->next;
                size++;
            }
            begin();
        }
        return *this;
    }

    // destructor for the list
    ~List() {
        begin();
        while (isNotAtEnd()) {
            Node* currentNode = cursor;
            advance();
            delete currentNode;
        }
        delete head;
        delete tail;
    }

    // gets the size of the list
    unsigned int count() const {
        return size;
    }

    // gets the maximum possible size of the list
    unsigned int maxCount() const {
        return maxSize;
    }

    /* adds a node to the end of the list and removes the oldest node
     * if the number of nodes in the list exceeds the maxSize
     */
    void add(const T& t) {
        Node* newNode = new Node;
        newNode->value = t;
        newNode->previous = tail->previous;
        newNode->next = tail;
        tail->previous->next = newNode;
        tail->previous = newNode;
        size += 1;
        if( maxSize != -1 && size > maxSize) {
            Node* nodeToDelete = head->next;
            head->next = head->next->next;
            head->next->next->previous = head;
            delete nodeToDelete;
            size -= 1;
        }
        begin();
    }

    // get newest element in the list
    T get_new() const {
        if( tail->previous != head) {
            return tail->previous->value; 
        } 
    }

    T get_old() const {
        if (head->next != tail) {
            return head->next->value;
        }
    }

    // gets the element in the ith location. 
    T get(const int& index) {
        if (index < size) {
            begin();
            for (int i = 0; i < index; i++) {
                advance();
            }
            return cursor->value;
        } 
    }

    // removes the oldest node in the list
    void pop_old() {
        if (head->next != tail) {
            Node* nodeToDelete = head->next;
            head->next = nodeToDelete->next;
            nodeToDelete->next->previous = head;
            begin();
            delete nodeToDelete;
            size -= 1;
        }
    }

    // removes the newest node in the list
    void pop_new() {
        if (tail->previous != head) {
            Node* nodeToDelete = tail->previous;
            tail->previous = nodeToDelete->previous;
            nodeToDelete->previous->next = tail;
            begin();
            delete nodeToDelete;
            size -= 1;
        }
    }

    // removes the node at the ith location, if it exists
    void pop(const int& index) {
        if (index < size) {
            begin();
            for (int i = 0; i < index; i++) {
                cursor = cursor->next;
            }
            cursor->previous->next = cursor->next;
            cursor->next->previous = cursor->previous;
            delete cursor;
            begin();
            size -= 1;
        }
    }

    // remove all nodes
    void clear() {
        begin();
        if (size == 0) {
            return;
        }
        while (isNotAtEnd()) {
            Node* nodeToDelete = cursor;
            advance();
            delete nodeToDelete;
        }
        begin();
        size = 0;
    }

    // whether the list is empty
    bool isEmpty() const {
        if (size == 0) {
            return true;
        }
        return false;
    }

    // advances the cursor to the next element until it reaches null
    void advance() {
        if (cursor != nullptr) {
            if (cursor->next != tail) {
                cursor = cursor->next;
            } else {
                cursor = nullptr;
            }
        }
    }

    // returns the reading being pointed by the cursor
    T getAtCursor() const {
        if (cursor != nullptr) {
            return cursor->value;
        } 
    }

    // moves the cursor to the last element
    void end() {
        if (tail->previous != head) {
            cursor = tail->previous;
        } else {
            cursor = nullptr;
        }
    }

    // begins the cursor to the first element
    void begin() {
        if (head->next != tail) {
            cursor = head->next;
        } else {
            cursor = nullptr;
        }
    }

    // identifies whether the cursor is at the beginning of the list
    bool isAtBeginning() const {
        if (cursor != nullptr && cursor->previous == head) {
            return true;
        }
        return false;
    }

    // identifies whether the cursor is not at the beginning of the list
    bool isNotAtBeginning() const {
        return !isAtBeginning();
    }

    // identifies whether the cursor is at the end of the list or not
    bool isAtEnd() const {
        if (cursor == nullptr) {
            return true;
        }
        return false;
    }

    // identifies whether the cursor is not at the end of the list
    bool isNotAtEnd() {
        return !isAtEnd();
    }

    // identifies whether the cursor is at the specified index 
    bool isAtLocation(unsigned int index) {
        if (index < size) {
            Node* holder = cursor;
            begin();
            for (int i = 0; i < index; i++) {
                advance();
            }
            if (cursor == holder) {
                cursor = holder;
                return true;
            } else {
                cursor = holder;
                return false;
            }
        }
        return false;
    }
};
#endif /* UTILITIES_INCLUDE_ */
