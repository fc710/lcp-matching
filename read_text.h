#ifndef READ_TEXT_H
#define READ_TEXT_H
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <string.h>
#include "klib/kseq.h"
#include <assert.h>
#include <zlib.h>
char* read_text(char* filename);
void file_no_extension(const char* file, char* newfile);
#endif
