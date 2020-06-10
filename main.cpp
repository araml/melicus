#include <ncurses.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/ioctl.h>

std::vector<std::string> load_lyric(const char *path) {
    std::ifstream fp(path);
    std::string line;
    std::vector<std::string> lyrics;
    while (std::getline(fp, line))
        lyrics.push_back(line);
    return lyrics;
}

int center(int text_width, int screen_width) {
    screen_width = screen_width >> 1;
    text_width = screen_width >> 1;
    return screen_width - text_width;
}

int main(int argc, char *argv[]) {
    // initializes screen
    // set ups memory and clear screen
    std::vector<std::string> lyrics = load_lyric("test.lyric");
    initscr();

    struct winsize max;
    ioctl(0, TIOCGWINSZ , &max);
    //getmaxyx(0, height, width);
    //mvprintw(10, 10, "width: %d height: %d", max.ws_row, max.ws_col);
    int height = 5;
    for (auto &l : lyrics) {
        int init_pos = center(l.length(), max.ws_row);
        mvprintw(height, init_pos, "%s", l.c_str());
        height++;
    }

    getch();
    //refresh(); // Refreshes the screen to match whats in memory

    endwin(); // Frees all ncurses related stuff
    return 0;
}
