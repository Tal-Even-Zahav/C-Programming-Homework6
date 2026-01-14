#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ENTRIES 100// maybe should allocate memory instead, length not defined
#define MAX_NAME_LEN 100
#define FINGERPRINT_LEN 9

// Position hierarchy for sorting
typedef enum {
    BOSS = 0,
    RIGHT_HAND = 1,
    LEFT_HAND = 2,
    SUPPORT_RIGHT = 3,
    SUPPORT_LEFT = 4,
    UNKNOWN = 5
} PositionType;

// Entry structure
typedef struct {
    char first_name[MAX_NAME_LEN];
    char second_name[MAX_NAME_LEN];
    char fingerprint[FINGERPRINT_LEN + 1];
    char position[MAX_NAME_LEN];
    PositionType pos_type;
    int original_order;  // For preserving order within Support positions
} Entry;

//TODO create functions that you can use to clean up the file

// Remove corruption characters from a character
int is_corruption(char c) {
    return (c == '#' || c == '?' || c == '!' || c == '@' || c == '&' || c == '$');
}

// Get position type from string
PositionType get_position_type(const char *pos) {
    if (strcmp(pos, "Boss") == 0) return BOSS;
    if (strcmp(pos, "Right Hand") == 0) return RIGHT_HAND;
    if (strcmp(pos, "Left Hand") == 0) return LEFT_HAND;
    if (strcmp(pos, "Support Right") == 0) return SUPPORT_RIGHT;
    if (strcmp(pos, "Support Left") == 0) return SUPPORT_LEFT;
    return UNKNOWN;
}

// Compare function for qsort
int compare_entries(const void *a, const void *b) {
    Entry *e1 = (Entry *)a;
    Entry *e2 = (Entry *)b;
    
    // First sort by position type
    if (e1->pos_type != e2->pos_type) {
        return e1->pos_type - e2->pos_type;
    }
    
    // If same position type, maintain original order (for Support positions)
    return e1->original_order - e2->original_order;
}

// Check if fingerprint already exists
int is_duplicate(Entry *entries, int count, const char *fingerprint) {
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].fingerprint, fingerprint) == 0) {
            return 1;
        }
    }
    return 0;
}

// Read entire file and remove corruption, returning cleaned string
char* read_and_clean_file(FILE *fp) {
    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Allocate buffer for original file
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    // Read entire file
    fread(buffer, 1, file_size, fp);
    buffer[file_size] = '\0';
    
    // Allocate buffer for cleaned text (same size is enough)
    char *cleaned = (char *)malloc(file_size + 1);
    if (cleaned == NULL) {
        printf("Memory allocation failed\n");
        free(buffer);
        return NULL;
    }
    
    // Remove corruption characters
    int j = 0;
    for (long i = 0; i < file_size; i++) {
        if (!is_corruption(buffer[i])) {
            cleaned[j++] = buffer[i];
        }
    }
    cleaned[j] = '\0';
    
    free(buffer);
    return cleaned;
}

