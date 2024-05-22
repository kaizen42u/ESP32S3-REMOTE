
#include "dictionary.h"

static struct nlist *hashtab[HASHSIZE]; /* pointer table */

/* hash: form hash value for string s */
unsigned dictionary_hash(char *s)
{
    unsigned hashval;
    for (hashval = 0; *s != '\0'; s++)
      hashval = *s + 31 * hashval;
    return hashval % HASHSIZE;
}

char *dictionary_strdup(char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
       strcpy(p, s);
    return p;
}

/* lookup: look for s in hashtab */
struct nlist *dictionary_lookup(char *s)
{
    struct nlist *np;
    for (np = hashtab[dictionary_hash(s)]; np != NULL; np = np->next)
        if (strcmp(s, np->name) == 0)
          return np; /* found */
    return NULL; /* not found */
}

/* install: put (name, defn) in hashtab */
struct nlist *dictionary_install(char *name, char *defn)
{
    struct nlist *np;
    unsigned hashval;
    if ((np = dictionary_lookup(name)) == NULL) { /* not found */
        np = (struct nlist *) malloc(sizeof(*np));
        if (np == NULL || (np->name = dictionary_strdup(name)) == NULL)
          return NULL;
        hashval = dictionary_hash(name);
        np->next = hashtab[hashval];
        hashtab[hashval] = np;
    } else /* already there */
        free((void *) np->defn); /*free previous defn */
    if ((np->defn = dictionary_strdup(defn)) == NULL)
       return NULL;
    return np;
}

//

void add_to_dictionary(size_t id, char *name)
{
    static char buffer[64] = {'\0'};
    snprintf(buffer, 64, "id_%d", id);
    dictionary_install(buffer, name);
}

char *get_from_dictionary(size_t id)
{
    static char buffer[64] = {'\0'};
    snprintf(buffer, 64, "id_%d", id);
    struct nlist *node = dictionary_lookup(buffer);
    if (!node) return "";
    return node->defn;
}