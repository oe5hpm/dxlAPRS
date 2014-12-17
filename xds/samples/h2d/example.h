/* example.h */
#define FALSE 0
#define TRUE  1

#define equal  10
#define more   20
#define less   30

typedef int BOOL;

typedef int INTEGER;

struct tree{
  INTEGER key;        /* key           */
  struct tree *right; /* right subtree */
  struct tree *left;  /* left  subtree */
};

#define compare(arg1, arg2) ((arg1.key == arg2.key)?(equal):\
                            ((arg1.key>arg2.key)?(more):(less)))

void insert(struct tree *head, INTEGER key);
BOOL find(struct tree *head, INTEGER key);
