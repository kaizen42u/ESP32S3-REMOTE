#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct nlist { /* table entry: */
    struct nlist *next; /* next entry in chain */
    char *name; /* defined name */
    char *defn; /* replacement text */
};

#define HASHSIZE 101

/* hash: form hash value for string s */
// unsigned dictionary_hash(char *s);

/* lookup: look for s in hashtab */
struct nlist *dictionary_lookup(char *s);

/* install: put (name, defn) in hashtab */
struct nlist *dictionary_install(char *name, char *defn);

/* make a duplicate of s */
// char *dictionary_strdup(char *s);

void add_to_dictionary(size_t id, char *name);
char *get_from_dictionary(size_t id);

#define SET_DICTIONARY_BY_NAME(x) add_to_dictionary(x, #x);