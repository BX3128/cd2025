#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLEFILE "sample.txt" // default file name

#define BUFFERSIZE 1024 // to store the token
#define MAXFILENAME 256 // limit the max filename length
#define EOFF 255        // use 255 to represent EOF (redefine EOF)
#define ERROR_STATE 99  // error state
#define LINEMAX 128 // max length of a line in the AST output

// use global variables to store the file content
unsigned char c;
unsigned char buffer[BUFFERSIZE] = {0};
int len = 0;

unsigned char* ptr = NULL; // pointer to the file content cache
size_t loc = 0; // current location in the file content cache

typedef enum TokenType {
    PLUS_TOKEN,
    MINUS_TOKEN,
    EQUAL_TOKEN,
    ASSIGN_TOKEN,
    LESS_TOKEN,
    LESSEQUAL_TOKEN,
    GREATER_TOKEN,
    GREATEREQUAL_TOKEN,
    LITERAL_TOKEN,
    ID_TOKEN,
    LEFTPAREN_TOKEN,
    RIGHTPAREN_TOKEN,
    LEFTBRACE_TOKEN,
    RIGHTBRACE_TOKEN,
    SEMICOLON_TOKEN,
    TYPE_TOKEN,
    MAIN_TOKEN,
    WHILE_TOKEN,
    IF_TOKEN,
    ELSE_TOKEN,
    EOF_TOKEN
} TokenType;

typedef struct Token Token;
typedef struct TokenList TokenList;
typedef struct ASTNode ASTNode;
typedef struct AST AST;
typedef struct ASTOutputLn ASTOutputLn;
typedef struct ASTOutput ASTOutput;

typedef struct Token {
    TokenType type;
    union {
        int intval;
        char* strval;
    };
    Token* next;
} Token;

typedef struct TokenList {
    Token* first;
    Token* last;
    Token* current; // current token for iteration
    size_t count;
} TokenList;

void addToken(TokenList* list, Token token) {
    Token* newToken = (Token*)malloc(sizeof(Token));
    if (newToken == NULL) {
        printf("Error: memory allocation failed\n");
        return;
    }
    *newToken = token;
    if (newToken->type != LITERAL_TOKEN) {
        newToken->strval = NULL;
    }
    newToken->next = NULL;

    if (list->first == NULL) {
        list->first = newToken;
    } else {
        list->last->next = newToken;
    }
    list->last = newToken;
    list->count++;
}

void freeTokenList(TokenList* list) {
    Token* current = list->first;
    while (current != NULL) {
        Token* next = current->next;
        if (current->type == ID_TOKEN || current->type == LITERAL_TOKEN) {
            free(current->strval);  // strval 是指向 strdup 的字串
        }
        free(current);
        current = next;
    }
    list->first = NULL;
    list->last = NULL;
    list->count = 0;
}

typedef enum NonterminalType {
    S,
    Sp,
    E,
    epsilon
} NonterminalType;

typedef enum SymbolKind {
    Terminal,
    Nonterminal
} SymbolKind;

typedef struct ASTNode {
    SymbolKind kind;
    union {
        TokenType token;
        NonterminalType nonterminal;
    };
    ASTNode* firstChild; // first child
    ASTNode* sibling; // right sibling
    ASTNode* lastChild; // last child
    ASTNode* parent; // parent node
    int childCount; // number of children
    int loc; // for printing, the x coordinate in the printing blackboard
             // 0 is the leftmost position
    union {
        char* strval; // for ID_TOKEN and TYPE_TOKEN
        int intval;  // for LITERAL_TOKEN
    };
} ASTNode;

