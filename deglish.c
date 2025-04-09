//
// deglish.c - command line tool for getting the definition of a word
//
// Fletcher M - 09/04/2025
//

// TODO Make readme
// TODO factor out parseing the file.
// TODO get a better command line, with verbose and stuff. flag.h?
// TODO if no word is found, find the closest words, Levenshtein distance?
// TODO make a shell, readline dependency? editline?
// TODO maybe make it a dictionary for faster access? but its fast enough as is? even in debug mode.
// TODO cram the english_definitions.utf-8 into the exe, dont wan't this to break when there is no file.


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
#define END_OF_DICTIONARY "*** END OF THE PROJECT GUTENBERG EBOOK WEBSTER'S UNABRIDGED DICTIONARY ***"


#define VERBOSE False


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
    SV word;
    // its definition
    // TODO more options for this.
    SV definition;
} English_Word;

typedef struct English_Word_Array {
    English_Word *items;
    u64 count;
    u64 capacity;
} English_Word_Array;


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


bool32 line_is_start_of_new_word_definition(SV line) {
    SV syn = SV_from_C_Str("Syn.");

    // @Hack, to catch the end of dict text.
    if (line.data[0] == '*') return True;

    // if its a number
    if ('0' <= line.data[0] && line.data[0] <= '9') return False;
    // or some other junk
    if (line.data[0] == '(') return False;


    // or its contains ':'
    if (SV_contains_char(line, ':')) return False;

    // or it starts with "Syn."
    if (SV_starts_with(line, syn)) return False;

    // if its a lowercase letter.
    for (u64 i = 0; i < line.size; i++) {
        if ('a' <= line.data[i] && line.data[i] <= 'z') return False;
    }

    return True;
}


int main(int argc, const char **argv) {
    const char *program_name = argv[0];

    if (argc != 2) {
        fprintf(stderr, "USAGE: %s [WORD]\n", program_name);
        return 1;
    }

    SV users_word = SV_from_C_Str(argv[1]);
    SV_To_Upper(&users_word);


    SV file = read_entire_file(ENGLISH_DEFINITION_FILE_PATH);

    // the read_entire_file function will print an error on error.
    if (file.data == NULL) { return 1; }
    if (VERBOSE) printf("read file, size = %zu\n", file.size);


    English_Word_Array words = {0};

    // now parse the file into a list of words, no dict for now.
    Parser p = {0};
    p.file = file;

    // remove the header.
    SV start_of_dict_line = SV_from_C_Str(START_OF_DICTIONARY);
    while (!parser_eof(&p)) {
        SV line = read_line(&p);
        // printf("got line '"SV_Fmt"'\n", SV_Arg(line));

        if (SV_Eq(line, start_of_dict_line)) break;
    }

    // Now read Every word and add to list.

    SV end_of_dict_line = SV_from_C_Str(END_OF_DICTIONARY);
    SV line;

    // skip empty lines
    while (!parser_eof(&p)) {
        line = read_line(&p);
        if (line.size) break;
    }

    while (!parser_eof(&p)) {
        English_Word new_word;
        new_word.word = line;

        // now find the rest of the definition
        char *definition_start = &p.file.data[p.cursor];

        line = read_line(&p); // the pronunciation
        while (True) {
            assert(!parser_eof(&p) && "cannot be end of file.");
            line = read_line(&p);
            if (line.size == 0) break;
        }

        // skip past the definitions
        line = read_line(&p); // start of definitions
        while (!parser_eof(&p)) {
            // skip over the definition, until newline
            while (!parser_eof(&p)) {
                line = read_line(&p);
                if (line.size == 0) break;
            }

            // skip blank lines
            while(!parser_eof(&p)) {
                line = read_line(&p);
                if (line.size > 0) break;
            }

            // now this could be the start of a new word, or it could be another section
            if (line_is_start_of_new_word_definition(line)) {
                break;
            }
        }

        new_word.definition = (SV) {
            .data = definition_start,
            .size = line.data - definition_start - 3,
        };

        // printf("New Word! '"SV_Fmt"'\n", SV_Arg(new_word.word));
        // printf("["SV_Fmt"", SV_Arg(new_word.definition));
        // printf("]\n");

        da_append(&words, new_word);

        // if its the end of the dictionary, quit.
        if (SV_Eq(line, end_of_dict_line)) break;
    }

    if (VERBOSE) printf("Got %zu words!\n", words.count);

    s64 word_index = -1;
    for (u64 i = 0; i < words.count; i++) {
        English_Word eng_word = words.items[i];

        if (SV_Eq(users_word, eng_word.word)) {
            word_index = i;
            break;
        }
    }

    if (word_index == -1) {
        printf("Cannot find word '"SV_Fmt"' in dictionary.\n", SV_Arg(users_word));
    } else {
        English_Word eng_word = words.items[word_index];

        printf("Found Word '"SV_Fmt"'\n", SV_Arg(users_word));
        printf("\n");
        printf(SV_Fmt"\n", SV_Arg(users_word));
        printf(SV_Fmt, SV_Arg(eng_word.definition));
        printf("\n\n");
    }


    free(file.data);
    if (VERBOSE) printf("file free'd\n");

    return 0;
}
