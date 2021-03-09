#ifndef WINDOW_H_
#define WINDOW_H_ 

extern size_t height, width;
extern bool window_size_changed;

void window_init();
void draw_status_bar();

struct winsize get_term_size();
/*
void draw_screen();
void draw_lyrics();
void resize_window();
*/
#endif // _WINDOW_H_
