#ifndef Py_SANDBOX_H
#define Py_SANDBOX_H
#ifdef __cplusplus
extern "C" {
#endif

/* Support for sandboxes */
// TODO essayer de faire que ça marche en comprenant comment le Makefile est généré 
// parce que c'est quand même plus propre si c'est dans un fichier séparé

/* Actually useless */
#define X_VAL 1
#define W_VAL (1 << 1)
#define R_VAL (1 << 2)

typedef struct {
  const char *name; /* Name of package */
  char perm;        /* Permissions encoded*/
} mem_view;

mem_view *parse_memory_view(const char *memc);

int sandbox_prolog(const char *mem, const char *sys); // TODO il y aura sûrement un id aussi
int sandbox_epilog(); // TODO aura aussi des arguments mais plus tard

#ifdef __cplusplus
}
#endif
#endif /* !Py_SANDBOX_H */
