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

    int find_depth() const {
        return find_depth(root);
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

    return 0;
}
