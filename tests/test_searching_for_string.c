#include <lyrics.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char lyric_html[] =
        "					<div class=\"holder lyric-box\"> \
         						Wake me up,<br/> \
         Test me up<br/><br/><div style=\"min-height: 25px; margin:0; padding:\
         12px 0 0 0; border-top: 1px dotted #ddd;\">";

    char *lyric = get_lyrics_from_page_string(lyric_html);
//    printf("%s\n", lyric);

    free(lyric);
    return 0;
}
