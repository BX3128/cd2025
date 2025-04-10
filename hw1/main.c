// HW#1, inherited from HW#0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*===========================*/
// Global Variables for HW#1 //
/*===========================*/
#define BUFFERSIZE 1024 // to store the token
#define MAXFILENAME 256 // limit the max filename length
#define EOFF 255        // use 255 to represent EOF (redefine EOF)
#define ERROR_STATE 99  // error state
#define MAXTOKEN 256    // max token length
#define MAXTOKENTYPE 50 // max token type length

// use global variables to store the file content
unsigned char c;
unsigned char buffer[BUFFERSIZE] = {0};
int len = 0;

unsigned char* ptr = NULL; // pointer to the file content cache
size_t loc = 0; // current location in the file content cache

/*===========================*/
// Struct & Enum for HW#1    //
/*===========================*/

typedef enum TokenType {
    TYPE,
    MAIN,
    IF,
    ELSE,
    WHILE,
    ID,
    LITERAL,
    ASSIGN,
    SEMICOLON,
    EQUAL,
    GREATEREQUAL,
    LESSEQUAL,
    GREATER,
    LESS,
    PLUS,
    MINUS,
    LEFTPAREN,
    RIGHTPAREN,
    LEFTBRACE,
    RIGHTBRACE
} TokenType;

typedef struct Token Token;
typedef struct Token {
    char value[MAXTOKEN];
    TokenType type;
    Token* next;
} Token;

typedef struct TokenList {
    Token* head;
    Token* tail;
    int size;
} TokenList;

TokenList tklist = {NULL, NULL, 0}; // initialize the token list

/*===========================*/
// Functions for HW#1        //
/*===========================*/

// Generate a string for the token type, for displaying purposes.
// Note that the variable for output must be long enough.
// No check will be made.
void genTokenType(TokenType no, char* typeName) {
    switch (no) {
        case TYPE:             strcpy(typeName, "TYPE_TOKEN");          break;
        case MAIN:             strcpy(typeName, "MAIN_TOKEN");          break;
        case IF:               strcpy(typeName, "IF_TOKEN");            break;
        case ELSE:             strcpy(typeName, "ELSE_TOKEN");          break;
        case WHILE:            strcpy(typeName, "WHILE_TOKEN");         break;
        case ID:               strcpy(typeName, "ID_TOKEN");            break;
        case LITERAL:          strcpy(typeName, "LITERAL_TOKEN");       break;
        case ASSIGN:           strcpy(typeName, "ASSIGN_TOKEN");        break;
        case SEMICOLON:        strcpy(typeName, "SEMICOLON_TOKEN");     break;
        case EQUAL:            strcpy(typeName, "EQUAL_TOKEN");         break;
        case GREATEREQUAL:     strcpy(typeName, "GREATEREQUAL_TOKEN");  break;
        case LESSEQUAL:        strcpy(typeName, "LESSEQUAL_TOKEN");     break;
        case GREATER:          strcpy(typeName, "GREATER_TOKEN");       break;
        case LESS:             strcpy(typeName, "LESS_TOKEN");          break;
        case PLUS:             strcpy(typeName, "PLUS_TOKEN");          break;
        case MINUS:            strcpy(typeName, "MINUS_TOKEN");         break;
        case LEFTPAREN:        strcpy(typeName, "LEFTPAREN_TOKEN");     break;
        case RIGHTPAREN:       strcpy(typeName, "RIGHTPAREN_TOKEN");    break;
        case LEFTBRACE:        strcpy(typeName, "LEFTBRACE_TOKEN");     break;
        case RIGHTBRACE:       strcpy(typeName, "RIGHTBRACE_TOKEN");    break;
        default:
            strcpy(typeName, "UNKNOWN_TOKEN");
            break;
    }
}

// Add a new token to the list
void appendToken(const char* value, TokenType type) {
    Token* newToken = (Token*)malloc(sizeof(Token));
    strcpy(newToken->value, value);
    newToken->type = type;
    newToken->next = NULL;

    if (tklist.head == NULL) {
        tklist.head = newToken;
        tklist.tail = newToken;
    } else {
        tklist.tail->next = newToken;
        tklist.tail = newToken;
    }
    tklist.size++;
}

void freeTokenList() {
    Token* current = tklist.head;
    while (current != NULL) {
        Token* next = current->next;
        free(current);
        current = next;
    }
    tklist.head = NULL;
    tklist.tail = NULL;
    tklist.size = 0;
}

void printTokenList() {
    Token* current = tklist.head;
    char typeName[MAXTOKENTYPE]; // buffer for token type name
    while (current != NULL) {
        genTokenType(current->type, typeName);
        printf("%s: %s\n", current->value, typeName);
        current = current->next;
    }
}

int in_s() {
    return c == ' ' || c == '\n' || c == '\t' || c == '\r' || c == EOFF || \
           c == '{' || c == '}' || c == '(' || c == ')' || c == ';';
}

int in_n() {
    return c >= '0' && c <= '9';
}

int in_r() {
    return c == '+' || c == '-' || c == '>' || c == '<' || c == '=';
}