typedef struct ASTOutputLn {
    char line[LINEMAX];
    int* locs; // array of x coordinates for each same symbol when expanded
    int locCount; // number of x coordinates in locs
    ASTOutputLn* next; // pointer to the next line in the output
    /*
    `locs` is a dynamically allocated array of x-coordinate positions (integers)
    used to indicate where vertical connector bars '|' should be rendered in the
    ASCII output of the Abstract Syntax Tree (AST). Each value in `locs` marks
    the horizontal position of a nonterminal symbol in the current line that
    has been or will be expanded in a subsequent line.

    This is essential for aligning multi-line expansions: any child line
    beginning with "X -> ..." must have its 'X' character aligned directly
    under the corresponding symbol in the parent line.

    For example:
    Line 0: S -> E S'
    Line 1:      E -> 3
    The `E` in line 1 is an expansion of the `E` at position 5 in line 0,
    so 5 will be recorded in `locs`. This allows a '|' to be drawn at column 5
    if lines are inserted between them.

    When a new line is inserted between line 0 and line 1:
    Line 0: S -> E S'
    Line 1:      | S' -> + S
    Line 2:      E -> 3
    Line 2 was previously line 1, and it still needs a '|' at position 5.
    In line 1, `locsCount` will be 2:
      - `locs[0]` = 5 (inherited from the original line 1)
      - `locs[1]` = 7 (for the 'S'' being expanded in line 1)

    This demonstrates how each line can inherit `locs` from the line below
    (i.e., line n inherits from line n+1), or from its previous version.

    If another line is inserted:
    Line 0: S -> E S'
    Line 1:      | S' -> + S
    Line 2:      |         S -> 9
    Line 3:      E -> 3
    Line 3 was previously line 2 and still requires a '|' at column 5.
    Line 2 now has `locsCount` = 2:
      - `locs[0]` = 5 (inherited from what is now line 3)
      - `locs[1]` = 7 (for the 'S' expanded in line 2)
    */
} ASTOutputLn;
typedef struct ASTOutput {
    ASTOutputLn* blackboard;
    int lineCount; // number of lines in the output
} ASTOutput;

typedef struct AST {
    ASTNode* root;
    int nodeCount;
    ASTNode* current; // node to be operated
} AST;

