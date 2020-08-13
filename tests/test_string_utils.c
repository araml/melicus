#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string_utils.h>

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
    assert_true(check_suffix(h1, "World", length(h1), length("World")) > 0);
}

static void test_check_prefix(void MELICUS_UNUSED **state) {
    char h1[] = "Hello World";
    assert_true(check_prefix(h1, "Hello"));
}

static void test_find_in_string(void MELICUS_UNUSED **state) {
    char long_string[] = "Hello World hello world";
    char to_find[] = "hello world";
    assert_true(find_in_string(long_string, to_find) == 12);
    assert_true(find_in_string(long_string, "Garbage") == MELICUS_ERR);
}

static void test_string_split(void MELICUS_UNUSED **state) {
    string_split *ss = create_string_split();
    assert_ptr_equal(ss->strings, NULL);
    assert_int_equal(ss->size, 0);
    assert_int_equal(ss->reserved_size, 0);

    push_to_string_split(ss, "Hello");
    assert_ptr_not_equal(ss->strings, NULL);
    assert_int_equal(ss->size, 1);
    assert_int_equal(ss->reserved_size, 1);
    push_to_string_split(ss, "World");
    assert_ptr_not_equal(ss->strings, NULL);
    assert_int_equal(ss->size, 2);
    assert_int_equal(ss->reserved_size, 2);
    push_to_string_split(ss, "!");
    assert_ptr_not_equal(ss->strings, NULL);
    assert_int_equal(ss->size, 3);
    assert_int_equal(ss->reserved_size, 4);

    destroy_string_split(ss);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_append_to_string),
        cmocka_unit_test(test_add_char_to_string),
        cmocka_unit_test(test_check_suffix),
        cmocka_unit_test(test_check_prefix),
        cmocka_unit_test(test_find_in_string),
        cmocka_unit_test(test_string_split),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
