#include <stddef.h>
#include <stdio.h>
 
typedef struct Song    Song;
typedef struct Artist  Artist;
typedef struct MusicDB MusicDB;          /* you define this one, in musicdb.c */
 
struct Song { //L = 1
    short         year;
    char         *title;
    unsigned int  id;
    const Artist *artist;
    float         rating;
    unsigned int  num_rated;
};

struct Artist {
    char    *name;      /* owned copy of the name; unique within one MusicDB   */
    size_t   n_songs;   /* number of songs by this artist                      */
    size_t   cap;       /* capacity of the songs array                         */
    Song   **songs;     /* owned array of (non-owning) pointers to the songs   */
};


typedef int (*song_pred)(const Song *s, const void *key);  /* non-zero = match */
 


MusicDB      *db_create(void);
void          db_destroy(MusicDB *db);
int           db_load(MusicDB *db, const char *dir);
size_t        db_song_count(const MusicDB *db);
size_t        db_artist_count(const MusicDB *db);
const Song   *db_get(const MusicDB *db, size_t i);
int           db_push(MusicDB *db, const char *title, const char *artist,
                      double rating, int year, unsigned long num_rated);
void          db_disp(const MusicDB *db, FILE *out, size_t lim);
size_t        db_search(const MusicDB *db, song_pred pred, const void *key,
                        const Song ***out);
void          db_sort(MusicDB *db, int card);
size_t        db_count_decade(const MusicDB *db, int start, const Song **best);
const Artist *db_top_artist(const MusicDB *db);
