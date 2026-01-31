#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "org_tree.h"

// Helper function to safely copy string with bounds checking
static void safe_copy(char *dest, const char *src, int max_len) {
    int i = 0;
    while (src[i] != '\0' && src[i] != '\n' && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Helper function to parse a single node from text starting at ptr
static Node* parse_node(char **ptr) {
    Node *node = (Node*)malloc(sizeof(Node));
    if (node == NULL) {
        printf("Memory allocation failed\n");
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
    safe_copy(node->first, field, MAX_FIELD);
    
    // Find and extract Second Name
    field = strstr(field, "Second Name: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("Second Name: ");
    safe_copy(node->second, field, MAX_FIELD);
    
    // Find and extract Fingerprint
    field = strstr(field, "Fingerprint: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("Fingerprint: ");
    safe_copy(node->fingerprint, field, MAX_FIELD);
    
    // Find and extract Position
    field = strstr(field, "Position: ");
    if (field == NULL) {
        free(node);
        return NULL;
    }
    field += strlen("Position: ");
    safe_copy(node->position, field, MAX_POS);
    
    return node;
}

// Helper function to append a node to the end of a linked list
static void append_to_list(Node **head, Node *new_node) {
    new_node->next = NULL;  // Ensure next is NULL
    
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
    
    size_t bytes_read = fread(org_text, 1, file_size, clean_file);
    fclose(clean_file);
    
    if (bytes_read != (size_t)file_size) {
        printf("Error reading file\n");
        free(org_text);
        return tree;
    }
    
    org_text[file_size] = '\0';
    
    // Parse all nodes from the file
    char *ptr = org_text;
    while (*ptr != '\0') {
        // Skip to next "First Name: " occurrence
        char *next_entry = strstr(ptr, "First Name: ");
        if (next_entry == NULL) break;
        
        ptr = next_entry;
        Node *node = parse_node(&ptr);
        
        if (node == NULL) {
            // Skip to next line and continue
            while (*ptr != '\0' && *ptr != '\n') ptr++;
            if (*ptr == '\n') ptr++;
            continue;
        }
        
        // Assign node to appropriate position in tree
        if (strcmp(node->position, "Boss") == 0) {
            if (tree.boss == NULL) {  // Only set if not already set
                tree.boss = node;
            } else {
                free(node);  // Duplicate boss, free it
            }
        } else if (strcmp(node->position, "Right Hand") == 0) {
            if (tree.right_hand == NULL) {  // Only set if not already set
                tree.right_hand = node;
                if (tree.boss != NULL) {
                    tree.boss->right = node;
                }
            } else {
                free(node);  // Duplicate right hand, free it
            }
        } else if (strcmp(node->position, "Left Hand") == 0) {
            if (tree.left_hand == NULL) {  // Only set if not already set
                tree.left_hand = node;
                if (tree.boss != NULL) {
                    tree.boss->left = node;
                }
            } else {
                free(node);  // Duplicate left hand, free it
            }
        } else if (strcmp(node->position, "Support Right") == 0) {
            if (tree.right_hand != NULL) {
                append_to_list(&(tree.right_hand->supports_head), node);
            } else {
                free(node);  // No right hand to attach to
            }
        } else if (strcmp(node->position, "Support Left") == 0) {
            if (tree.left_hand != NULL) {
                append_to_list(&(tree.left_hand->supports_head), node);
            } else {
                free(node);  // No left hand to attach to
            }
        } else {
            // Unknown position
            free(node);
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
        org->left_hand = NULL;
    }
    
    // Free Right Hand and its supports
    if (org->right_hand != NULL) {
        free_list(org->right_hand->supports_head);
        free(org->right_hand);
        org->right_hand = NULL;
    }
    
    // Free Boss
    if (org->boss != NULL) {
        free(org->boss);
        org->boss = NULL;
    }
}