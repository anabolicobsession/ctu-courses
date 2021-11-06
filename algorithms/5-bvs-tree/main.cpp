#include <iostream>
#include <algorithm>
#include <limits>

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

};

int main() {

    return 0;
}
