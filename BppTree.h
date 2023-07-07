#pragma once
//
// Created by giacomo on 07/06/23.
//

#ifndef KALANCO_BPLUSTREE_H
#define KALANCO_BPLUSTREE_H

extern "C" {
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
};

#define DEFAULT_ORDER 3

//typedef struct record {
//  int value;
//} record;

typedef struct node {
    int order;
    void** pointers; // ToDo: to be changed into a vector of vector of GameObjects, where each key is associated to a specific element in the vector, containing a vector of GameObjects with the same z-curve
    int* keys;
    struct node* parent;
    bool is_leaf;
    int num_keys;
    //    struct node *next;
} node;

//int order = DEFAULT_ORDER;
//node *queue = nullptr;
//bool verbose_output = false;

static inline node* make_node(int ord) {
    node* new_node;
    new_node = static_cast<node*>(malloc(sizeof(node)));
    memset(new_node, 0, sizeof(node));
    new_node->order = ord;
    if (new_node == nullptr) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }
    new_node->keys = static_cast<int*>(malloc((new_node->order - 1) * sizeof(int)));
    if (new_node->keys == nullptr) {
        perror("New node keys array.");
        exit(EXIT_FAILURE);
    }
    new_node->pointers = static_cast<void**>(malloc(new_node->order * sizeof(void*)));
    if (new_node->pointers == nullptr) {
        perror("New node pointers array.");
        exit(EXIT_FAILURE);
    }
    new_node->is_leaf = false;
    new_node->num_keys = 0;
    new_node->parent = nullptr;
    //    new_node->next = nullptr;
    return new_node;
}


static inline node* make_leaf(int ord) {
    node* leaf = make_node(ord);
    leaf->is_leaf = true;
    return leaf;
}


//void enqueue(node *new_node);
//node *dequeue(void);
//int height(node *const root);
//int path_to_root(node *const root, node *child);
//void print_leaves(node *const root);
//void print_tree(node *const root);
//void find_and_print(node *const root, int key, bool verbose);
//void find_and_print_range(node *const root, int range1, int range2, bool verbose);
//int find_range(node *const root, int key_start, int key_end, bool verbose,
//         int returned_keys[], void *returned_pointers[]);
//node *find_leaf(node *const root, int key, bool verbose);
//void *find(node *root, int key, bool verbose, node **leaf_out);
//int cut(int length);

//template <typename T> T *make_record(int value);
//node *make_node(void);
//node *make_leaf(void);
//int get_left_index(node *parent, node *left);
//node *insert_into_leaf(node *leaf, int key, void *pointer);
//node *insert_into_leaf_after_splitting(node *root, node *leaf, int key,
//                     void *pointer);
//node *insert_into_node(node *root, node *parent,
//             int left_index, int key, node *right);
node* insert_into_node_after_splitting(node* root, node* parent,
    int left_index,
    int key, node* right, int ord);
node* insert_into_parent(node* root, node* left, int key, node* right, int ord);
//node *insert_into_new_root(node *left, int key, node *right);
//node *start_new_tree(int key, void *pointer);
//node *insert(node *root, int key, int value);

//int get_neighbor_index(node *n);
//node *adjust_root(node *root);
node* coalesce_nodes(node* root, node* n, node* neighbor,
    int neighbor_index, int k_prime);
//node *redistribute_nodes(node *root, node *n, node *neighbor,
//             int neighbor_index,
//             int k_prime_index, int k_prime);
node* delete_entry(node* root, node* n, int key, void* pointer);
//node *delete_ (node *root, int key);

static inline
node* insert_into_node(node* root, node* n,
    int left_index, int key, node* right) {
    int i;

    for (i = n->num_keys; i > left_index; i--) {
        n->pointers[i + 1] = n->pointers[i];
        n->keys[i] = n->keys[i - 1];
    }
    n->pointers[left_index + 1] = right;
    n->keys[left_index] = key;
    n->num_keys++;
    return root;
}

