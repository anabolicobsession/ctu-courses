 #include <iostream>
#include <algorithm>
#include <queue>
#include <limits>

#include "utils.h"

using namespace std;

class BinarySearchTree {

private:
    typedef int key_t;
    static const key_t KEY_POSITIVE_INF = numeric_limits<key_t>::max();

    struct Node {
        key_t key;
        Node *left;
        Node *right;
    };

    Node *root;

    Node* insert(Node *n, key_t k) {
        if (n == nullptr) {
            n = new Node;
            n->key = k;
            n->left = n->right = nullptr;
        } else if (k < n->key) {
            n->left = insert(n->left, k);
        } else if (k > n->key) {
            n->right = insert(n->right, k);
        }

        return n;
    }

    Node* insert_right(Node *n, key_t start, key_t end) {
        if (start > end) return n;
        n = insert(n, start);
        n->right = insert_right(n->right, start + 1, end);
        return n;
    }

    Node* insert(Node *n, key_t start, key_t end) {
        if (n == nullptr) {
            n = insert_right(n, start, end);
        } else if (end < n->key) {
            n->left = insert(n->left, start, end);
        } else if (start > n->key) {
            n->right = insert(n->right, start, end);
        } else {
            if (start < n->key) n->left = insert(n->left, start, n->key - 1);
            if (n->key < end) n->right = insert(n->right, n->key + 1, end);
        }

        return n;
    }

    Node* remove(Node *n, key_t k) {
        Node *tmp;
        if (n == nullptr) {
            return nullptr;
        } else if (k < n->key) {
            n->left = remove(n->left, k);
        } else if (k > n->key) {
            n->right = remove(n->right, k);
        } else if (n->left != nullptr && n->right != nullptr) {
            tmp = find_min(n->right);
            n->key = tmp->key;
            n->right = remove(n->right, n->key);
        } else {
            tmp = n;
            n = n->left != nullptr ? n->left : n->right;
            delete tmp;
        }

        return n;
    }

    void clear(Node *n) {
        if (n != nullptr) {
            clear(n->left);
            clear(n->right);
            delete n;
        }
    }

    int find_depth(Node *n) const {
        if (n == nullptr) return -1;
        return 1 + max(find_depth(n->left), find_depth(n->right));
    }

    int find_number_of_nodes(Node *n) const {
        if (n == nullptr) return 0;
        return 1 + find_number_of_nodes(n->left) + find_number_of_nodes(n->right);
    }

    Node* find_min(Node *n) const {
        if (n == nullptr) {
            return nullptr;
        } else if (n->left == nullptr) {
            return n;
        }

        return find_min(n->left);
    }

    Node* find_max(Node *n) const {
        if (n == nullptr) {
            return nullptr;
        } else if (n->right == nullptr) {
            return n;
        }

        return find_min(n->right);
    }

public:
    BinarySearchTree() : root(nullptr) {}

    ~BinarySearchTree() {
        clear(root);
    }

    void insert(key_t k) {
        root = insert(root, k);
    }

    void insert(key_t start, key_t end) {
        root = insert(root, start, end);
    }

    void remove(key_t k) {
        root = remove(root, k);
    }

    int find_depth() const {
        return find_depth(root);
    }

    int find_number_of_nodes() const {
        return find_number_of_nodes(root);
    }

    friend ostream& operator<<(ostream &os, const BinarySearchTree &bst) {
        queue<Node*> q, next_q;
        if (bst.root != nullptr) q.push(bst.root);

        while (!q.empty()) {
            Node *n = q.front();
            q.pop();

            os << n->key << ' ';
            if (n->left) next_q.push(n->left);
            if (n->right) next_q.push(n->right);

            if (q.empty()) {
                q = next_q;
                next_q = queue<Node*>();
                os << '\n';
            }
        }

        return os;
    }

};

int main() {
    BinarySearchTree bst;
    int n_ops;
    cin >> n_ops;

    for (int i = 0; i < n_ops; ++i) {
        char op = utils::get_letter();
        int start = utils::get_integer();
        int end = utils::get_integer();

        if (op == 'i') bst.insert(start, end);
        else for (int k = start; k <= end; ++k) bst.remove(k);
    }

    cout << bst.find_number_of_nodes() << ' ' << bst.find_depth() << '\n';

    return 0;
}
