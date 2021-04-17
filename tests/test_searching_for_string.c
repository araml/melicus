#include <lyrics.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <sm.h>
#include <string_utils.h>

void test_search_sm_lyric_box(void MELICUS_UNUSED **state) { 
    char lyric_html[] =
        "					<div class=\"holder lyric-box\"> \
         						Wake me up,<br/> \
         Test me up<br/><br/><div style=\"min-height: 25px; margin:0; padding:\
         12px 0 0 0; border-top: 1px dotted #ddd;\">";

    char *lyric = sm_get_lyrics_from_page_string(lyric_html);
    
    assert_true(lyric);
    
    free(lyric);
}

int main() {
     const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_search_sm_lyric_box),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