static inline
node* insert_into_new_root(node* left, int key, node* right, int order) {
    node* root = make_node(order);
    root->keys[0] = key;
    root->pointers[0] = left;
    root->pointers[1] = right;
    root->num_keys++;
    root->parent = nullptr;
    left->parent = root;
    right->parent = root;
    return root;
}

static inline
node* start_new_tree(int key, void* pointer, int ord) {
    node* root = make_leaf(ord);
    root->keys[0] = key;
    root->pointers[0] = pointer;
    root->pointers[root->order - 1] = nullptr;
    root->parent = nullptr;
    root->num_keys++;
    return root;
}

static inline
int height(node* const root) {
    int h = 0;
    node* c = root;
    while (!c->is_leaf) {
        c = static_cast<node*>(c->pointers[0]);
        h++;
    }
    return h;
}



static inline node* find_leaf(node* const root, int key, bool verbose) {
    if (root == nullptr) {
        if (verbose)
            printf("Empty tree.\n");
        return root;
    }
    int i = 0;
    node* c = root;
    while (!c->is_leaf) {
        if (verbose) {
            printf("[");
            for (i = 0; i < c->num_keys - 1; i++)
                printf("%d ", c->keys[i]);
            printf("%d] ", c->keys[i]);
        }
        i = 0;
        while (i < c->num_keys) {
            if (key >= c->keys[i])
                i++;
            else
                break;
        }
        if (verbose)
            printf("%d ->\n", i);
        c = (node*)c->pointers[i];
    }
    if (verbose) {
        printf("Leaf [");
        for (i = 0; i < c->num_keys - 1; i++)
            printf("%d ", c->keys[i]);
        printf("%d] ->\n", c->keys[i]);
    }
    return c;
}

static inline
void* find(node* root, int key, bool verbose, node** leaf_out) {
    if (root == nullptr) {
        if (leaf_out != nullptr) {
            *leaf_out = nullptr;
        }
        return nullptr;
    }

    int i = 0;
    node* leaf = nullptr;

    leaf = find_leaf(root, key, verbose);

    for (i = 0; i < leaf->num_keys; i++)
        if (leaf->keys[i] == key)
            break;
    if (leaf_out != nullptr) {
        *leaf_out = leaf;
    }
    if (i == leaf->num_keys)
        return nullptr;
    else
        return leaf->pointers[i];
}

#include <queue>

//static inline void enqueue(std::queue<node*>& queue, node *new_node) {
//  node *c;
//  if (queue.empty()) {
//    queue.emplace(new_node);
////    queue->next = nullptr;
//  } else {
////    c = queue;
////    while (c->next != nullptr) {
////      c = c->next;
////    }
////    c->next = new_node;
////    new_node->next = nullptr;
//  }
//}

//static inline node *dequeue() {
//  node *n = queue;
//  queue = queue->next;
//  n->next = nullptr;
//  return n;
//}

static inline void print_leaves(node* const root, bool verbose_output = false) {
    if (root == nullptr) {
        printf("Empty tree.\n");
        return;
    }
    int i;
    node* c = root;
    while (!c->is_leaf)
        c = static_cast<node*>(c->pointers[0]);
    while (true) {
        for (i = 0; i < c->num_keys; i++) {
            if (verbose_output)
                printf("%p ", c->pointers[i]);
            printf("%d ", c->keys[i]);
        }
        if (verbose_output)
            printf("%p ", c->pointers[c->order - 1]);
        if (c->pointers[c->order - 1] != nullptr) {
            printf(" | ");
            c = static_cast<node*>(c->pointers[c->order - 1]);
        }
        else
            break;
    }
    printf("\n");
}


static inline int path_to_root(node* const root, node* child) {
    int length = 0;
    node* c = child;
    while (c != root) {
        c = c->parent;
        length++;
    }
    return length;
}