void printNodeInfo(ASTNode* node) {
    if (node != NULL) {
        if (node->kind == Nonterminal) {
            switch (node->nonterminal) {
                case S: printf("S"); break;
                case Sp: printf("S'"); break;
                case E: printf("E"); break;
            }
        } else {
            switch (node->token) {
                case PLUS_TOKEN: printf("+"); break;
                case MINUS_TOKEN: printf("-"); break;
                case EQUAL_TOKEN: printf("=="); break;
                case ASSIGN_TOKEN: printf("="); break;
                case LESS_TOKEN: printf("<"); break;
                case LESSEQUAL_TOKEN: printf("<="); break;
                case GREATER_TOKEN: printf(">"); break;
                case GREATEREQUAL_TOKEN: printf(">="); break;
                case LITERAL_TOKEN: printf("%d", node->intval); break;
                case ID_TOKEN: printf("%s", node->strval); break;
                case LEFTPAREN_TOKEN: printf("("); break;
                case RIGHTPAREN_TOKEN: printf(")"); break;
                case LEFTBRACE_TOKEN: printf("{"); break;
                case RIGHTBRACE_TOKEN: printf("}"); break;
                case SEMICOLON_TOKEN: printf(";"); break;
                case TYPE_TOKEN: printf("%s", node->strval); break;
                case MAIN_TOKEN: printf("main"); break;
                case WHILE_TOKEN: printf("while"); break;
                case IF_TOKEN: printf("if"); break;
                case ELSE_TOKEN: printf("else"); break;
            }
        }
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

int scanner(const char* filename, TokenList* tokenList, int useDefault) {
    // estimate the file size
    size_t fileSize = 0;
    FILE* fp;
    unsigned char* content;
    if (useDefault == 0) {
        fp = fopen(filename, "rb");
        if (fp == NULL) {
            printf("Error: file %s not found\n", filename);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // Allocate a memory cache which the entire file content is to read into.
        content = (unsigned char*)malloc(fileSize + 1);
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
        fclose(fp);
    } else {
        content = strdup("(1+2+(3+4))+5 ");
        fileSize = strlen(content);
    }
    content[fileSize] = EOFF; // add a redefined EOF terminator to the end of the content

    // initialize the token list
    tokenList->first = NULL;
    tokenList->last = NULL;
    tokenList->current = NULL;
    tokenList->count = 0;

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
                    return 1;
                }
                break;
            case 1:
                // printf("+: PLUS_TOKEN\n");
                addToken(tokenList, (Token){.type = PLUS_TOKEN});
                tokenList->last->strval = strdup("+");
                state = 0;
                break;
            case 2:
                // printf("-: MINUS_TOKEN\n");
                addToken(tokenList, (Token){.type = MINUS_TOKEN});
                tokenList->last->strval = strdup("-");
                state = 0;
                break;
            case 3:
                mygetc();
                if (c == '=') state = 4;
                else state = 5;
                break;
            case 4:
                // printf("==: EQUAL_TOKEN\n");
                addToken(tokenList, (Token){.type = EQUAL_TOKEN});
                state = 0;
                break;
            case 5:
                myungetc();
                // printf("=: ASSIGN_TOKEN\n");
                addToken(tokenList, (Token){.type = ASSIGN_TOKEN});
                state = 0;
                break;
            case 6:
                mygetc();
                if (c == '=') state = 7;
                else state = 8;
                break;
            case 7:
                // printf("<=: LESSEQUAL_TOKEN\n");
                addToken(tokenList, (Token){.type = LESSEQUAL_TOKEN});
                state = 0;
                break;
            case 8:
                myungetc();
                // printf("<: LESS_TOKEN\n");
                addToken(tokenList, (Token){.type = LESS_TOKEN});
                state = 0;
                break;
            case 9:
                mygetc();
                if (c == '=') state = 10;
                else state = 11;
                break;
            case 10:
                // printf(">=: GREATEREQUAL_TOKEN\n");
                addToken(tokenList, (Token){.type = GREATEREQUAL_TOKEN});
                state = 0;
                break;
            case 11:
                myungetc();
                // printf(">: GREATER_TOKEN\n");
                addToken(tokenList, (Token){.type = GREATER_TOKEN});
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
                    return 1;
                }
                break;
            case 13:
                myungetc();
                buffer[len] = '\0'; // add a null terminator to the buffer
                // printf("%s: LITERAL_TOKEN\n", buffer);
                addToken(tokenList, (Token){.type = LITERAL_TOKEN, .intval = atoi(buffer)});
                state = 0;
                break;
            case 14:
                buffer[len] = '\0';
                switch (c) {
                    case '(':
                        // printf("(: LEFTPAREN_TOKEN\n");
                        addToken(tokenList, (Token){.type = LEFTPAREN_TOKEN});
                        tokenList->last->strval = strdup("(");
                        break;
                    case ')':
                        // printf("): RIGHTPAREN_TOKEN\n");
                        addToken(tokenList, (Token){.type = RIGHTPAREN_TOKEN});
                        tokenList->last->strval = strdup(")");
                        break;
                    case '{':
                        // printf("{: LEFTBRACE_TOKEN\n");
                        addToken(tokenList, (Token){.type = LEFTBRACE_TOKEN});
                        break;
                    case '}':
                        // printf("}: RIGHTBRACE_TOKEN\n");
                        addToken(tokenList, (Token){.type = RIGHTBRACE_TOKEN});
                        break;
                    case ';':
                        // printf(";: SEMICOLON_TOKEN\n");
                        addToken(tokenList, (Token){.type = SEMICOLON_TOKEN});
                        break;
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
                    return 1;
                }
                break;
            case 27:
                // printf("in state 27: ");
                mygetc();
                // printf("get a c = %d (%c) at loc = %zu\n", c, c, loc);
                // printf("in_s() = %d, in_r() = %d, in_n() = %d, in_a() = %d\n", in_s(), in_r(), in_n(), in_a());
                if (in_s() || in_r()) state = 32;
                else if (in_n() || in_a()) state = 37;
                else {
                    printf("Error in state 27: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
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
                    return 1;
                }
                break;
            case 32:
                myungetc();
                // printf("else: ELSE_TOKEN\n");
                addToken(tokenList, (Token){.type = ELSE_TOKEN});
                state = 0;
                break;
            case 33:
                myungetc();
                // printf("if: IF_TOKEN\n");
                addToken(tokenList, (Token){.type = IF_TOKEN});
                state = 0;
                break;
            case 34:
                myungetc();
                // printf("int: TYPE_TOKEN\n");
                addToken(tokenList, (Token){.type = TYPE_TOKEN, .strval = strdup("int")});
                state = 0;
                break;
            case 35:
                myungetc();
                // printf("main: MAIN_TOKEN\n");
                addToken(tokenList, (Token){.type = MAIN_TOKEN});
                state = 0;
                break;
            case 36:
                myungetc();
                // printf("while: WHILE_TOKEN\n");
                addToken(tokenList, (Token){.type = WHILE_TOKEN});
                state = 0;
                break;
            case 37:
                mygetc();
                if (in_n() || in_a()) state = 37;
                else if (in_s() || in_r()) state = 38;
                else {
                    printf("Error in state 37: found unexpected character with decimal = %u, represented as %c\n", c, c);
                    free(content); // free the memory cache
                    return 1;
                }
                break;
            case 38:
                myungetc();
                buffer[len] = '\0'; // add a null terminator to the buffer
                // printf("%s: ID_TOKEN\n", buffer);
                addToken(tokenList, (Token){.type = ID_TOKEN, .strval = strdup(buffer)});
                state = 0;
                break;
            case 39: // End of process
                // append EOF token at the end of the token list
                addToken(tokenList, (Token){.type = EOF_TOKEN});
                tokenList->last->strval = strdup("EOF");
                return 0;
            default:
                printf("Error due to invalid token detected.\n");
                printf("Alphabet character should not be followed by a digit\n");
                printf("when the token is to be determined as an integer.\n");
                return 1;
        }
    }

    free(content); // free the memory cache if we reach here (should not happen)
}

