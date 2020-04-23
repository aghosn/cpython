/* Support for sandboxes */

#define DELIMITER_ENTRY ':'

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "sandbox.h"

mem_view *parse_memory_view(const char *memc) {
  // Only handles a single entry
  size_t i, len;
  char p;
  char *perm;
  mem_view *res;

  perm = strchr(memc, DELIMITER_ENTRY);
  perm = '\0';
  perm++;
  len = strlen(perm);
  p = 0;

  for (i = 0; i < len; ++i) {
    switch (perm[i]) {
      case 'X': 
        p |= X_VAL;
        break;
      case 'W':
        p |= W_VAL;
        break;
      case 'R':
        p |= R_VAL;
        break;
    }
  }

  res = malloc(sizeof(mem_view));
  res->name = memc;
  res->perm = p;

  return res;
}

int sandbox_prolog(const char *mem, const char *sys) {
    mem_view *parsed = parse_memory_view(mem);
    printf("%s\n", "call prolog");
    printf("permissions are %s:%d\n", parsed->name, parsed->perm);
    fflush(stdout);
    return 1;
}

int sandbox_epilog() {
    printf("%s\n", "call epilog");
    fflush(stdout);
    return 1;
}
