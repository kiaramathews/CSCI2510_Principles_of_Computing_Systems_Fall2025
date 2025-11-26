//This program brute-forces a given password hash by trying all possible
//passwords of a given length.
//
//Usage:
//crack <threads> <keysize> <target>
//
//Where <threads> is the number of threads to use, <keysize> is the maximum
//password length to search, and <target> is the target password hash.
//
//For example:
//
//./crack 1 5 na3C5487Wz4zw
//
//Should return the password 'apple'

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>


char *crypt(const char *key, const char *salt);

// Global variables
char* target_hash;
char salt[3];
int current_length;
int num_threads;
int found = 0;
char found_password[9];
pthread_mutex_t found_mutex = PTHREAD_MUTEX_INITIALIZER;

// thread argument
typedef struct {
    int thread_id;
    int total_threads;
    int length;
} thread_arg_t;

// a function that increments a password string
int increment_password(char* password, int length) {
    for (int i = length - 1; i >= 0; i--) {
        if (password[i] < 'z') {
            password[i]++;
            return 1;
        }
        password[i] = 'a';
    }
    return 0; // Overflow
}

// the thread worker
void* crack_passwords(void* arg) {
    thread_arg_t* args = (thread_arg_t*)arg;
    int thread_id = args->thread_id;
    int total_threads = args->total_threads;
    int length = args->length;
    
    // password buffer
    char password[9];
    memset(password, 0, sizeof(password));
    
    // total number of passwords of the length
    long total_passwords = 1;
    for (int i = 0; i < length; i++) {
        total_passwords *= 26;
    }
    
    
    long passwords_per_thread = total_passwords / total_threads;
    long start_index = thread_id * passwords_per_thread;
    long end_index = (thread_id == total_threads - 1) ? total_passwords : (thread_id + 1) * passwords_per_thread;
    
    memset(password, 'a', length);
    
    // converts start_index to the actual password
    long temp = start_index;
    for (int i = length - 1; i >= 0; i--) {
        password[i] = 'a' + (temp % 26);
        temp /= 26;
    }
    
    for (long count = start_index; count < end_index; count++) {
        // Checks if thread found it
        pthread_mutex_lock(&found_mutex);
        if (found) {
            pthread_mutex_unlock(&found_mutex);
            break;
        }
        pthread_mutex_unlock(&found_mutex);
        
        // hashs current password
        char* hash = crypt(password, salt);
        
        //checks if it matches
        if (strcmp(hash, target_hash) == 0) {
            pthread_mutex_lock(&found_mutex);
            if (!found) {
                found = 1;
                strcpy(found_password, password);
            }
            pthread_mutex_unlock(&found_mutex);
            break;
        }
        
        // next password
        increment_password(password, length);
    }
    
    return NULL;
}

// cracks passwords in a specific length
void crack_length(int length, int threads) {
    pthread_t thread_ids[threads];
    thread_arg_t thread_args[threads];
    
    for (int i = 0; i < threads; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].total_threads = threads;
        thread_args[i].length = length;
        pthread_create(&thread_ids[i], NULL, crack_passwords, &thread_args[i]);
    }
    
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <threads> <keysize> <target>\n", argv[0]);
        return 1;
    }
    
    num_threads = atoi(argv[1]);
    int keysize = atoi(argv[2]);
    target_hash = argv[3];
    
    if (num_threads < 1) {
        fprintf(stderr, "Number of threads must be at least 1\n");
        return 1;
    }
    
    if (keysize < 1 || keysize > 8) {
        fprintf(stderr, "Keysize must be between 1 and 8\n");
        return 1;
    }
    
    if (strlen(target_hash) < 2) {
        fprintf(stderr, "Invalid target hash\n");
        return 1;
    }
    
    // extracts first 2 characters of salt
    salt[0] = target_hash[0];
    salt[1] = target_hash[1];
    salt[2] = '\0';
    
    for (int len = 1; len <= keysize; len++) {
        crack_length(len, num_threads);
        
        if (found) {
            printf("%s\n", found_password);
            return 0;
        }
    }
    
    return 0;
}