void printTokenList(const TokenList* list) {
    printf("Printing Token List: ");
    if (list->count == 0) {
        printf("empty token list.\n");
        return;
    }
    printf("total %zu tokens\n", list->count-1); // exclude the EOF token
    Token* current = list->first;
    while (current != NULL) {
        switch (current->type) {
            case PLUS_TOKEN:
                printf("+: PLUS_TOKEN (%d)\n", PLUS_TOKEN);
                break;
            case MINUS_TOKEN:
                printf("-: MINUS_TOKEN (%d)\n", MINUS_TOKEN);
                break;
            case EQUAL_TOKEN:
                printf("==: EQUAL_TOKEN (%d)\n", EQUAL_TOKEN);
                break;
            case ASSIGN_TOKEN:
                printf("=: ASSIGN_TOKEN (%d)\n", ASSIGN_TOKEN);
                break;
            case LESS_TOKEN:
                printf("<: LESS_TOKEN (%d)\n", LESS_TOKEN);
                break;
            case LESSEQUAL_TOKEN:
                printf("<=: LESSEQUAL_TOKEN (%d)\n", LESSEQUAL_TOKEN);
                break;
            case GREATER_TOKEN:
                printf(">: GREATER_TOKEN (%d)\n", GREATER_TOKEN);
                break;
            case GREATEREQUAL_TOKEN:
                printf(">=: GREATEREQUAL_TOKEN (%d)\n", GREATEREQUAL_TOKEN);
                break;
            case LITERAL_TOKEN:
                printf("%d: LITERAL_TOKEN (%d)\n", current->intval, LITERAL_TOKEN);
                break;
            case ID_TOKEN:
                printf("%s: ID_TOKEN (%d)\n", current->strval, ID_TOKEN);
                break;
            case LEFTPAREN_TOKEN:
                printf("(: LEFTPAREN_TOKEN (%d)\n", LEFTPAREN_TOKEN);
                break;
            case RIGHTPAREN_TOKEN:
                printf("): RIGHTPAREN_TOKEN (%d)\n", RIGHTPAREN_TOKEN);
                break;
            case LEFTBRACE_TOKEN:
                printf("{: LEFTBRACE_TOKEN (%d)\n", LEFTBRACE_TOKEN);
                break;
            case RIGHTBRACE_TOKEN:
                printf("}: RIGHTBRACE_TOKEN (%d)\n", RIGHTBRACE_TOKEN);
                break;
            case SEMICOLON_TOKEN:
                printf(";: SEMICOLON_TOKEN (%d)\n", SEMICOLON_TOKEN);
                break;
            case TYPE_TOKEN:
                printf("%s: TYPE_TOKEN (%d)\n", current->strval, TYPE_TOKEN);
                break;
            case MAIN_TOKEN:
                printf("main: MAIN_TOKEN (%d)\n", MAIN_TOKEN);
                break;
            case WHILE_TOKEN:
                printf("while: WHILE_TOKEN (%d)\n", WHILE_TOKEN);
                break;
            case IF_TOKEN:
                printf("if: IF_TOKEN (%d)\n", IF_TOKEN);
                break;
            case ELSE_TOKEN:
                printf("else: ELSE_TOKEN (%d)\n", ELSE_TOKEN);
                break;
            case EOF_TOKEN:
                printf("EOF: EOF_TOKEN (%d); this token is not counted.\n", EOF_TOKEN);
                break;
            default:
                printf("Unknown token type: %d\n", current->type);
                break;
        }
        current = current->next;
    }
}

