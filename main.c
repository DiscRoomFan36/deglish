#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include "ints.h"
#include "dynamic_array.h"

#define STRING_VIEW_IMPLEMENTATION
#include "String_View.h"


#define ENGLISH_DEFINITION_FILE_PATH "./english_definitions.utf-8"
#define START_OF_DICTIONARY "*** START OF THE PROJECT GUTENBERG EBOOK WEBSTER'S UNABRIDGED DICTIONARY ***"

typedef struct Parser {
    SV file;
    u64 cursor;
    u64 line_number;
} Parser;

bool32 parser_eof(Parser *p) {
    if (p->cursor < p->file.size) return False;
    return True;
}

SV read_line(Parser *p) {
    SV result = {0};

    assert(!parser_eof(p));

    result.data = p->file.data + p->cursor;

    while (p->file.data[p->cursor] != '\n' && p->cursor < p->file.size) {
        p->cursor += 1;
    }

    // -1 to remove the newline
    result.size = p->file.data + p->cursor - result.data - 1;
    assert(result.size >= 0 && result.size < 100);

    // skip past the new line for the next one.
    p->cursor += 1;
    p->line_number += 1;

    return result;
}

typedef struct English_Word {
    // the word
    SV Word;
    // its definition
    // TODO more options for this.
    SV definition;
} English_Word;


// from file.h
SV read_entire_file(char *filename) {
    FILE *file = fopen(filename, "rb");
    SV result = {0};

    if (!file) {
        fprintf(stderr, "ERROR when opening the file: %s\n", strerror(errno));
        return result;
    }

    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    result.data = malloc(result.size * sizeof(char));
    assert(result.data != NULL);

    u64 read_bytes = fread(result.data, sizeof(char), result.size, file);
    assert(read_bytes == result.size);

    if (fclose(file)) {} // error, we dont care

    return result;
}


// TODO accept arguments for the word to find a definition of.
int main(void) {
    SV file = read_entire_file(ENGLISH_DEFINITION_FILE_PATH);

    // the read_entire_file function will print an error on error.
    if (file.data == NULL) { return 1; }
    printf("read file, size = %zu\n", file.size);


    // now parse the file into a list of words, no dict for now.
    Parser p = {0};
    p.file = file;

    // remove the header.
    SV start_of_dict_line = SV_From_C_Str(START_OF_DICTIONARY);
    while (!parser_eof(&p)) {
        SV line = read_line(&p);
        printf("got line '"SV_Fmt"'\n", SV_Arg(line));

        if (SV_Eq(line, start_of_dict_line)) break;
    }

    // Now read Every word and add to list.


    free(file.data);
    printf("file free'd\n");

    return 0;
}
