#include <stddef.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string_utils.h>
#include <song_data.h>
#include <sm.h>

void test_find_link(void MELICUS_UNUSED **state) {
    int fd = open("tests/page.html", O_RDONLY, S_IRUSR | S_IWUSR);
    struct stat sb;

    if (fstat(fd, &sb) == - 1) {
        perror("Error getting file info");
    }

    char *file_in_memory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    song_data data;
    char artist_name[] = "Artist 1";
    data.artist_name = artist_name;
    char song_name[] = "Song";
    data.song_name = song_name;

    char *link = sm_find_link_for_song(file_in_memory, &data);

    assert_string_equal(link, "https://song link 1");

    munmap(file_in_memory, sb.st_size);
    close(fd);
}

int main () {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_find_link),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);

}