void addASTNode(AST* ast, ASTNode* node, ASTNode* parent) {
    if (ast->root == NULL) {
        ast->root = node;
    } else {
        if (parent->firstChild == NULL) {
            parent->firstChild = node;
            parent->lastChild = node;
        } else { // parent already has children
            parent->lastChild->sibling = node;
            parent->lastChild = node; // update the last child to the new node
            // if the parent has no children, set the first child to the new node
        }
    }
    node->parent = parent; // set the parent of the new node in the AST
    node->sibling = NULL; // initialize sibling to NULL
    ast->current = node; // set the current node to the new node
    ast->nodeCount++;
}

// for creating a nonterminal AST node
ASTNode* createASTNode(NonterminalType nonterminal, int loc) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "ERROR: memory allocation failed for ASTNode\n");
        return NULL;
    }
    // node->strval = NULL; // nonterminal nodes do not have strval
    switch (nonterminal) {
        case S: node->strval = strdup("S"); break;
        case Sp: node->strval = strdup("S'"); break;
        case E: node->strval = strdup("E"); break;
        case epsilon: node->strval = strdup("epsilon"); break;
        default:
            fprintf(stderr, "ERROR: unknown nonterminal type %d\n", nonterminal);
            free(node);
            return NULL;
    }
    node->kind = Nonterminal;
    node->nonterminal = nonterminal;
    node->firstChild = NULL;
    node->sibling = NULL;
    node->lastChild = NULL;
    node->parent = NULL;
    node->childCount = 0;
    node->loc = loc;
    return node;
}

// for creating a terminal AST node
ASTNode* createASTNodeT(TokenType token, int loc) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        fprintf(stderr, "ERROR: memory allocation failed for ASTNode\n");
        return NULL;
    }
    if (token != LITERAL_TOKEN && token != ID_TOKEN) {
        switch (token) {
            case LEFTPAREN_TOKEN: node->strval = strdup("("); break;
            case RIGHTPAREN_TOKEN: node->strval = strdup(")"); break;
            case PLUS_TOKEN: node->strval = strdup("+"); break;
            default:
                node->strval = NULL; // other tokens are not assigned a string value
                break;
        }
    }
    node->kind = Terminal;
    node->token = token;
    node->firstChild = NULL; // terminal nodes do not have children
    node->sibling = NULL; // no sibling yet
    node->lastChild = NULL; // terminal nodes do not have children
    node->parent = NULL; // parent will be set later
    node->childCount = 0; // terminal nodes do not have children
    node->loc = loc;
    return node;
}