int in_a() {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int in_a1() {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') && \
            (c != 'e' && c!= 'i' && c != 'm' && c != 'w');
}

int in_s1() {
    return c == '{' || c == '}' || c == '(' || c == ')' || c == ';';
}

// in_a2() can not be implemented directly
//         because it falls into the "else" branch in the if statement.

void mygetc() { // no EOF check here, it is left for the switch statement
    c = ptr[loc++];
    buffer[len++] = c;
}

void myungetc() {
    loc--;
    len--;
    buffer[len] = '\0'; // reset the buffer to empty
}

void resetBuffer() { // note: buffer is to store the token, not file content
    len = 0;
    buffer[len] = '\0'; // reset the buffer to empty
}

/*===========================*/
// Macros for HW#0           //
/*===========================*/

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

/*=============================*/
// Functions & Struct for HW#0 //
/*=============================*/

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


/*===========================*/
// main()                    //
/*===========================*/

int main(int argc, char* argv[]) {
    char filename[MAXFILENAME];
    size_t fileSize = 0;
    if (argc == 1) {
        strcpy(filename, "sample.c"); // '\0' is added automatically via strcpy()
    } else if (argc >= 2) {
        strcpy(filename, argv[1]);
    }
    
    // estimate the file size
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Error: file %s not found\n", filename);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Allocate a memory cache which the entire file content is to read into.
    unsigned char* content = (unsigned char*)malloc(fileSize + 1);
    if (content == NULL) {
        printf("Error: memory allocation failed\n");
        fclose(fp);
        return 1;
    }
    size_t bytesRead = fread(content, 1, fileSize, fp);
    if (bytesRead != fileSize) {
        printf("Error: read %zu bytes, but expected %lu bytes\n", bytesRead, fileSize);
        free(content);
        fclose(fp);
        return 1;
    }
    content[fileSize] = EOFF; // add a redefined EOF terminator to the end of the content

    fclose(fp);

    // assign the content to the pointer for later use in functions
    ptr = content;
    loc = 0; // reset the location to the beginning of the file content

    int state = 0;
    while (1) {
        switch (state) {
            case 0:
                resetBuffer(); // reset the buffer to empty
                mygetc();      // read the next character
                     if (c == '+') state = 1;
                else if (c == '-') state = 2;
                else if (c == '=') state = 3;
                else if (c == '<') state = 6;
                else if (c == '>') state = 9;
                else if (in_n()) state = 12;
                else if (in_s1()) state = 14;
                else if (in_a1()) state = 37;
                else if (c == EOFF) state = 39;
                else if (c == 'e') state = 15;
                else if (c == 'i') state = 18;
                else if (c == 'm') state = 20;
                else if (c == 'w') state = 23;
                else if (c == ' ') state = 0;
                else if (c == '\n') state = 0;
                else if (c == '\t') state = 0;
                else if (c == '\r') state = 0;
                else {
                    printf("Error in state 0: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 1:
                // printf("+: PLUS_TOKEN\n");
                appendToken("+", PLUS);
                state = 0;
                break;
            case 2:
                // printf("-: MINUS_TOKEN\n");
                appendToken("-", MINUS);
                state = 0;
                break;
            case 3:
                mygetc();
                if (c == '=') state = 4;
                else state = 5;
                break;
            case 4:
                // printf("==: EQUAL_TOKEN\n");
                appendToken("==", EQUAL);
                state = 0;
                break;
            case 5:
                myungetc();
                // printf("=: ASSIGN_TOKEN\n");
                appendToken("=", ASSIGN);
                state = 0;
                break;
            case 6:
                mygetc();
                if (c == '=') state = 7;
                else state = 8;
                break;
            case 7:
                // printf("<=: LESSEQUAL_TOKEN\n");
                appendToken("<=", LESSEQUAL);
                state = 0;
                break;
            case 8:
                myungetc();
                // printf("<: LESS_TOKEN\n");
                appendToken("<", LESS);
                state = 0;
                break;
            case 9:
                mygetc();
                if (c == '=') state = 10;
                else state = 11;
                break;
            case 10:
                // printf(">=: GREATEREQUAL_TOKEN\n");
                appendToken(">=", GREATEREQUAL);
                state = 0;
                break;
            case 11:
                myungetc();
                // printf(">: GREATER_TOKEN\n");
                appendToken(">", GREATER);
                state = 0;
                break;
            case 12:
                mygetc();
                if (in_n()) state = 12;
                else if (in_s() || in_r()) state = 13;
                else if (in_a()) state = ERROR_STATE;
                else {
                    printf("Error in state 12: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 13:
                myungetc();
                buffer[len] = '\0'; // add a null terminator to the buffer
                // printf("%s: LITERAL_TOKEN\n", buffer);
                appendToken(buffer, LITERAL);
                state = 0;
                break;
            case 14:
                buffer[len] = '\0';
                switch (c) {
                    // case '(': printf("(: LEFTPAREN_TOKEN\n"); break;
                    // case ')': printf("): RIGHTPAREN_TOKEN\n"); break;
                    // case '{': printf("{: LEFTBRACE_TOKEN\n"); break;
                    // case '}': printf("}: RIGHTBRACE_TOKEN\n"); break;
                    // case ';': printf(";: SEMICOLON_TOKEN\n"); break;
                    case '(': appendToken("(", LEFTPAREN); break;
                    case ')': appendToken(")", RIGHTPAREN); break;
                    case '{': appendToken("{", LEFTBRACE); break;
                    case '}': appendToken("}", RIGHTBRACE); break;
                    case ';': appendToken(";", SEMICOLON); break;
                    default:
                        printf("Error in state 14: found unexpected character with decimal = %u, represented as %c\n", c, c);
                        free(content); // free the memory cache
                        freeTokenList();
                        return 1;
                }
                state = 0;
                break;
            case 15:
                mygetc();
                if (c == 'l') state = 16;
                else if (in_n() || in_a()) state = 37; // because c == 'l' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 15: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 16:
                mygetc();
                if (c == 's') state = 17;
                else if (in_n() || in_a()) state = 37; // because c == 's' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 16: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 17:
                mygetc();
                if (c == 'e') state = 27;
                else if (in_n() || in_a()) state = 37; // because c == 'e' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 17: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 18:
                mygetc();
                if (c == 'f') state = 28;
                else if (c == 'n') state = 19;
                else if (in_n() || in_a()) state = 37; // because c == 'f' and 'n' are used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 18: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 19:
                mygetc();
                if (c == 't') state = 29;
                else if (in_n() || in_a()) state = 37; // because c == 't' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 19: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 20:
                mygetc();
                if (c == 'a') state = 21;
                else if (in_n() || in_a()) state = 37; // because c == 'a' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 20: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 21:
                mygetc();
                if (c == 'i') state = 22;
                else if (in_n() || in_a()) state = 37; // because c == 'i' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 21: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 22:
                mygetc();
                if (c == 'n') state = 30;
                else if (in_n() || in_a()) state = 37; // because c == 'n' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 22: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 23:
                mygetc();
                if (c == 'h') state = 24;
                else if (in_n() || in_a()) state = 37; // because c == 'h' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 23: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 24:
                mygetc();
                if (c == 'i') state = 25;
                else if (in_n() || in_a()) state = 37; // because c == 'i' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 24: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 25:
                mygetc();
                if (c == 'l') state = 26;
                else if (in_n() || in_a()) state = 37; // because c == 'l' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 25: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 26:
                mygetc();
                if (c == 'e') state = 31;
                else if (in_n() || in_a()) state = 37; // because c == 'e' is used, here in_a() is actually in_a2()
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 26: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 27:
                mygetc();
                if (in_s() || in_r()) state = 32;
                else if (in_n() || in_a()) state = 37;
                else {
                    printf("Error in state 27: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 28:
                mygetc();
                if (in_s() || in_r()) state = 33;
                else if (in_n() || in_a()) state = 37;
                else {
                    printf("Error in state 28: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 29:
                mygetc();
                if (in_s() || in_r()) state = 34;
                else if (in_n() || in_a()) state = 37;
                else {
                    printf("Error in state 29: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 30:
                mygetc();
                if (in_s() || in_r()) state = 35;
                else if (in_n() || in_a()) state = 37;
                else {
                    printf("Error in state 30: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 31:
                mygetc();
                if (in_s() || in_r()) state = 36;
                else if (in_n() || in_a()) state = 37;
                else {
                    printf("Error in state 31: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 32:
                myungetc();
                // printf("else: ELSE_TOKEN\n");
                appendToken("else", ELSE);
                state = 0;
                break;
            case 33:
                myungetc();
                // printf("if: IF_TOKEN\n");
                appendToken("if", IF);
                state = 0;
                break;
            case 34:
                myungetc();
                // printf("int: TYPE_TOKEN\n");
                appendToken("int", TYPE);
                state = 0;
                break;
            case 35:
                myungetc();
                // printf("main: MAIN_TOKEN\n");
                appendToken("main", MAIN);
                state = 0;
                break;
            case 36:
                myungetc();
                // printf("while: WHILE_TOKEN\n");
                appendToken("while", WHILE);
                state = 0;
                break;
            case 37:
                mygetc();
                if (in_n() || in_a()) state = 37;
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 37: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    freeTokenList();
                    return 1;
                }
                break;
            case 38:
                myungetc();
                buffer[len] = '\0'; // add a null terminator to the buffer
                // printf("%s: ID_TOKEN\n", buffer);
                appendToken(buffer, ID);
                state = 0;
                break;
            case 39: // End of process
                printTokenList(); // print the token list
                free(content);
                freeTokenList();
                return 0;
            default:
                printf("Error due to invalid token detected.\n");
                printf("Alphabet character should not be followed by a digit\n");
                printf("when the token is to be determined as an integer.\n");
                free(content);
                freeTokenList();
                return 1;
        }
    }

    // free(content); // free the memory cache
    // freeTokenList(); // free the token list

    return 0;

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