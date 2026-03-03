#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "org_tree.h"

// Helper function to parse a single node from text starting at ptr
static Node* parse_node(char **ptr) {
    Node *node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        return NULL;
    }
    
    // Initialize node
    node->left = NULL;
    node->right = NULL;
    node->supports_head = NULL;
    node->next = NULL;
    
    // Find and extract First Name
    char *field = strstr(*ptr, "First Name: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("First Name: ");
    int i = 0;
    while (*field != '\n' && *field != '\0' && i < MAX_FIELD - 1) {
        node->first[i++] = *field++;
    }
    node->first[i] = '\0';
    
    // Find and extract Second Name
    field = strstr(field, "Second Name: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("Second Name: ");
    i = 0;
    while (*field != '\n' && *field != '\0' && i < MAX_FIELD - 1) {
        node->second[i++] = *field++;
    }
    node->second[i] = '\0';
    
    // Find and extract Fingerprint
    field = strstr(field, "Fingerprint: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("Fingerprint: ");
    i = 0;
    while (*field != '\n' && *field != '\0' && i < MAX_FIELD - 1) {
        node->fingerprint[i++] = *field++;
    }
    node->fingerprint[i] = '\0';
    
    // Find and extract Position
    field = strstr(field, "Position: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("Position: ");
    i = 0;
    while (*field != '\n' && *field != '\0' && i < MAX_POS - 1) {
        node->position[i++] = *field++;
    }
    node->position[i] = '\0';
    
    // Update pointer to continue parsing
    *ptr = field;
    
    return node;
}

// Helper function to append a node to the end of a linked list
static void append_to_list(Node **head, Node *new_node) {
    if (*head == NULL) {
        *head = new_node;
    } else {
        Node *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

// Helper function to print a single node
static void print_node(const Node *node) {
    if (node == NULL) return;
    
    printf("First Name: %s\n", node->first);
    printf("Second Name: %s\n", node->second);
    printf("Fingerprint: %s\n", node->fingerprint);
    printf("Position: %s\n", node->position);
    printf("\n");
}

// Helper function to free a linked list of nodes
static void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current);
        current = next;
    }
}

Org build_org_from_clean_file(const char *path) {
    Org tree;
    tree.boss = NULL;
    tree.left_hand = NULL;
    tree.right_hand = NULL;
    
    // Open file
    FILE* clean_file = fopen(path, "r");
    if (clean_file == NULL) {
        return tree;
    }
    
    // Get file size
    fseek(clean_file, 0, SEEK_END);
    long file_size = ftell(clean_file);
    fseek(clean_file, 0, SEEK_SET);
    
    // Allocate and read file
    char *org_text = (char *)malloc(file_size + 1);
    if (org_text == NULL) {
        printf("Memory allocation failed\n");
        fclose(clean_file);
        return tree;
    }
    
    fread(org_text, 1, file_size, clean_file);
    org_text[file_size] = '\0';
    fclose(clean_file);
    
    // Parse all nodes from the file
    char *ptr = org_text;
    while (*ptr != '\0') {
        // Skip to next "First Name: " occurrence
        char *next_entry = strstr(ptr, "First Name: ");
        if (next_entry == NULL) break;
        
        ptr = next_entry;
        Node *node = parse_node(&ptr);
        
        if (node == NULL) continue;
        
        // Assign node to appropriate position in tree
        if (strcmp(node->position, "Boss") == 0) {
            tree.boss = node;
        } else if (strcmp(node->position, "Right Hand") == 0) {
            tree.right_hand = node;
            if (tree.boss != NULL) {
                tree.boss->right = node;
            }
        } else if (strcmp(node->position, "Left Hand") == 0) {
            tree.left_hand = node;
            if (tree.boss != NULL) {
                tree.boss->left = node;
            }
        } else if (strcmp(node->position, "Support_Right") == 0) {
            if (tree.right_hand != NULL) {
                append_to_list(&(tree.right_hand->supports_head), node);
            }
        } else if (strcmp(node->position, "Support_Left") == 0) {
            if (tree.left_hand != NULL) {
                append_to_list(&(tree.left_hand->supports_head), node);
            }
        }
    }
    
    free(org_text);
    return tree;
}

void print_tree_order(const Org *org) {
    if (org == NULL) return;
    
    // Print Boss
    if (org->boss != NULL) {
        print_node(org->boss);
    }
    
    // Print Left Hand
    if (org->left_hand != NULL) {
        print_node(org->left_hand);
        
        // Print Left Supports
        Node *support = org->left_hand->supports_head;
        while (support != NULL) {
            print_node(support);
            support = support->next;
        }
    }
    
    // Print Right Hand
    if (org->right_hand != NULL) {
        print_node(org->right_hand);
        
        // Print Right Supports
        Node *support = org->right_hand->supports_head;
        while (support != NULL) {
            print_node(support);
            support = support->next;
        }
    }
}

void free_org(Org *org) {
    if (org == NULL) return;
    
    // Free Left Hand and its supports
    if (org->left_hand != NULL) {
        free_list(org->left_hand->supports_head);
        free(org->left_hand);
    }
    
    // Free Right Hand and its supports
    if (org->right_hand != NULL) {
        free_list(org->right_hand->supports_head);
        free(org->right_hand);
    }
    
    // Free Boss
    if (org->boss != NULL) {
        free(org->boss);
    }
}