/* Grammar:
S -> E S'
S' -> ε
S' -> + S
E -> num   (note: num is 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9)
E -> ( S )
*/

// Use recursive descent parser to parse the token list
// The following functions are specific to the test string: (1+2+(3+4))+5
void parse_S(AST* ast, TokenList* tokenlist, int loc);
void parse_Sp(AST* ast, TokenList* tokenlist, int loc);
void parse_E(AST* ast, TokenList* tokenlist, int loc);

int hasError = 0;
#define ERR if (hasError) return;
#define ERR2 if (hasError) return 0;
#define ERR3(ptr) if (ptr == NULL) { \
    hasError = 1; \
    return; \
}
#define ERR4(ptr) if (ptr == NULL) { \
    hasError = 1; \
    return 0; \
}

void parse_S(AST* ast, TokenList* tokenlist, int loc) {
    // if (tokenlist->current->type != LITERAL_TOKEN) {
    //     printf("Parsing S at %d, dealing token %s (%d)\n", loc,
    //            tokenlist->current->strval, tokenlist->current->type);
    // } else {
    //     printf("Parsing S at %d, dealing token %d (%d)\n", loc,
    //            tokenlist->current->intval, tokenlist->current->type);
    // }
    ASTNode* nodeS = ast->current; // nodeS already exists for parse_S
    switch (tokenlist->current->type) {
        case LITERAL_TOKEN:
        case LEFTPAREN_TOKEN:
            // apply S -> E S'
            ASTNode* nodeE = createASTNode(E, loc+5); ERR3(nodeE)
            ASTNode* nodeSp = createASTNode(Sp, loc+7); ERR3(nodeSp)
            addASTNode(ast, nodeE, nodeS); // add E as a child of S (1st child)
            addASTNode(ast, nodeSp, nodeS); // add S' as a child of S (2nd child)
            ast->current = nodeE; // set the current node to E for the next parse
            parse_E(ast, tokenlist, loc+5); ERR
            // ast->current = ast->current->parent; // this should be equivalent to the following line
            ast->current = nodeSp; // set the current node to Sp for the next parse
            parse_Sp(ast, tokenlist, loc+7); ERR
            return;
        default:
            fprintf(stderr, "ERROR in parsing S\n");
            fprintf(stderr, "Handling token type %d at location %d\n", tokenlist->current->type, loc);
            hasError = 1;
            return;
    }
}

void parse_Sp(AST* ast, TokenList* tokenlist, int loc) {
    // if (tokenlist->current->type != LITERAL_TOKEN) {
    //     printf("Parsing S' at %d, dealing token %s (%d)\n", loc,
    //            tokenlist->current->strval, tokenlist->current->type);
    // } else {
    //     printf("Parsing S' at %d, dealing token %d (%d)\n", loc,
    //            tokenlist->current->intval, tokenlist->current->type);
    // }
    ASTNode* nodeSp = ast->current; // nodeSp already exists for parse_Sp
    switch (tokenlist->current->type) {
        case PLUS_TOKEN:
            // apply S' -> + S
            tokenlist->current = tokenlist->current->next; // consume the '+' token
            ASTNode* nodePlus = createASTNodeT(PLUS_TOKEN, loc+6); ERR3(nodePlus)
            ASTNode* nodeS = createASTNode(S, loc+8); ERR3(nodeS)
            addASTNode(ast, nodePlus, nodeSp); // add '+' as a child of S' (1st child)
            addASTNode(ast, nodeS, nodeSp); // add S as a child of S' (2nd child)
            ast->current = nodeS; // set the current node to S for the next parse
            parse_S(ast, tokenlist, loc+8); ERR
            return;
        case RIGHTPAREN_TOKEN:
        case EOF_TOKEN: // end of file
            // apply S' -> ε
            // nodeSp->childCount = 0; // S' has no children in this case, but no need to set it
            ASTNode* nodeEpsilon = createASTNode(epsilon, loc+6); ERR3(nodeEpsilon)
            addASTNode(ast, nodeEpsilon, nodeSp); // add the ε node as a child of S' (1st child)
            // Should this token (RIGHTPAREN_TOKEN or EOF_TOKEN) be consumed?
            // Ans: No, because S' is the last production in the grammar <-- verify this later
            return; // ε production
        default:
            fprintf(stderr, "ERROR in parsing S'\n");
            hasError = 1;
            return;
    }
}

