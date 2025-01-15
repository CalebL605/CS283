#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
int reverse_string(char *, int);
int word_print(char *, int);
int replace_string(char *, int, char *, char *);

int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    char *buff_ptr = buff;  //pointer to the buffer
    char prev = ' ';    //initialize previous pointer to a space character
    int count = 0;      //tracks the number of characters being put in the buffer
    
    // Fill the buffer with the user string
    while (*user_str != '\0' && count < len){
        if (*user_str == ' ' || *user_str == '\t'){
            // If the previous character was not a space or tab and the next character is not a space, tab, or \0, add a space
            if (prev != ' ' && prev != '\t' && *(user_str+1) != ' ' && *(user_str+1) != '\t' && *(user_str+1) != '\0'){ 
                *buff_ptr++ = ' ';
                count++;
                prev = ' ';
            }
        } else {
            *buff_ptr++ = *user_str;
            count++;
            prev = *user_str;
        }
        user_str++;
    }

    // If the user string is not empty, then the user string is too long
    if (*user_str != '\0'){
        return -1;
    }

    int index = count;

    // Fill the rest of the buffer with periods
    while (index < len){
        *buff_ptr++ = '.';
        index++;
    }

    // Return the number of characters in the buffer
    return count;
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    if (str_len > len) {
        // Error: str_len exceeds the allocated buffer size 50
        return -2; // Return an error code
    }

    int word_count = 0;     //tracks the number of words in the buffer
    int in_word = 0;        //tracks if the current character is in a word

    // Count the number of words in the buffer
    for (int i=0; i<str_len; i++) {
        if (*(buff+i) == ' '){
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            word_count++;
        }
    }

    return word_count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int reverse_string(char *buff, int str_len){
    char reversed_string[str_len];  //holds the reversed string

    // Reverse the string
    for(int i=0; i<str_len; i++){
        *(reversed_string+i) = *(buff+str_len-i-1);
    }

    printf("Reversed String: %s\n", reversed_string);
    return 0;
}

int word_print(char *buff, int str_len){
    printf("Word Print\n----------\n");

    int word_start = 0;    //index of the start of the current word
    int word_length = 0;   //length of the current word
    int word_count = 0;    //tracks the number of words in the buffer

    // Print the words in the buffer
    for (int i=0; i<=str_len; i++) {
        if (*(buff+i) == ' ' || i == str_len) {
            if (word_length > 0) {
                word_count++;
                printf("%d. ", word_count);
                for (int j = word_start; j<word_start+word_length; j++) {
                    putchar(*(buff+j));
                }
                printf(" (%d)\n", word_length);
                word_length = 0;
            }
        } else {
            if (word_length == 0) {
                word_start = i;
            }
            word_length++;
        }
    }

    return 0;
}

int replace_string(char *buff, int str_len, char *search, char *replace){
    int search_len = 0;    //length of the search string
    int replace_len = 0;   //length of the replace string

    // Find the length of the search and replace strings
    while (*(search+search_len) != '\0') {
        search_len++;
    }
    while (*(replace+replace_len) != '\0') {
        replace_len++;
    }

    // If the search string is empty, return an error
    if (search_len == 0){
        return -1;
    }

    // If the replace string is empty, return an error
    if (replace_len == 0){
        return -2;
    }

    // If the search string is longer than the user string, return an error
    if (search_len > str_len){
        return -3;
    }

    // If the replace string is longer than the user string, return an error
    if (replace_len > str_len){
        return -4;
    }

    // Find the first occurrence of the search string
    char *match_start = NULL;
    for (int i = 0; i <= str_len-search_len; i++) {
        int match = 1;
        for (int j = 0; j < search_len; j++) {
            if (*(buff+i+j) != *(search+j)) {
                match = 0;
                break;
            }
        }
        if (match) {
            match_start = buff+i;
            break;
        }
    }

    // If the search string is not found, return an error
    if (!match_start) {
        return -5;
    }

    // If the new length of the string exceeds the buffer size, return an error
    if (str_len-search_len+replace_len > BUFFER_SZ) {
        return -6;
    }

    char new[BUFFER_SZ];   //store the new modified string
    char *new_ptr = new;  //pointer to the new string

    // Copy the buffer into the new string with the search string replaced
    for (char *ptr = buff; ptr < match_start; ptr++) {
        *new_ptr++ = *ptr;
    }
    for (char *ptr = replace; *ptr != '\0'; ptr++) {
        *new_ptr++ = *ptr;
    }
    for (char *ptr = match_start+search_len; ptr < buff+str_len; ptr++) {
        *new_ptr++ = *ptr;
    }

    // Null terminate the new string
    *new_ptr = '\0';

    printf("Modified String: %s\n", new);
    return 0;
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if argv[1] does not exist?
    /* 
        This is safe because the left-hand expression of the OR statement checks to 
        see if there are at least two arguments. If argv[1] does not exist then the 
        program will execute the usage function and exit because it satisfies the 
        left-hand expression of the OR statement.
    */
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    /*
        This if statement checks to see if there are less than 3 arguments. If there are 
        less than 3 arguments, the program will execute the usage function and exit.
    */
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }
    
    input_string = argv[2]; //capture the user input string
    
    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = (char *)malloc(BUFFER_SZ);
    if (buff == NULL){
        printf("Error allocating buffer, error = %d\n", 99);
        exit(99);
    }

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d\n", user_str_len);
        free(buff);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            rc = reverse_string(buff, user_str_len);
            if (rc < 0){
                printf("Error reversing string, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            break;

        case 'w':
            rc = word_print(buff, user_str_len);
            if (rc < 0){
                printf("Error printing words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            break;

        case 'x':
            // If there are not enough arguments, print the usage and exit
            if (argc < 5) {
                printf("usage: %s -x \"string\" [search arg] [replace arg]\n", argv[0]);
                free(buff);
                exit(1);
            }

            rc = replace_string(buff, user_str_len, argv[3], argv[4]);
            if (rc < 0){
                printf("Error searching and replacing words, rc = %d\n", rc);
                free(buff);
                exit(2);
            }
            break;

        default:
            usage(argv[0]);
            free(buff);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          This is a good practice because it allows the helper functions to be more
//          flexible and reusable. If the helper functions only took the buffer pointer,
//          then they would be limited to only working with buffers of size 50 if the 
//          function needs to utilize the length of the buffer. It also helps with error
//          checking because the function can check if the length of the buffer is less
//          than the length of the string that is being passed in. 