#include "read_text.h"

KSEQ_INIT(gzFile, gzread)
char* read_text(char* filename) {
    gzFile fp;
    kseq_t *seq;
    fp = gzopen(filename, "r");
    assert(fp != NULL && "Could not open fasta file\n");
    seq = kseq_init(fp);
    int res = kseq_read(seq);
    assert(res >= 0);
    gzclose(fp);
    char* text = NULL;
    int err = asprintf(&text, "%s", seq->seq.s);
    assert(err >= 0);
    kseq_destroy(seq);
    return text;
}

//copy filename without extension -- ie. a substring up to the first '.' --
//into newfile
void file_no_extension(const char* file, char* newfile){
    char* ret = strchr(file, '.');
    //printf("len file: %lu\n",strlen(file));
    if( ret == NULL)
        memcpy(newfile, file, strlen(file));
    else{
        long n = ret - file;
        //printf("n bytes: %ld\n",n);
        memcpy(newfile, file, n);
        newfile[n]= '\0';
    }
    //printf("newfile: \"%s\"\nlen: %lu\n",newfile,strlen(newfile));
}

