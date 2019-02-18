#include "lcp-create.h"
#include "read_text.h"

static inline int min(int a, int b) {
    return a < b ? a : b;
}

// Lcp[i] = lcp(i, i-1)
// rank is SA^-1
static void lcp_kasai(const char* text, const int* sa, unsigned int* lcp, const unsigned int n){
    //lcp[0] = 0;
    int *rank = malloc(n * sizeof(*rank));
    for(unsigned int i = 0; i < n; ++i){
        rank[sa[i]] = i;
    }  
    unsigned int k, j = 0;
    unsigned int h = 0;
    for(unsigned int i = 0; i < n; ++i){
        k = rank[i];
        if(k == 0) {
            lcp[0] = 0;
        }
        else{
            j = sa[k - 1];
            while(i + h < n && j + h <n && text[i + h] == text[j + h])
                ++h;
            lcp[k] = h;
        }
        if(h > 0)
            --h;
    }
    free(rank);
}
//in-order trasversal of binary search tree
//base case: leaf i found --> copy value from lcp(i, i-1)
//recursive case: parent --> min(child_left, child_right)
static unsigned int lcps_recursive(const unsigned int* lcp,unsigned int* llcp, 
        unsigned int* rlcp ,unsigned int l, unsigned int r){
    unsigned int c = (l + r)/2;
    if (l+1 == c)
        llcp[c] = lcp[c];
    else
        llcp[c] = lcps_recursive(lcp, llcp, rlcp, l, c);
    if (c +1 == r)
        rlcp[c] = lcp[r];
    else
        rlcp[c] = lcps_recursive(lcp, llcp, rlcp, c ,r);
    return min(llcp[c], rlcp[c]);
}
//llcp[mp] = lcp(lp, mp)
//rlcp[mp] = lcp(rp, mp)
static void lcp_lr(const unsigned int* lcp, unsigned int* llcp, 
        unsigned int* rlcp, const unsigned int n){

    lcps_recursive(lcp, llcp, rlcp, 0, n-1);
}


int main(int argc, char** argv){
    if( argc == 0){
        fprintf(stderr, "%s: No file specified.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char* text =read_text(argv[1]);
    //filename without path
    char* filename = basename(argv[1]);
    size_t e =strlen(filename) + 5;
    char no_ext[e];
    file_no_extension(filename, no_ext);
    //printf("file: \"%s\"\nlen: %lu\n",no_ext,strlen(no_ext));
    strncat(no_ext, ".bin", e -strlen(no_ext));
    //char* text = argv[1];
    uint32_t n = strlen(text);
    printf("Text has %d char/s\nlen: %lu\n",n, strlen(no_ext));
    printf("file : %s\n",no_ext);
    int* sa = malloc(n * sizeof(*sa));
    unsigned int* lcp = malloc(n * sizeof(*lcp));
    unsigned int* llcp = malloc(n * sizeof(*lcp));
    unsigned int* rlcp = malloc(n * sizeof(*lcp));
    printf("Creating Sa...\n");
    if(sais((unsigned char*) text, sa, (int)n) != 0) {
        fprintf(stderr, "%s: Cannot allocate memory.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    //sais_lcp((unsigned char*) text, sa, lcp, n);    
    printf("Done.\nCreating Lcp_1...\n");
    lcp_kasai(text, sa, lcp, n);
    printf("Done.\nCreating Lcp_lr...\n");
    lcp_lr(lcp, llcp, rlcp, n);
    printf("Done.\nWriting Sa, Lcp_lr to \"%s.bin\"\n", no_ext);
    free(lcp);
    lcp = 0;
    FILE *write_ptr;
    write_ptr = fopen(no_ext,"wb");
    fwrite(sa, sizeof(*sa),n, write_ptr);
    //fwrite(lcp, sizeof(*lcp),n, write_ptr);
    fwrite(llcp, sizeof(*llcp),n, write_ptr);
    fwrite(rlcp, sizeof(*rlcp),n, write_ptr);
    fclose(write_ptr);
    printf("File written successfully\n");
    free(sa);
    free(llcp);
    free(rlcp);
}

