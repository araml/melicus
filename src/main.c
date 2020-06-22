#include <ncurses.h>

#include <lyrics.h>
#include <cmus_status.h>
#include <string_utils.h>
#include <string.h>
#include <network.h>
#include <song_data.h>

song_data *current_song;

const char artist[] = "artist";
const char album[] = "album";
const char song_title[] = "title";

int check_prefix(const char *prefix, const char *s) {
    for (size_t i = 0; i < length(prefix) && i < length(s); i++) {
        // CMUS_remote q data starts after the tag word, maybe regex it?
        if (prefix[i] != s[i + 4])
            return 0;
    }

    return 1;
}

void destroy_song_data(song_data *d) {
    free(d->album);
    free(d->song_name);
    free(d->name);
    free(d);
}

void if_substring_fill(char **to_fill, const char *prefix, const char *subs) {
    if (!check_prefix(prefix, subs)) {
        return;
    }

    //TODO: allocating a few bytes more here but meh
    *to_fill = (char *)malloc(length(subs));
    // hardcoding part of the cmus struct "tag album ALBUM_NAME"
    // 3 letters for tag, 2 spaces and the word album/artist/song
    for (size_t i = 0, k = length(prefix) + 5; i < length(subs); i++, k++) {
        (*to_fill)[i] = subs[k];
    }
}

song_data *get_current_song() {
    string_split *ss = get_cmus_status();
    if (ss->used_size == 0) {
        return NULL;
    }

    song_data *s = (song_data *)malloc(sizeof(song_data));
    s->name = s->album = s->song_name = NULL;

    for (size_t i = 0; i < ss->used_size; i++) {
        if_substring_fill(&(s->album), album, ss->strings[i]);
        if_substring_fill(&(s->song_name), song_title, ss->strings[i]);
        if_substring_fill(&(s->name), artist, ss->strings[i]);

        if (s->name && s->song_name && s->album)
            break;
    }

    return s;
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
    printf("Artist: %s\nAlbum: %s\nSong: %s\n", s->name, s->album, s->song_name);


    /*
    curl_buffer *buf = (curl_buffer *)malloc(sizeof(curl_buffer));
   // memset(buf, 0, sizeof(curl_buffer));
    buf->buffer = NULL;
    buf->size = 0;
    CURL *handle = make_handle("https://songmeanings.com/artist/directory/", buf);
    curl_easy_perform(handle);
    //printf("%s", buf->buffer);
    curl_easy_cleanup(handle);

    free(buf->buffer);
    free(buf);*/

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