static inline void print_tree(node* const root, bool verbose_output = false) {
    node* n = nullptr;
    int i = 0;
    int rank = 0;
    int new_rank = 0;

    if (root == nullptr) {
        printf("Empty tree.\n");
        return;
    }
    std::queue<node*> queue;
    queue.emplace(root);
    //  queue = nullptr;
    //  enqueue(root);
    while (!queue.empty()) {
        auto n = queue.front();
        queue.pop();
        if (n->parent != nullptr && n == n->parent->pointers[0]) {
            new_rank = path_to_root(root, n);
            if (new_rank != rank) {
                rank = new_rank;
                printf("\n");
            }
        }
        if (verbose_output)
            printf("(%p)", n);
        for (i = 0; i < n->num_keys; i++) {
            if (verbose_output)
                printf("%p ", n->pointers[i]);
            printf("%d ", n->keys[i]);
        }
        if (!n->is_leaf)
            for (i = 0; i <= n->num_keys; i++)
                queue.emplace((node*)n->pointers[i]);
        if (verbose_output) {
            if (n->is_leaf)
                printf("%p ", n->pointers[n->order - 1]);
            else
                printf("%p ", n->pointers[n->num_keys]);
        }
        printf("| ");
    }
    printf("\n");
}

static inline void find_and_print(node* const root, int key, bool verbose) {
    node* leaf = nullptr;
    void* r = find(root, key, verbose, nullptr);
    if (r == nullptr)
        printf("Record not found under key %d.\n", key);
    else
        printf("Record at %p -- key %d, ptr %x.\n",
            r, key, r);
}

static inline int find_range(node* const root, int key_start, int key_end, bool verbose,
    int returned_keys[], void* returned_pointers[]) {
    int i, num_found;
    num_found = 0;
    node* n = find_leaf(root, key_start, verbose);// Finding the element from the tree
    if (n == nullptr)
        return 0; // Nothing was found
    for (i = 0; i < n->num_keys && n->keys[i] < key_start; i++) // getting the key within the node if exists
        ;
    if (i == n->num_keys)
        return 0; // Nothing was found: beyon the set of elements
    while (n != nullptr) {
        for (; i < n->num_keys && n->keys[i] <= key_end; i++) { // Iterating over the existing elements
            returned_keys[num_found] = n->keys[i];
            returned_pointers[num_found] = n->pointers[i];
            num_found++;
        }
        n = static_cast<node*>(n->pointers[n->order - 1]); // Moving towards the next leaf
        i = 0; // re-setting the counter to zero
    }
    return num_found;
}

static inline
void find_and_print_range(node* const root, int key_start, int key_end,
    bool verbose) {
    int i;
    int array_size = key_end - key_start + 1;
    int* returned_keys = (int*)malloc(sizeof(int)*(array_size));
    void** returned_pointers = (void**)malloc(sizeof(void*) * (array_size));
    int num_found = find_range(root, key_start, key_end, verbose,
        returned_keys, returned_pointers);
    if (!num_found)
        printf("None found.\n");
    else {
        for (i = 0; i < num_found; i++)
            printf("Key: %d   Location: %p  Value: %x\n",
                returned_keys[i],
                returned_pointers[i],
                (
                    returned_pointers[i]));
    }
    free(returned_keys);
    free(returned_pointers);
}





#define cut(length) (((length) % 2 == 0) ? ((length)/2) : ((length)/2+1))

//int cut(int length) {
//  if (length % 2 == 0)
//    return length / 2;
//  else
//    return length / 2 + 1;
//}

//template <typename T>
//T *make_record(int value) {
//  T *new_record = new T();
//  if (new_record == nullptr) {
//    perror("Record creation.");
//    exit(EXIT_FAILURE);
//  } else {
//    new_record->value = value;
//  }
//  return new_record;
//}



static inline int get_left_index(node* parent, node* left) {
    int left_index = 0;
    while (left_index <= parent->num_keys &&
        parent->pointers[left_index] != left)
        left_index++;
    return left_index;
}


