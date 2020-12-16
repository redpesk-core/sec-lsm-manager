#include <CUnit/Basic.h>

#include "test_paths.h"
#include "test_policies.h"
#include "test_secure_app.h"
#include "test_utils.h"

int main() {
    if (CU_initialize_registry() != CUE_SUCCESS)
        goto error;

    // test utils
    CU_pSuite utils_suite = CU_add_suite("test_utils", NULL, NULL);
    if (utils_suite == NULL)
        goto error;

    if (CU_add_test(utils_suite, "test check file exists", test_check_file_exists) == NULL)
        goto error;

    if (CU_add_test(utils_suite, "test check file type", test_check_file_type) == NULL)
        goto error;

    if (CU_add_test(utils_suite, "test check executable", test_check_executable) == NULL)
        goto error;

    if (CU_add_test(utils_suite, "test remove", test_remove_file) == NULL)
        goto error;

    CU_pSuite paths_suite = CU_add_suite("test_paths", NULL, NULL);
    if (paths_suite == NULL)
        goto error;

    if (CU_add_test(paths_suite, "test init paths", test_init_paths) == NULL)
        goto error;

    if (CU_add_test(paths_suite, "test free paths", test_free_paths) == NULL)
        goto error;

    if (CU_add_test(paths_suite, "test paths add path", test_paths_add_path) == NULL)
        goto error;

    if (CU_add_test(paths_suite, "test valid path type", test_valid_path_type) == NULL)
        goto error;

    if (CU_add_test(paths_suite, "test get path type", test_get_path_type) == NULL)
        goto error;

    if (CU_add_test(paths_suite, "test get path type string", test_get_path_type_string) == NULL)
        goto error;

    CU_pSuite policies_suite = CU_add_suite("test_policies", NULL, NULL);
    if (paths_suite == NULL)
        goto error;

    if (CU_add_test(policies_suite, "test init policies", test_init_policies) == NULL)
        goto error;

    if (CU_add_test(policies_suite, "test free policies", test_free_policies) == NULL)
        goto error;

    if (CU_add_test(policies_suite, "test paths add path", test_policies_add_policy) == NULL)
        goto error;

    CU_pSuite secure_app_suite = CU_add_suite("test_secure_app", NULL, NULL);
    if (secure_app_suite == NULL)
        goto error;

    if (CU_add_test(secure_app_suite, "test create secure app", test_create_secure_app) == NULL)
        goto error;

    if (CU_add_test(secure_app_suite, "test secure app set id", test_secure_app_set_id) == NULL)
        goto error;

    if (CU_add_test(secure_app_suite, "test secure app add permission", test_secure_app_add_permission) == NULL)
        goto error;

    if (CU_add_test(secure_app_suite, "test secure app add path", test_secure_app_add_path) == NULL)
        goto error;

    if (CU_add_test(secure_app_suite, "test free secure app", test_free_secure_app) == NULL)
        goto error;

    if (CU_add_test(secure_app_suite, "test destroy secure app", test_destroy_secure_app) == NULL)
        goto error;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    // Number of failures
    unsigned int ret = CU_get_number_of_failures();
    goto clean;

error:
    ret = CU_get_error();
clean:
    CU_cleanup_registry();

    return (int)ret;
}