void parse_E(AST* ast, TokenList* tokenlist, int loc) {
    // if (tokenlist->current->type != LITERAL_TOKEN) {
    //     printf("Parsing E at %d, dealing token %s (%d)\n", loc,
    //            tokenlist->current->strval, tokenlist->current->type);
    // } else {
    //     printf("Parsing E at %d, dealing token %d (%d)\n", loc,
    //            tokenlist->current->intval, tokenlist->current->type);
    // }
    ASTNode* nodeE = ast->current; // nodeE already exists for parse_E
    switch (tokenlist->current->type) {
        case LITERAL_TOKEN:
            // apply E -> num
            nodeE->childCount = 1; // E has one child: the literal token
            ASTNode* nodeNum = createASTNodeT(LITERAL_TOKEN, loc+5); ERR3(nodeNum)
            nodeNum->intval = tokenlist->current->intval; // retrieve the value before consuming the token
            addASTNode(ast, nodeNum, nodeE); // add the literal token as a child of E
            tokenlist->current = tokenlist->current->next; // consume the literal token after retrieving its value
            return;
        case LEFTPAREN_TOKEN:
            // apply E -> ( S )
            tokenlist->current = tokenlist->current->next; // consume the '(' token
            ASTNode* nodeLParen = createASTNodeT(LEFTPAREN_TOKEN, loc+5); ERR3(nodeLParen)
            ASTNode* nodeS = createASTNode(S, loc+7); ERR3(nodeS)
            ASTNode* nodeRParen = createASTNodeT(RIGHTPAREN_TOKEN, loc+9); ERR3(nodeRParen)
            addASTNode(ast, nodeLParen, nodeE); // add '(' as a child of E (1st child)
            addASTNode(ast, nodeS, nodeE); // add S as a child of E (2nd child)
            addASTNode(ast, nodeRParen, nodeE); // add ')' as a child of E (3rd child)
            ast->current = nodeS; // set the current node to S for the next parse
            parse_S(ast, tokenlist, loc+7); // parse the expression inside the parentheses
            if (tokenlist->current->type != RIGHTPAREN_TOKEN) {
                fprintf(stderr, "ERROR: expected RIGHTPAREN_TOKEN but found %d\n", tokenlist->current->type);
                hasError = 1;
                return;
            }
            tokenlist->current = tokenlist->current->next; // consume the ')' token
            return;
        default:
            fprintf(stderr, "ERROR in parsing E\n");
            hasError = 1;
            return;
    }
}

// print space
void sp(int n) {
    for (int i = 0; i < n; i++) {
        printf(" ");
    }
}

