#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "org_tree.h"
#define FP_LEN 9


static void print_success(int mask, char *op, char* fingerprint, char* First_Name, char* Second_Name)
{
    printf("Successful Decrypt! The Mask used was mask_%d of type (%s) and The fingerprint was %.*s belonging to %s %s\n",
                       mask, op, FP_LEN, fingerprint, First_Name, Second_Name);
}

static void print_unsuccess()
{
    printf("Unsuccesful decrypt, Looks like he got away\n");
}

// Helper function to check if a node's fingerprint encrypted with mask matches cipher
static int check_encryption_match(const Node *node, unsigned char *cipher, int mask, int use_xor) {
    if (node == NULL) return 0;
    
    for (int i = 0; i < FP_LEN; i++) {
        unsigned char encrypted;
        if (use_xor) {
            encrypted = (unsigned char)node->fingerprint[i] ^ (unsigned char)mask;
        } else {
            encrypted = (unsigned char)node->fingerprint[i] & (unsigned char)mask;
        }
        
        if (encrypted != cipher[i]) {
            return 0;
        }
    }
    return 1;
}

// Helper function to find a node whose fingerprint encrypts to the cipher
static Node* try_decrypt(const Org *org, unsigned char *cipher, int mask, int use_xor) {
    // Check Boss
    if (check_encryption_match(org->boss, cipher, mask, use_xor)) {
        return org->boss;
    }
    
    // Check Left Hand
    if (check_encryption_match(org->left_hand, cipher, mask, use_xor)) {
        return org->left_hand;
    }
    
    // Check Left Supports
    if (org->left_hand != NULL) {
        Node *support = org->left_hand->supports_head;
        while (support != NULL) {
            if (check_encryption_match(support, cipher, mask, use_xor)) {
                return support;
            }
            support = support->next;
        }
    }
    
    // Check Right Hand
    if (check_encryption_match(org->right_hand, cipher, mask, use_xor)) {
        return org->right_hand;
    }
    
    // Check Right Supports
    if (org->right_hand != NULL) {
        Node *support = org->right_hand->supports_head;
        while (support != NULL) {
            if (check_encryption_match(support, cipher, mask, use_xor)) {
                return support;
            }
            support = support->next;
        }
    }
    
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <clean_file.txt> <cipher_bits.txt> <mask_start_s>\n", argv[0]);
        return 0;
    }
    
    // Read the input files
    const char *clean_file_path = argv[1];
    const char *cipher_file_path = argv[2];
    int mask_start = atoi(argv[3]);
    
    // Build the organization
    Org org = build_org_from_clean_file(clean_file_path);
    if (org.boss == NULL) {
        printf("Error opening file: %s\n", clean_file_path);
        return 0;
    }
    
    // Read cipher bits file
    FILE *cipher_file = fopen(cipher_file_path, "r");
    if (cipher_file == NULL) {
        printf("Error opening file: %s\n", cipher_file_path);
        free_org(&org);
        return 0;
    }
    
    // Read 9 lines of 8-bit binary numbers
    unsigned char cipher[FP_LEN];
    char line[16];
    for (int i = 0; i < FP_LEN; i++) {
        if (fgets(line, sizeof(line), cipher_file) == NULL) {
            printf("Error reading cipher file\n");
            fclose(cipher_file);
            free_org(&org);
            return 0;
        }
        
        // Convert binary string to byte
        cipher[i] = 0;
        for (int j = 0; j < 8 && line[j] != '\0' && line[j] != '\n'; j++) {
            cipher[i] = (cipher[i] << 1) | (line[j] - '0');
        }
    }
    fclose(cipher_file);
    
    // Attempt to decrypt the file
    int found = 0;
    Node *match = NULL;
    int successful_mask = 0;
    char *operation = NULL;
    
    // Try masks from mask_start to mask_start + 10
    for (int mask = mask_start; mask <= mask_start + 10 && !found; mask++) {
        // Try XOR
        match = try_decrypt(&org, cipher, mask, 1);
        if (match != NULL) {
            found = 1;
            successful_mask = mask;
            operation = "XOR";
            break;
        }
        
        // Try AND
        match = try_decrypt(&org, cipher, mask, 0);
        if (match != NULL) {
            found = 1;
            successful_mask = mask;
            operation = "AND";
            break;
        }
    }
    
    // Print result
    if (found) {
        print_success(successful_mask, operation, match->fingerprint, 
                     match->first, match->second);
    } else {
        print_unsuccess();
    }
    
    // Free any memory you may have allocated
    free_org(&org);
    
    return 0;
}
