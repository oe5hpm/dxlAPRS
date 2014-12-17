#include <linux/unistd.h>


int symblink(char *existing, char *newname)
{
return symlink(existing, newname);
}
