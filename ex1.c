#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    char *first_name;
    char *second_name;
    char fingerprint[FINGERPRINT_LEN + 1];
    char *position;
    PositionType pos_type;
    int original_order;
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
    if (strcmp(pos, "Support_Right") == 0) return SUPPORT_RIGHT;
    if (strcmp(pos, "Support_Left") == 0) return SUPPORT_LEFT;
    return UNKNOWN;
}

// Compare function for qsort
int compare_entries(const void *a, const void *b) {
    Entry *e1 = (Entry *)a;
    Entry *e2 = (Entry *)b;
    
    if (e1->pos_type != e2->pos_type) {
        return e1->pos_type - e2->pos_type;
    }
    
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

// Free a single entry
void free_entry(Entry *entry) {
    if (entry->first_name) free(entry->first_name);
    if (entry->second_name) free(entry->second_name);
    if (entry->position) free(entry->position);
}

// Free all entries
void free_entries(Entry *entries, int count) {
    for (int i = 0; i < count; i++) {
        free_entry(&entries[i]);
    }
    free(entries);
}

// Read entire file and remove corruption, returning cleaned string
char* read_and_clean_file(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    fread(buffer, 1, file_size, fp);
    buffer[file_size] = '\0';
    
    char *cleaned = (char *)malloc(file_size + 1);
    if (cleaned == NULL) {
        printf("Memory allocation failed\n");
        free(buffer);
        return NULL;
    }
    
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

// Find a label in text, ignoring whitespace/newlines within the label itself
// Returns pointer to position after the label, or NULL if not found
const char* find_label(const char *text, const char *label) {
    for (const char *start = text; *start != '\0'; start++) {
        const char *t = start;
        const char *l = label;
        
        while (*l != '\0') {
            // Skip whitespace in BOTH text and label
            while (*t == ' ' || *t == '\t' || *t == '\n' || *t == '\r') {
                t++;
            }
            while (*l == ' ' || *l == '\t' || *l == '\n' || *l == '\r') {
                l++;
            }
            
            // Check if we've reached end of label
            if (*l == '\0') break;
            
            // Compare characters
            if (*t == *l) {
                t++;
                l++;
            } else {
                break;
            }
        }
        
        // Did we match entire label?
        if (*l == '\0') {
            // Skip trailing whitespace in text
            while (*t == ' ' || *t == '\t' || *t == '\n' || *t == '\r') {
                t++;
            }
            return t;
        }
    }
    
    return NULL;
}

// Extract value between current position and next label
// Strips leading/trailing whitespace but preserves internal structure
char* extract_value_until_label(const char *start, const char *label_to_find) {
    // We need to find where the label starts in the text
    // Search character by character and try to match the label
    const char *value_end = start;
    
    while (*value_end != '\0') {
        // Try to match label from this position
        const char *t = value_end;
        const char *l = label_to_find;
        int match = 1;
        
        while (*l != '\0') {
            // Skip whitespace in both text and label
            while (*t == ' ' || *t == '\t' || *t == '\n' || *t == '\r') {
                t++;
            }
            while (*l == ' ' || *l == '\t' || *l == '\n' || *l == '\r') {
                l++;
            }
            
            if (*l == '\0') break;
            
            if (*t == *l) {
                t++;
                l++;
            } else {
                match = 0;
                break;
            }
        }
        
        // Did we find the label?
        if (match && *l == '\0') {
            // value_end now points to start of label
            break;
        }
        
        value_end++;
    }
    
    // Now extract from start to value_end
    const char *value_start = start;
    
    // Skip leading whitespace
    while (value_start < value_end && (*value_start == ' ' || *value_start == '\t' || 
           *value_start == '\n' || *value_start == '\r')) {
        value_start++;
    }
    
    // Skip trailing whitespace (work backwards from value_end)
    const char *actual_end = value_end;
    while (actual_end > value_start && (*(actual_end - 1) == ' ' || *(actual_end - 1) == '\t' || 
           *(actual_end - 1) == '\n' || *(actual_end - 1) == '\r')) {
        actual_end--;
    }
    
    if (actual_end <= value_start) {
        char *result = (char *)malloc(1);
        if (result) result[0] = '\0';
        return result;
    }
    
    // Calculate length (excluding internal newlines/tabs)
    int len = 0;
    for (const char *p = value_start; p < actual_end; p++) {
        if (*p != '\n' && *p != '\r' && *p != '\t') {
            len++;
        }
    }
    
    char *result = (char *)malloc(len + 1);
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    // Copy value, removing newlines/tabs but keeping spaces
    int j = 0;
    for (const char *p = value_start; p < actual_end; p++) {
        if (*p != '\n' && *p != '\r' && *p != '\t') {
            result[j++] = *p;
        }
    }
    result[j] = '\0';
    
    return result;
}

// Parse entries from cleaned text
Entry* parse_entries(const char *text, int *entry_count) {
    *entry_count = 0;
    int capacity = 10;
    int order_counter = 0;
    
    Entry *entries = (Entry *)malloc(capacity * sizeof(Entry));
    if (entries == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    const char *ptr = text;
    
    while (*ptr != '\0') {
        // Find "First Name:"
        const char *after_first_label = find_label(ptr, "First Name:");
        if (after_first_label == NULL) break;
        
        // Extract first name (until "Second Name:")
        char *first_name = extract_value_until_label(after_first_label, "Second Name:");
        if (first_name == NULL) break;
        
        // Find "Second Name:"
        const char *after_second_label = find_label(after_first_label, "Second Name:");
        if (after_second_label == NULL) {
            free(first_name);
            break;
        }
        
        // Extract second name (until "Fingerprint:")
        char *second_name = extract_value_until_label(after_second_label, "Fingerprint:");
        if (second_name == NULL) {
            free(first_name);
            break;
        }
        
        // Find "Fingerprint:"
        const char *after_fingerprint_label = find_label(after_second_label, "Fingerprint:");
        if (after_fingerprint_label == NULL) {
            free(first_name);
            free(second_name);
            break;
        }
        
        // Extract fingerprint (exactly 9 alphanumeric characters)
        char fingerprint[FINGERPRINT_LEN + 1];
        int fp_len = 0;
        const char *fp_ptr = after_fingerprint_label;
        while (fp_len < FINGERPRINT_LEN && *fp_ptr != '\0') {
            if (isalnum(*fp_ptr)) {
                fingerprint[fp_len++] = *fp_ptr;
            } else if (*fp_ptr != ' ' && *fp_ptr != '\t' && *fp_ptr != '\n' && *fp_ptr != '\r') {
                // Hit a non-alphanumeric, non-whitespace character
                break;
            }
            fp_ptr++;
        }
        fingerprint[fp_len] = '\0';
        
        if (fp_len != FINGERPRINT_LEN) {
            free(first_name);
            free(second_name);
            // Move ptr forward to try next entry
            ptr = after_first_label;
            continue;
        }
        
        // Find "Position:"
        const char *after_position_label = find_label(fp_ptr, "Position:");
        if (after_position_label == NULL) {
            free(first_name);
            free(second_name);
            break;
        }
        
        // Extract position (until next "First Name:" or end of string)
        char *position;
        const char *next_entry_label = find_label(after_position_label, "First Name:");
        if (next_entry_label != NULL) {
            position = extract_value_until_label(after_position_label, "First Name:");
        } else {
            // Last entry - extract until end
            const char *pos_start = after_position_label;
            const char *pos_end = pos_start + strlen(pos_start);
            
            // Skip leading whitespace
            while (pos_start < pos_end && (*pos_start == ' ' || *pos_start == '\t' || 
                   *pos_start == '\n' || *pos_start == '\r')) {
                pos_start++;
            }
            
            // Skip trailing whitespace
            while (pos_end > pos_start && (*(pos_end - 1) == ' ' || *(pos_end - 1) == '\t' || 
                   *(pos_end - 1) == '\n' || *(pos_end - 1) == '\r')) {
                pos_end--;
            }
            
            int len = 0;
            for (const char *p = pos_start; p < pos_end; p++) {
                if (*p != '\n' && *p != '\r' && *p != '\t') {
                    len++;
                }
            }
            
            position = (char *)malloc(len + 1);
            if (position == NULL) {
                free(first_name);
                free(second_name);
                free_entries(entries, *entry_count);
                return NULL;
            }
            
            int j = 0;
            for (const char *p = pos_start; p < pos_end; p++) {
                if (*p != '\n' && *p != '\r' && *p != '\t') {
                    position[j++] = *p;
                }
            }
            position[j] = '\0';
        }
        
        if (position == NULL) {
            free(first_name);
            free(second_name);
            free_entries(entries, *entry_count);
            return NULL;
        }
        
        // Check for duplicates
        if (!is_duplicate(entries, *entry_count, fingerprint)) {
            // Expand array if needed
            if (*entry_count >= capacity) {
                capacity *= 2;
                Entry *temp = (Entry *)realloc(entries, capacity * sizeof(Entry));
                if (temp == NULL) {
                    printf("Memory allocation failed\n");
                    free(first_name);
                    free(second_name);
                    free(position);
                    free_entries(entries, *entry_count);
                    return NULL;
                }
                entries = temp;
            }
            
            // Add entry
            entries[*entry_count].first_name = first_name;
            entries[*entry_count].second_name = second_name;
            strcpy(entries[*entry_count].fingerprint, fingerprint);
            entries[*entry_count].position = position;
            entries[*entry_count].pos_type = get_position_type(position);
            entries[*entry_count].original_order = order_counter++;
            (*entry_count)++;
        } else {
            // Duplicate found, free the strings
            free(first_name);
            free(second_name);
            free(position);
        }
        
        // Move pointer forward to continue searching
        ptr = after_first_label;
    }
    
    return entries;
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
    
    char *cleaned_text = read_and_clean_file(corrupted_text);
    fclose(corrupted_text);
    
    if (cleaned_text == NULL) {
        return 0;
    }
    
    int entry_count = 0;
    Entry *entries = parse_entries(cleaned_text, &entry_count);
    
    free(cleaned_text);
    
    if (entries == NULL) {
        return 0;
    }
    
    qsort(entries, entry_count, sizeof(Entry), compare_entries);
    
    FILE *clean_text = fopen(argv[2], "w");
    if (clean_text == NULL) {
        printf("Error opening file: %s\n", argv[2]);
        free_entries(entries, entry_count);
        return 0;
    }
    
    write_output(clean_text, entries, entry_count);
    fclose(clean_text);
    
    free_entries(entries, entry_count);
    
    return 0;
}
