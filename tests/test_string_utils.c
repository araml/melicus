#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string_utils.h>

#define MELICUS_UNUSED __attribute__((unused))

static void test_find_in_string(void MELICUS_UNUSED **state) {
    char long_string[] = "Hello World hello world";
    char to_find[] = "hello world";
    assert_true(find_in_string(long_string, to_find) == 12);
}

static void test_append_to_string(void MELICUS_UNUSED **state) {
    char *test_str = NULL;
    add_to_string(&test_str, "Test");
    assert_string_equal(test_str, "Test");

    free(test_str);
    test_str = NULL;

    add_to_string(&test_str, "");
    assert_string_equal(test_str, "");
    free(test_str);

    test_str = NULL;
    add_to_string(&test_str, NULL);
    assert_ptr_equal(test_str, NULL);

}

static void test_add_char_to_string(void MELICUS_UNUSED **state) {
    char *test_str = NULL;
    add_char_to_string(&test_str, 'h');
    assert_true(length(test_str) == 1);
    add_char_to_string(&test_str, 'e');
    assert_true(length(test_str) == 2);
    add_char_to_string(&test_str, 'l');
    assert_true(length(test_str) == 3);
    add_char_to_string(&test_str, 'l');
    assert_true(length(test_str) == 4);
    add_char_to_string(&test_str, 'o');
    assert_true(length(test_str) == 5);
    assert_string_equal(test_str, "hello");
    free(test_str);
}

static void test_check_suffix(void MELICUS_UNUSED **state) {
    char h1[] = "Hello World";
    assert_true(check_suffix(h1, "Hello", length(h1), length("Hello")));
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_find_in_string),
        cmocka_unit_test(test_append_to_string),
        cmocka_unit_test(test_add_char_to_string),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
