// HW#0
#define LINEARSEARCH 1    // Linear Search
#define HASH 2            // Hash Table (Direct Access)
#define BST 3             // Binary Search Tree
// #define AVL 4          // AVL (Balanced Binary Search Tree), not implemented

#define YES 1
#define NO 0

#define EFFCHECK NO     // YES/NO, efficiency check: big-O (max depth), min/avg depth, etc.

#define SEARCH HASH
/*******************************************************************************
   Algorithm                           Time complexity comparison
1: linear search                         O(n)
2: hash table (direct access)            O(1)
3. BST (binary search tree)              O(n)
4: AVL (balanced binary search tree)     O(log n), not implemented

Description:
1. Linear search is O(n) time complexity
   Unlimited set of characters.
   The last added character in the linked-list is with O(n) time complexity.
   No additional memory overhead, simply a linked-list.
2. Direct access is O(1) time complexity
   Essentially a hash table with the key being the ASCII value of the character.
   Specific to a finite set of 256 ASCII characters, but can be extended to Unicode.
   In this implementation, only printable ASCII characters (32-126) are counted.
   Finite and few additional memory overhead for the map.
3. Binary search tree is O(n) time complexity.
   Unlimited set of characters.
   Worst case time complexity is O(n) when the tree is unbalanced.
   Additional memory overhead for tree structure.
4. AVL, a Balanced binary search tree, is O(log n) time complexity.
   Unlimited set of characters.
   Additional memory overhead for tree structure.
   Not implemented in this code.
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if EFFCHECK == YES
    #define INT_MAX 0x7fffffff  // initialized for min depth calculation
    double log2approx(int x) {
        int n = 0;
        int temp = x;
        while (temp > 1) {
            temp >>= 1;
            n++;
        }
        double x1 = (double)x / (1 << n);

        double xx[] = {1.0, 1.2, 1.4, 1.6, 1.8, 2.0};
        double y[] = {0, 0.26303, 0.48543, 0.67807, 0.84800, 1};
        int in = 6; // number of nodes for lagrange interpolation
        double result = 0;
        for (int i = 0; i < in; i++) { // Largrange interpolation
            double term = y[i];
            for (int j = 0; j < in; j++) {
                if (j != i) {
                    term *= (x1 - xx[j]) / (xx[i] - xx[j]);
                }
            }
            result += term;
        }

        return n + result;
    }
#endif

typedef struct Node Node;
typedef struct List List;
// Reasons why Node Node and List List are used:
// https://stackoverflow.com/questions/1675351/typedef-struct-vs-struct-definitions

struct Node {
    char ch;
    int count;
    Node* next;

    #if SEARCH == BST || SEARCH == AVL
        Node* prev; // for doubly linked-list
        Node* left; // for [balanced] binary search tree
        Node* right;
    #endif

    #if SEARCH == AVL
        int BF; // balance factor, for AVL tree
        Node* parent; // parent node, for AVL tree
    #endif
};

// Note:
// Linked-List is necessary for recording the order of appearance.
// BST and AVL are used to enhance search performance.
// When using BST and AVL, each node hybridizes doubly linked-list structure and binary tree structure.
// This is to maintain the order of appearance and to enhance search performance.

struct List {
    Node* head;
    Node* tail;
    List* (*addch)(struct List* lst, char ch);

    #if SEARCH == HASH
        Node* hash[256];
    #endif

    #if SEARCH == AVL
        Node* root; // root node of the AVL tree
    #endif

    #if EFFCHECK == YES
        int n; // number of nodes created
        int depth; // maximum traversal depth
        int mindepth; // minimum traversal depth
        int searches; // number of searches, for average traversal depth calculation
        int totaldepth; // total traversal depth, for average traversal depth calculation
    #endif
};

#if SEARCH == LINEARSEARCH
// add up character count in a linked-list using linear search
List* addch(List* lst, char ch) {
    #if EFFCHECK == YES
        int trav = 0;
        lst->searches++;
    #endif

    if (lst->head == NULL) {
        lst->head = (Node*)malloc(sizeof(Node));
        lst->head->ch = ch;
        lst->head->count = 1;
        lst->head->next = NULL;
        lst->tail = lst->head;

        #if EFFCHECK == YES
            lst->n++; // new node created
            trav++;
        #endif
    } else {
        Node* cur = lst->head;
        while (cur != NULL) {
            #if EFFCHECK == YES
                trav++;
            #endif

            if (cur->ch == ch) {
                cur->count++;

                #if EFFCHECK == YES // update depth before returning
                    lst->totaldepth += trav;
                    if (trav > lst->depth) {
                        lst->depth = trav;
                    }
                    if (trav < lst->mindepth) {
                        lst->mindepth = trav;
                    }
                #endif

                return lst;
            }
            cur = cur->next;
        }
        lst->tail->next = (Node*)malloc(sizeof(Node));
        lst->tail->next->ch = ch;
        lst->tail->next->count = 1;
        lst->tail->next->next = NULL;
        lst->tail = lst->tail->next;

        #if EFFCHECK == YES
            lst->n++; // new node created
        #endif
    }

    #if EFFCHECK == YES // update depth before returning
        lst->totaldepth += trav;
        if (trav > lst->depth) {
            lst->depth = trav;
        }
        if (trav < lst->mindepth) {
            lst->mindepth = trav;
        }
    #endif

    return lst;
}
#elif SEARCH == HASH
// add up character count in a linked-list using hash table
List* addch(List* lst, char ch) {
    #if EFFCHECK == YES
        lst->searches++;
        int trav = 0;
    #endif

    if (lst->hash[ch] == NULL) {
        lst->hash[ch] = (Node*)malloc(sizeof(Node));
        lst->hash[ch]->ch = ch;
        lst->hash[ch]->count = 1;
        lst->hash[ch]->next = NULL;

        #if EFFCHECK == YES
            lst->n++;
            trav++;
        #endif

        if (lst->head == NULL) {
            lst->head = lst->hash[ch];
            lst->tail = lst->hash[ch];
        } else {
            lst->tail->next = lst->hash[ch];
            lst->tail = lst->hash[ch];
        }
    } else {
        lst->hash[ch]->count++;

        #if EFFCHECK == YES
            trav++;
        #endif
    }

    #if EFFCHECK == YES
        if (lst->searches > 0) {
            lst->totaldepth += trav;
            if (trav > lst->depth) {
                lst->depth = trav;
            }
        }
        if (lst->mindepth > trav) {
            lst->mindepth = trav;
        }
    #endif

    return lst;
}
#elif SEARCH == BST
// add up character count in a linked-list using BST
List* addch(List* lst, char ch) {
    #if EFFCHECK == YES
        lst->searches++;
        int trav = 0;
    #endif

    if (lst->head == NULL) {
        lst->head = (Node*)malloc(sizeof(Node));
        lst->head->ch = ch;
        lst->head->count = 1;
        lst->head->left = NULL;
        lst->head->right = NULL;
        lst->head->next = NULL;
        lst->head->prev = NULL;
        lst->tail = lst->head;

        #if EFFCHECK == YES
            lst->n++;
            trav++;
        #endif
    } else {
        Node* cur = lst->head; // head is the root node
        Node* parent = NULL;
        while (cur != NULL) {
            #if EFFCHECK == YES
                trav++;
            #endif

            if (cur->ch == ch) {
                cur->count++;

                #if EFFCHECK == YES // update depth before returning
                    lst->totaldepth += trav;
                    if (trav > lst->depth) {
                        lst->depth = trav;
                    }
                    if (trav < lst->mindepth) {
                        lst->mindepth = trav;
                    }
                #endif

                return lst;
            } else if (ch < cur->ch) {
                parent = cur;
                cur = cur->left;
            } else {
                parent = cur;
                cur = cur->right;
            }
        }
        Node* newnode = (Node*)malloc(sizeof(Node));
        newnode->ch = ch;
        newnode->count = 1;
        newnode->left = NULL;
        newnode->right = NULL;
        newnode->next = NULL;
        newnode->prev = lst->tail;
        if (ch < parent->ch) {
            parent->left = newnode;
        } else {
            parent->right = newnode;
        }
        lst->tail->next = newnode;
        lst->tail = newnode;

        #if EFFCHECK == YES
            lst->n++;
        #endif
    }

    #if EFFCHECK == YES // update depth before returning
        lst->totaldepth += trav;
        if (trav > lst->depth) {
            lst->depth = trav;
        }
        if (trav < lst->mindepth) {
            lst->mindepth = trav;
        }
    #endif

    return lst;
}
#elif SEARCH == AVL
// add up character count in a linked-list using AVL tree
List* addch(List* lst, char ch) {
    #if EFFCHECK == YES
        lst->searches++;
        int trav = 0;
    #endif

    if (lst->head == NULL) {
        lst->head = (Node*)malloc(sizeof(Node));
        lst->head->ch = ch;
        lst->head->count = 1;
        lst->head->left = NULL;  // for tree
        lst->head->right = NULL; // for tree
        lst->head->next = NULL;  // for linked-list
        lst->head->prev = NULL;  // for linked-list
        lst->head->BF = 0;       // for AVL tree
        lst->head->parent = NULL; // for AVL tree
        lst->root = lst->head;   // for AVL tree
        lst->tail = lst->head;   // only one node, head is also tail

        #if EFFCHECK == YES
            lst->n++;
            trav++;
        #endif
    } else {
        Node* cur = lst->root; // start from the root node
        Node* parent = NULL; // this stores the parent node of the new node in the AVL tree
        while (cur != NULL) {
            #if EFFCHECK == YES
                trav++;
            #endif

            // if the character is found, increment the count and return
            // otherwise, insert a new node later
            if (cur->ch == ch) {
                cur->count++;

                #if EFFCHECK == YES // update depth before returning
                    lst->totaldepth += trav;
                    if (trav > lst->depth) {
                        lst->depth = trav;
                    }
                    if (trav < lst->mindepth) {
                        lst->mindepth = trav;
                    }
                #endif

                return lst;
            } else if (ch < cur->ch) {
                parent = cur;
                cur = cur->left;
            } else {
                parent = cur;
                cur = cur->right;
            }
        } // while-loop breaks when the character is not found in the AVL tree
        // and then we need to insert a new node,
        // which is a bit more complicated than BST

        Node* newnode = (Node*)malloc(sizeof(Node));
        newnode->parent = parent; // parent node of the new node
        newnode->ch = ch; // key value
        newnode->count = 1; // new character, must be 1
        newnode->left = NULL; // default value, should be updated later
        newnode->right = NULL; // default value, should be updated later
        newnode->next = NULL; // must be NULL as it is the last node in the linked-list (newly added)
        newnode->prev = lst->tail; // must be tailed to the linked-list
        newnode->BF = 0; // default value, should be updated later
        lst->tail = newnode; // update the tail of the linked-list

        #if EFFCHECK == YES
            lst->n++; // new node created

            // below can only be rotation-related operations
            // so we can update the statstics here
            lst->totaldepth += trav;
            if (trav > lst->depth) {
                lst->depth = trav;
            }
            if (trav < lst->mindepth) {
                lst->mindepth = trav;
            }
        #endif

        // linked-list is updated in above code
        //
        // below is to update the AVL tree structure, we need to:
        // (1) find the correct position to insert the new node
        // Note: We don't have to redo the search because we already know
        //       the parent node where the new node to be inserted.
        // (2) and then update the balance factor (BF) of the nodes in the path
        // Note: BF is defined as left subtree height - right subtree height
        // Note: parent->BF will definitely change, but its further ancestors may not
        // (3) and then rebalance the tree if necessary
        //
        // Remark: These to-do's are not separate steps,
        //         but they may be implemented together.

        if (ch < parent->ch) {
            parent->left = newnode;
            parent->BF++;
        } else {
            parent->right = newnode;
            parent->BF--;
        }
        
    }
}
#endif

// List constructor
List* new_list() {
    List* lst = (List*)malloc(sizeof(List));
    lst->head = NULL;
    lst->tail = NULL;
    lst->addch = addch;

    #if SEARCH == HASH
        for (int i = 0; i < 256; i++) {
            lst->hash[i] = NULL;
        }
    #endif

    #if EFFCHECK == YES
        lst->n = 0;
        lst->depth = 0;
        lst->mindepth = INT_MAX;
        lst->searches = 0;
        lst->totaldepth = 0;
    #endif
    
    return lst;
}

// List destructor
void free_list(List* lst) {
    Node* cur = lst->head;
    while (cur != NULL) {
        Node* tmp = cur;
        cur = cur->next;
        free(tmp);
    }

    #if SEARCH == HASH // erase hash content
        for (int i = 0; i < 256; i++) {
            lst->hash[i] = NULL;
        }
    #endif

    free(lst);
}

int main() {
    #if EFFCHECK == YES
        int totalprintablechars = 0;
    #endif

    // read from file
    FILE* file = fopen("main.c", "r");
    if (file == NULL) {
        printf("File not found.\n");
        return 1;
    }
    List* lst = new_list();
    char ch;
    while ((ch = fgetc(file)) != EOF) {
        if (ch >= 32 && ch <= 126) { // only printable ASCII characters are counted
            #if EFFCHECK == YES
                totalprintablechars++;
            #endif
            lst = lst->addch(lst, ch);
        }
    }
    fclose(file);

    // print character count in the order of appearance (Linked-List)
    // Note: range is 32-126 (printable ASCII characters)
    // Reference: https://www.ascii-code.com/characters/printable-characters
    Node* cur = lst->head;
    while (cur != NULL) {
        if (cur->ch >= 32 && cur->ch <= 126) {
            printf("%c : %d\n", cur->ch, cur->count);
        }
        cur = cur->next;
    }

    #if EFFCHECK == YES
        printf("Statistics:\n");
        printf("  Number of nodes created: %d\n", lst->n);
        printf("  Number of searches: %d\n", lst->searches);
        printf("  Total printable characters: %d\n", totalprintablechars);
        printf("  Aggregated Traversal Depth: %d\n", lst->totaldepth);
        printf("  Minimum traversal depth: %d\n", lst->mindepth);
        printf("  Average traversal depth: %.2f\n", (float)lst->totaldepth / lst->searches);
        printf("  Maximum traversal depth: %d\n", lst->depth);
        #if SEARCH == AVL
            // AVL tree height estimation
            // https://stackoverflow.com/questions/30769383
            printf("Minimum possible height of the AVL tree: %d\n", (int)log2approx(lst->n + 1));
            printf("Maximum possible height of the AVL tree: %d\n", (int)(1.44*log2approx(lst->n + 2)-0.328));
        #endif
    #endif

    free_list(lst);
    return 0;
}