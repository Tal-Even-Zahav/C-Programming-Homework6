#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//TODO create functions that you can use to clean up the file

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }
    // TODO: implement
    FILE *corrupted_text = fopen(argv[1], "r");
    FILE *clean_text = fopen(argv[2],"w");
       
    if  (corrupted_text == NULL)
    {
        printf("Error opening file:  %s\n",argv[1]);
        return 0;
    }
    if  (clean_text == NULL)
    {
        printf("Error opening file:  %s\n",argv[1]);
        return 0;
    }
    int c;
    while ((c = fgetc(corrupted_text))!= EOF)
        {
            if (!(c == '#' || c == '?' || c == '!' || c == '@' || c == '&' || c == '$' )) 
            {    
                fputc(c, clean_text);         
            }
        }
    fclose(corrupted_text);
    fclose(clean_text);
    return 0;
}
