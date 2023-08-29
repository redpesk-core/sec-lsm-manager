
#include <check.h>
#include <stdbool.h>

/*
#include <fcntl.h>
#include <linux/xattr.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>
*/

extern void mksuite(const char *name);

extern void addtcase(const char *name);

extern void addtest(const TTest *fun);

extern int srun(const char *log_file);

#define TMP_MIN_LENGTH 20
extern void create_tmp_dir(char tmp_dir[TMP_MIN_LENGTH]);

extern void create_tmp_file(char tmp_file[TMP_MIN_LENGTH]);

extern void create_etc_tmp_file(char tmp_file[TMP_MIN_LENGTH]);

extern bool compare_xattr(const char *path, const char *xattr, const char *value);
extern void test_paths(void);
extern void test_plugs(void);
extern void test_permissions(void);
extern void test_secure_app(void);
extern void test_utils(void);

#if !SIMULATE_CYNAGORA
extern void test_cynagora(void);
#endif

#if WITH_SMACK
extern void test_smack(void);
extern void test_smack_label(void);
#endif

#if WITH_SELINUX
extern void test_selinux_template(void);
extern void test_selinux(void);
#endif