void printAST(AST* ast) {
    if (ast->root == NULL) {
        printf("printAST: AST is empty.\n");
        return;
    }
    ASTNode* current = ast->current; // this will be altered during processing
    if (current == NULL) {
        printf("printAST: Current node is NULL.\n");
        return;
    }
    ASTNode* currentChild = current->firstChild; // for traversing children
    sp(current->loc);
    printf("%s ->", current->strval); // this must be a nonterminal node
    current = current->firstChild;
    while (current != NULL) {
        printf(" ");
        if (current->kind == Nonterminal) {
            printf("%s", current->strval);
        } else if (current->token != LITERAL_TOKEN) { // below must be a terminal node
            printf("%s", current->strval);
        } else { // terminal node with LITERAL_TOKEN
            printf("%d", current->intval);
        }
        current = current->sibling; // move to the next sibling
    }
    printf("\n");

    // restore the current node to its original state and traverse its children
    current = currentChild;
    while (current != NULL) {
        if (current->kind == Nonterminal) {
            ast->current = current;
            if (current->nonterminal != epsilon) { // do not print ε nodes
                printAST(ast);
            }
        }
        current = current->sibling;
    }
}

int rdparser(AST* ast, TokenList* tokenlist) {
    // printf("Starting recursive descent parser...\n");
    hasError = 0; // reset the error state
    ast->nodeCount = 0; // reset the node count
    ast->root = NULL; // initialize the root of the AST
    ast->current = NULL; // initialize the current node of the AST
    ast->nodeCount = 0; // initialize the node count of the AST
    tokenlist->current = tokenlist->first;
    if (tokenlist->current == NULL) {
        fprintf(stderr, "ERROR: token list is empty\n");
        return 0; // return 0 if the token list is empty
    }
    ASTNode* root = createASTNode(S, 0); ERR4(root)
    root->strval = strdup("S"); // set the name of the root node
    // printf("Root node created: %s %p\n", root->strval, (void*)root);
    addASTNode(ast, root, NULL); // add the root node to the AST
    parse_S(ast, tokenlist, 0); ERR2
    return 1; // return 1 if parsing is successful
}

int main(int argc, char* argv[]) {
    char filename[MAXFILENAME];
    int useDefault = 0;
    if (argc == 1) {
        useDefault = 1;
        strcpy(filename, SAMPLEFILE); // '\0' is added automatically via strcpy()
    } else if (argc >= 2) {
        strcpy(filename, argv[1]);
        printf("Using file: %s\n", filename);
    }
    
    // Scanner
    TokenList tokenList;
    int state = scanner(filename, &tokenList, useDefault);
    if (state != 0) {
        fprintf(stderr, "Scanner error!\n");
        return 1;
    } else {
        // printf("Scanner completed successfully.\n");
    }

    // Print the token list
    // printTokenList(&tokenList);

    // Parser: to generate the AST
    // printf("\nStarting parser...\n");
    AST* ast = (AST*)malloc(sizeof(AST));
    if (ast == NULL) {
        fprintf(stderr, "ERROR: memory allocation failed for AST\n");
        return 1;
    }
    rdparser(ast, &tokenList);
    if (hasError) {
        fprintf(stderr, "Parser error!\n");
        return 1;
    } else {
        // printf("Parser completed successfully.\n");
    }

    // print the AST
    // printf("There are %d nodes in the AST.\n", ast->nodeCount);
    ast->current = ast->root;
    printAST(ast);

    // free the memory allocated for the token list, the AST, and the AST nodes
    // and all mallocated strings inside ASTNode and Token
    Token* current = tokenList.first;
    while (current != NULL) {
        Token* next = current->next;
        if (current->strval != NULL && current->type != LITERAL_TOKEN) {
            free(current->strval); // free the string if it is allocated
        }
        free(current);
        current = next;
    }
    // printf("Token list freed.\n");

    // free the AST nodes
    ASTNode* astCurrent = ast->root;
    while (astCurrent != NULL) {
        ASTNode* astNext = astCurrent->sibling;
        if (astCurrent->strval != NULL) {
            free(astCurrent->strval); // free the string if it is allocated
        }
        free(astCurrent);
        astCurrent = astNext;
    }
    free(ast);
    // printf("AST nodes freed.\n");

    // free the content of the file
    free(ptr); // free the memory cache allocated in scanner

    return 0;
}