static inline node* insert_into_leaf(node* leaf, int key, void* pointer) {
    int i, insertion_point;

    insertion_point = 0;
    while (insertion_point < leaf->num_keys && leaf->keys[insertion_point] < key)
        insertion_point++;

    for (i = leaf->num_keys; i > insertion_point; i--) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
    }
    leaf->keys[insertion_point] = key;
    leaf->pointers[insertion_point] = pointer;
    leaf->num_keys++;
    return leaf;
}

static inline node* insert_into_leaf_after_splitting(node* root, node* leaf, int key, void* pointer, int ord) {
    node* new_leaf;
    int* temp_keys;
    void** temp_pointers;
    int insertion_index, split, new_key, i, j;

    new_leaf = make_leaf(ord);

    temp_keys = static_cast<int*>(malloc(new_leaf->order * sizeof(int)));
    if (temp_keys == nullptr) {
        perror("Temporary keys array.");
        exit(EXIT_FAILURE);
    }

    temp_pointers = static_cast<void**>(malloc(new_leaf->order * sizeof(void*)));
    if (temp_pointers == nullptr) {
        perror("Temporary pointers array.");
        exit(EXIT_FAILURE);
    }

    insertion_index = 0;
    while (insertion_index < new_leaf->order - 1 && leaf->keys[insertion_index] < key)
        insertion_index++;

    for (i = 0, j = 0; i < leaf->num_keys; i++, j++) {
        if (j == insertion_index)
            j++;
        temp_keys[j] = leaf->keys[i];
        temp_pointers[j] = leaf->pointers[i];
    }

    temp_keys[insertion_index] = key;
    temp_pointers[insertion_index] = pointer;

    leaf->num_keys = 0;

    split = cut(leaf->order - 1);

    for (i = 0; i < split; i++) {
        leaf->pointers[i] = temp_pointers[i];
        leaf->keys[i] = temp_keys[i];
        leaf->num_keys++;
    }

    for (i = split, j = 0; i < leaf->order; i++, j++) {
        new_leaf->pointers[j] = temp_pointers[i];
        new_leaf->keys[j] = temp_keys[i];
        new_leaf->num_keys++;
    }

    free(temp_pointers);
    free(temp_keys);

    new_leaf->pointers[new_leaf->order - 1] = leaf->pointers[leaf->order - 1];
    leaf->pointers[leaf->order - 1] = new_leaf;

    for (i = leaf->num_keys; i < leaf->order - 1; i++)
        leaf->pointers[i] = nullptr;
    for (i = new_leaf->num_keys; i < new_leaf->order - 1; i++)
        new_leaf->pointers[i] = nullptr;

    new_leaf->parent = leaf->parent;
    new_key = new_leaf->keys[0];

    return insert_into_parent(root, leaf, new_key, new_leaf, ord);
}



//static inline
node* insert_into_node_after_splitting(node* root, node* old_node, int left_index,
    int key, node* right, int ord);



static inline
node* insert(node* root, int key, void* record_pointer = nullptr, int ord = DEFAULT_ORDER) {
    node* leaf;

    auto tmp = find(root, key, false, nullptr);
    if (tmp != nullptr) {
        return root;
    }

    //  record_pointer = make_record(value);

    if (root == nullptr)
        return start_new_tree(key, record_pointer, ord);

    leaf = find_leaf(root, key, false);

    if (leaf->num_keys < leaf->order - 1) {
        leaf = insert_into_leaf(leaf, key, record_pointer);
        return root;
    }

    return insert_into_leaf_after_splitting(root, leaf, key, record_pointer, ord);
}

static inline
int get_neighbor_index(node* n) {
    int i;
    for (i = 0; i <= n->parent->num_keys; i++)
        if (n->parent->pointers[i] == n)
            return i - 1;

    printf("Search for nonexistent pointer to node in parent.\n");
    printf("Node:  %#lx\n", (unsigned long)n);
    exit(EXIT_FAILURE);
}