// Parse entries from cleaned text
int parse_entries(const char *text, Entry *entries, int *entry_count) {
    const char *ptr = text;
    *entry_count = 0;
    int order_counter = 0;
    
    while (*ptr != '\0' && *entry_count < MAX_ENTRIES) {
        // Look for "First Name:"
        const char *first_name_label = strstr(ptr, "First Name:");
        if (first_name_label == NULL) break;
        
        ptr = first_name_label + strlen("First Name:");
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        // Extract first name (until we hit "Second Name:")
        const char *second_name_label = strstr(ptr, "Second Name:");
        if (second_name_label == NULL) break;
        
        // Copy first name
        int name_len = 0;
        const char *temp = ptr;
        while (temp < second_name_label && name_len < MAX_NAME_LEN - 1) {
            if (*temp != '\n' && *temp != '\r' && *temp != ' ' && *temp != '\t') {
                break;
            }
            temp++;
        }
        
        // Extract actual first name
        while (temp < second_name_label && name_len < MAX_NAME_LEN - 1) {
            if (*temp == '\n' || *temp == '\r') {
                break;
            }
            entries[*entry_count].first_name[name_len++] = *temp;
            temp++;
        }
        // Trim trailing spaces
        while (name_len > 0 && entries[*entry_count].first_name[name_len - 1] == ' ') {
            name_len--;
        }
        entries[*entry_count].first_name[name_len] = '\0';
        
        ptr = second_name_label + strlen("Second Name:");
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        // Extract second name (until we hit "Fingerprint:")
        const char *fingerprint_label = strstr(ptr, "Fingerprint:");
        if (fingerprint_label == NULL) break;
        
        // Copy second name
        name_len = 0;
        temp = ptr;
        while (temp < fingerprint_label && name_len < MAX_NAME_LEN - 1) {
            if (*temp != '\n' && *temp != '\r' && *temp != ' ' && *temp != '\t') {
                break;
            }
            temp++;
        }
        
        while (temp < fingerprint_label && name_len < MAX_NAME_LEN - 1) {
            if (*temp == '\n' || *temp == '\r') {
                break;
            }
            entries[*entry_count].second_name[name_len++] = *temp;
            temp++;
        }
        // Trim trailing spaces
        while (name_len > 0 && entries[*entry_count].second_name[name_len - 1] == ' ') {
            name_len--;
        }
        entries[*entry_count].second_name[name_len] = '\0';
        
        ptr = fingerprint_label + strlen("Fingerprint:");
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        // Extract fingerprint (exactly 9 characters)
        int fp_len = 0;
        while (fp_len < FINGERPRINT_LEN && (isalnum(*ptr))) {
            entries[*entry_count].fingerprint[fp_len++] = *ptr++;
        }
        entries[*entry_count].fingerprint[fp_len] = '\0';
        
        if (fp_len != FINGERPRINT_LEN) {
            // Invalid fingerprint, skip this entry
            continue;
        }
        
        // Look for "Position:"
        const char *position_label = strstr(ptr, "Position:");
        if (position_label == NULL) break;
        
        ptr = position_label + strlen("Position:");
        
        // Skip whitespace
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        // Extract position (until newline or next "First Name:")
        int pos_len = 0;
        while (*ptr != '\0' && *ptr != '\n' && *ptr != '\r' && pos_len < MAX_NAME_LEN - 1) {
            entries[*entry_count].position[pos_len++] = *ptr++;
        }
        // Trim trailing spaces
        while (pos_len > 0 && entries[*entry_count].position[pos_len - 1] == ' ') {
            pos_len--;
        }
        entries[*entry_count].position[pos_len] = '\0';
        
        // Check for duplicates
        if (!is_duplicate(entries, *entry_count, entries[*entry_count].fingerprint)) {
            entries[*entry_count].pos_type = get_position_type(entries[*entry_count].position);
            entries[*entry_count].original_order = order_counter++;
            (*entry_count)++;
        }
    }
    
    return 1;
}

// Write entries to output file
void write_output(FILE *fp, Entry *entries, int count) {
    for (int i = 0; i < count; i++) {
        fprintf(fp, "First Name: %s\n", entries[i].first_name);
        fprintf(fp, "Second Name: %s\n", entries[i].second_name);
        fprintf(fp, "Fingerprint: %s\n", entries[i].fingerprint);
        fprintf(fp, "Position: %s\n", entries[i].position);
        fprintf(fp, "\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }
    // TODO: implement
    
    FILE *corrupted_text = fopen(argv[1], "r");
    if (corrupted_text == NULL) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }
    
    // Read and clean the file
    char *cleaned_text = read_and_clean_file(corrupted_text);
    fclose(corrupted_text);
    
    if (cleaned_text == NULL) {
        return 0;
    }
    
    // Parse entries
    Entry entries[MAX_ENTRIES];
    int entry_count = 0;
    
    if (!parse_entries(cleaned_text, entries, &entry_count)) {
        free(cleaned_text);
        return 0;
    }
    
    free(cleaned_text);
    
    // Sort entries by position hierarchy
    qsort(entries, entry_count, sizeof(Entry), compare_entries);
    
    // Write to output file
    FILE *clean_text = fopen(argv[2], "w");
    if (clean_text == NULL) {
        printf("Error opening file: %s\n", argv[2]);
        return 0;
    }
    
    write_output(clean_text, entries, entry_count);
    fclose(clean_text);
    
    return 0;
}
