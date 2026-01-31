#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FINGERPRINT_LEN 9
#define INITIAL_CAPACITY 10

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
    
    size_t bytes_read = fread(buffer, 1, file_size, fp);
    if (bytes_read != (size_t)file_size) {
        printf("Error reading file\n");
        free(buffer);
        return NULL;
    }
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

// Find a pattern in text, ignoring whitespace within the pattern
// Returns pointer to start of match in text
const char* find_label(const char *text, const char *pattern) {
    const char *t = text;
    
    while (*t != '\0') {
        const char *t_check = t;
        const char *p = pattern;
        int matched = 1;
        
        // Try to match pattern from this position
        while (*p != '\0') {
            // Skip whitespace in text
            while (*t_check != '\0' && (*t_check == ' ' || *t_check == '\t' || 
                   *t_check == '\n' || *t_check == '\r')) {
                t_check++;
            }
            
            if (*t_check != *p) {
                matched = 0;
                break;
            }
            
            t_check++;
            p++;
        }
        
        if (matched) {
            return t;
        }
        
        t++;
    }
    
    return NULL;
}

// Skip past a label (including any whitespace within it)
const char* skip_label(const char *text, const char *pattern) {
    const char *t = text;
    const char *p = pattern;
    
    while (*p != '\0') {
        // Skip whitespace in text
        while (*t != '\0' && (*t == ' ' || *t == '\t' || *t == '\n' || *t == '\r')) {
            t++;
        }
        
        if (*t != *p) {
            return text; // Shouldn't happen if find_label was called first
        }
        
        t++;
        p++;
    }
    
    return t;
}

// Extract value between current position and next label
// Removes newlines, keeps spaces
char* extract_value_to_label(const char *start, const char *end) {
    // Skip leading whitespace
    while (start < end && (*start == ' ' || *start == '\t' || *start == '\n' || *start == '\r')) {
        start++;
    }
    
    // Skip trailing whitespace
    while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' || 
           *(end - 1) == '\n' || *(end - 1) == '\r')) {
        end--;
    }
    
    if (end <= start) {
        char *result = (char *)malloc(1);
        if (result == NULL) {
            printf("Memory allocation failed\n");
            return NULL;
        }
        result[0] = '\0';
        return result;
    }
    
    // Count characters (excluding newlines)
    int len = 0;
    for (const char *p = start; p < end; p++) {
        if (*p != '\n' && *p != '\r') {
            len++;
        }
    }
    
    char *result = (char *)malloc(len + 1);
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    // Copy, skipping newlines
    int j = 0;
    for (const char *p = start; p < end; p++) {
        if (*p != '\n' && *p != '\r') {
            result[j++] = *p;
        }
    }
    result[j] = '\0';
    
    return result;
}

// Parse entries from cleaned text
Entry* parse_entries(const char *text, int *entry_count) {
    *entry_count = 0;
    int capacity = INITIAL_CAPACITY;
    int order_counter = 0;
    
    Entry *entries = (Entry *)malloc(capacity * sizeof(Entry));
    if (entries == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }
    
    const char *ptr = text;
    
    while (*ptr != '\0') {
        // Look for "First Name:" (ignoring whitespace)
        const char *first_name_start = find_label(ptr, "FirstName:");
        if (first_name_start == NULL) break;
        
        // Skip past the label
        ptr = skip_label(first_name_start, "FirstName:");
        
        // Find "Second Name:"
        const char *second_name_start = find_label(ptr, "SecondName:");
        if (second_name_start == NULL) break;
        
        // Extract first name
        char *first_name = extract_value_to_label(ptr, second_name_start);
        if (first_name == NULL) {
            free_entries(entries, *entry_count);
            return NULL;
        }
        
        // Skip past "Second Name:"
        ptr = skip_label(second_name_start, "SecondName:");
        
        // Find "Fingerprint:"
        const char *fingerprint_start = find_label(ptr, "Fingerprint:");
        if (fingerprint_start == NULL) {
            free(first_name);
            break;
        }
        
        // Extract second name
        char *second_name = extract_value_to_label(ptr, fingerprint_start);
        if (second_name == NULL) {
            free(first_name);
            free_entries(entries, *entry_count);
            return NULL;
        }
        
        // Skip past "Fingerprint:"
        ptr = skip_label(fingerprint_start, "Fingerprint:");
        
        // Skip whitespace before fingerprint
        while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') ptr++;
        
        // Extract fingerprint (exactly 9 alphanumeric characters)
        char fingerprint[FINGERPRINT_LEN + 1];
        int fp_len = 0;
        while (fp_len < FINGERPRINT_LEN && isalnum(*ptr)) {
            fingerprint[fp_len++] = *ptr++;
        }
        fingerprint[fp_len] = '\0';
        
        if (fp_len != FINGERPRINT_LEN) {
            free(first_name);
            free(second_name);
            continue;
        }
        
        // Find "Position:"
        const char *position_start = find_label(ptr, "Position:");
        if (position_start == NULL) {
            free(first_name);
            free(second_name);
            break;
        }
        
        // Skip past "Position:"
        ptr = skip_label(position_start, "Position:");
        
        // Find next "First Name:" or end of string
        const char *next_entry = find_label(ptr, "FirstName:");
        const char *position_end = next_entry ? next_entry : (ptr + strlen(ptr));
        
        // Extract position
        char *position = extract_value_to_label(ptr, position_end);
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
        
        // Move to next entry
        if (next_entry) {
            ptr = next_entry;
        } else {
            break;
        }
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
    
    FILE *corrupted_text = fopen(argv[1], "rb");
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
    
    FILE *clean_text = fopen(argv[2], "wb");
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