static inline
node* remove_entry_from_node(node* n, int key, node* pointer) {
    int i, num_pointers;
    i = 0;
    while (n->keys[i] != key)
        i++;
    for (++i; i < n->num_keys; i++)
        n->keys[i - 1] = n->keys[i];

    num_pointers = n->is_leaf ? n->num_keys : n->num_keys + 1;
    i = 0;
    while (n->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        n->pointers[i - 1] = n->pointers[i];

    n->num_keys--;

    if (n->is_leaf)
        for (i = n->num_keys; i < n->order - 1; i++)
            n->pointers[i] = nullptr;
    else
        for (i = n->num_keys + 1; i < n->order; i++)
            n->pointers[i] = nullptr;

    return n;
}

static inline
node* adjust_root(node* root) {
    node* new_root;

    if (root->num_keys > 0)
        return root;

    if (!root->is_leaf) {
        new_root = static_cast<node*>(root->pointers[0]);
        new_root->parent = nullptr;
    }

    else
        new_root = nullptr;

    free(root->keys);
    free(root->pointers);
    free(root);

    return new_root;
}



static inline
node* redistribute_nodes(node* root, node* n, node* neighbor, int neighbor_index,
    int k_prime_index, int k_prime) {
    int i;
    node* tmp;

    if (neighbor_index != -1) {
        if (!n->is_leaf)
            n->pointers[n->num_keys + 1] = n->pointers[n->num_keys];
        for (i = n->num_keys; i > 0; i--) {
            n->keys[i] = n->keys[i - 1];
            n->pointers[i] = n->pointers[i - 1];
        }
        if (!n->is_leaf) {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys];
            tmp = (node*)n->pointers[0];
            tmp->parent = n;
            neighbor->pointers[neighbor->num_keys] = nullptr;
            n->keys[0] = k_prime;
            n->parent->keys[k_prime_index] = neighbor->keys[neighbor->num_keys - 1];
        }
        else {
            n->pointers[0] = neighbor->pointers[neighbor->num_keys - 1];
            neighbor->pointers[neighbor->num_keys - 1] = nullptr;
            n->keys[0] = neighbor->keys[neighbor->num_keys - 1];
            n->parent->keys[k_prime_index] = n->keys[0];
        }
    }

    else {
        if (n->is_leaf) {
            n->keys[n->num_keys] = neighbor->keys[0];
            n->pointers[n->num_keys] = neighbor->pointers[0];
            n->parent->keys[k_prime_index] = neighbor->keys[1];
        }
        else {
            n->keys[n->num_keys] = k_prime;
            n->pointers[n->num_keys + 1] = neighbor->pointers[0];
            tmp = (node*)n->pointers[n->num_keys + 1];
            tmp->parent = n;
            n->parent->keys[k_prime_index] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->num_keys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!n->is_leaf)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
    }

    n->num_keys++;
    neighbor->num_keys--;

    return root;
}



static inline
node* delete_(node* root, int key) {
    node* key_leaf = nullptr;
    void* key_record = nullptr;
    key_record = find(root, key, false, &key_leaf);
    if (key_record != nullptr && key_leaf != nullptr) {
        root = delete_entry(root, key_leaf, key, key_record);
        if (!key_leaf->is_leaf) free(key_record);
    }
    return root;
}

static inline
void destroy_tree_nodes(node* root) {
    int i;
    if (root->is_leaf)
        for (i = 0; i < root->num_keys; i++)
            free(root->pointers[i]);
    else
        for (i = 0; i < root->num_keys + 1; i++)
            destroy_tree_nodes((node*)root->pointers[i]);
    free(root->pointers);
    free(root->keys);
    free(root);
}

static inline
node* destroy_tree(node* root) {
    destroy_tree_nodes(root);
    return nullptr;
}

#endif //KALANCO_BPLUSTREE_H
