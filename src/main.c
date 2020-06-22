#include <ncurses.h>

#include <lyrics.h>
#include <cmus_status.h>
#include <string_utils.h>
#include <string.h>
#include <network.h>
#include <song_data.h>

song_data *current_song;

char *make_song_url(char *song_name) {
    char prefix[] = "https://songmeanings.com/query/?query=";
    char postfix[] = "&type=songtitles";
    size_t url_length = length(prefix) + length(postfix) + length(song_name) + 1;
    char *url = (char *)malloc(url_length);
    memset(url, 0, url_length);
    memcpy(url, prefix, length(prefix));
    memcpy(url + length(prefix), song_name, length(song_name));
    memcpy(url + length(prefix) + length(song_name), postfix, length(postfix));
    printf("URL: %s\n", url);
    return url;
}

int main(int argc, char *argv[]) {
    (void) argc; (void)argv;
    // Initialize current_song data
    current_song = NULL;

    // initializes screen
    // set ups memory and clear screen
    lyrics_t *lyrics = create_lyrics();
    load_lyric_from_file(lyrics, "test.lyric");

    string_split *ss = get_cmus_status();
    printf("CMUS status lines: %zu\n", ss->used_size);
    printf("CMUS Status:\n");

    for (size_t i = 0; i < ss->used_size; i++)
        printf("%s\n", ss->strings[i]);
    destroy_string_split(ss);

    free_lyrics(lyrics);

    song_data *s = get_current_song();
    if (s)
        printf("Artist: %s\nAlbum: %s\nSong: %s\n", s->name, s->album, s->song_name);




    char *url = make_song_url(s->song_name);

    curl_buffer *buf = (curl_buffer *)malloc(sizeof(curl_buffer));
   // memset(buf, 0, sizeof(curl_buffer));
    buf->buffer = NULL;
    buf->size = 0;


    CURL *handle = make_handle(url, buf);
    curl_easy_perform(handle);
    printf("%s", buf->buffer);
    curl_easy_cleanup(handle);

    free(buf->buffer);
    free(buf);
    destroy_song_data(s);
    return 0;
}

/*
 *
     initscr();

    struct winsize max;
    ioctl(1, TIOCGWINSZ, &max);
    //getmaxyx(0, height, width);
    int height = 3;
    printf("width: %d, height %d\n", max.ws_row, max.ws_col);
    //    mvprintw(height, init_pos, "%s", l.c_str());

    nodelay(stdscr, true);

  //
  //  char *line;
  //  while (true) {
  //      char *line = next_line(lyrics);
  //      if (line == 0) {
  //          break;
  //      }
//
  //      //printf("%s\n", line);
  //      int init_pos = center_text(length(line), max.ws_col);
  //      mvprintw(height, init_pos, "%s", line);
  //      height++;
  //  }


    //int status_file = open(status_path, O_RDONLY);

    while (true) {
        if (getch() != ERR) {
            break;
        }
    }

    //getch();
    // refresh(); // Refreshes the screen to match whats in memory

    endwin(); // Frees all ncurses related stuff/

    free_lyrics(lyrics);*/
