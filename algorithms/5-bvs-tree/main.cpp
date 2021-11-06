#include <iostream>
#include <algorithm>
#include <limits>
#include <queue>

#include "utils.h"

using namespace std;

class BinarySearchTree {

private:
    typedef int key_t;
    static const key_t KEY_NEGATIVE_INF = numeric_limits<key_t>::min();
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
        } else {
            n->right = insert(n->right, k);
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

    key_t find_min(Node *n) const {
        if (n == nullptr) return KEY_POSITIVE_INF;
        return min({n->key, find_min(n->left), find_min(n->right)});
    }

    key_t find_max(Node *n) const {
        if (n == nullptr) return KEY_NEGATIVE_INF;
        return max({n->key, find_max(n->left), find_max(n->right)});
    }

public:
    BinarySearchTree() : root(nullptr) {}

    ~BinarySearchTree() {
        clear(root);
    }

    void insert(int key) {
        root = insert(root, key);
    }

    int find_depth() const {
        return find_depth(root);
    }

    key_t find_min() const {
        return find_min(root);
    }

    key_t find_max() const {
        return find_max(root);
    }

    friend ostream& operator<<(ostream &os, const BinarySearchTree &bst) {
        queue<Node*> q, next_q;
        if (bst.root != nullptr) q.push(bst.root);

        while (!q.empty()) {
            Node *n = q.front();
            q.pop();

            os << n->key;
            if (n->left || n->right) os << "(";
            if (n->left) {
                os << n->left->key;
                next_q.push(n->left);
            }
            if (n->left && n->right) os << ",";
            if (n->right) {
                os << n->right->key;
                next_q.push(n->right);
            }
            if (n->left || n->right) os << ")";
            os << " ";

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

    return 0;
}
