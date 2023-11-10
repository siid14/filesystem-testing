#include <string.h>
#include <stdio.h>
#include <stdlib.h>
char* cleanPath(char* path){

    char *tokens[100];  

    // Tokenize the string
    char *token = strtok(path, "/");
    int i = 0;

    // Loop through the tokens and store them in the array
    while (token != NULL) {
        if(strcmp(token,"..") == 0){
            if(i == 0){
                i = 0;
            }else{
                i --;
            }
        }else if (strcmp(token,".") == 0)
        {
            i = i;
        }else{
            tokens[i] = token;
            i++;
        }    
        
        token = strtok(NULL, "/");
    }

    // Print the tokens
    printf("Tokens:\n");
    for (int j = 0; j < i; j++) {
        printf("%s\n", tokens[j]);
    }
    int  totalLength = 0;

    for (int j = 0; j < i; j++) {
        totalLength += strlen(tokens[j]);
    }

    // Add space for the separator '/' and null terminator
    totalLength += i - 1;

    // Create a buffer to store the concatenated string
    char* result = (char*)malloc(totalLength + 1);

    // Concatenate the tokens with '/'
    strcpy(result, tokens[0]);
    for (int j = 1; j < i; j++) {
        strcat(result, "/");
        strcat(result, tokens[j]);
    }

    // Print the result
    printf("Concatenated string: %s\n", result);
    free(result);
    result = NULL;

    return 0;
}

int main(){
    char path[] = "/home/student/document/foo/../../bar/./";
    cleanPath(path);
    return 0;
}