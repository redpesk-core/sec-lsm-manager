#include <check.h>
#include <fcntl.h>
#include <linux/xattr.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

void mksuite(const char *name);

void addtcase(const char *name);

void addtest(const TTest *fun);

int srun();

void create_tmp_dir(char *tmp_dir);

void create_tmp_file(char *tmp_file);

void create_etc_tmp_file(char *tmp_file);

bool compare_xattr(const char *path, const char *xattr, const char *value);