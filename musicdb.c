#define _POSIX_C_SOURCE 200809L 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "musicdb.h"

struct MusicDB { 
    Song   **songs; //In order: year, title, artist, rating, num_rated
    size_t   n_songs; //just an int
    size_t   cap_songs;
    Artist **artists; //name, n_songs, cap, songs
    size_t   n_artists;
    size_t   cap_artists;
};

// --------------- FUNCTIONS -------------------------
MusicDB *db_create(void){ //create empty database

    return calloc(1, sizeof(MusicDB)); //allocate memory the size of MusicDB
    //everything is 0, so we need to grow this db

}


void db_destroy (MusicDB *db){ //flag ALL the data to be reused

    //If there is no database, do nothing
    if (db == NULL) return;

    //flag deletable data using free(), for every type of data 
    //db ->n_songs means "in the address of db find the member n_songs"
    for (size_t i = 0; i < db->n_songs; i++){
        free(db->songs[i]->title); //free title text (the struct just points to an address of it)
        free(db->songs[i]);        //free the Song structure
    }
    free(db->songs); //free the array

    for(size_t i = 0; i < db-> n_artists; i++){
        free(db->artists[i]->name);
        free(db->artists[i]->songs); 
        free(db->artists[i]);
    }
    free(db->artists);

    free(db);
}


/*-------------------------------------Add data functions-------------------------*/
int db_push(MusicDB *db, const char *title, const char *artist,
        double rating, int year, unsigned long num_rated){ //We should make those pointers const because they're not copies of data.

    int created = 0;

    // if didnt open / invalid
    if (db == NULL || title == NULL || artist == NULL) return -1;

    /*Creates the artist only if no artist with that exact name exists. */
    //look for an existing Artist with this exact name

    Artist *found = NULL; // pointer to address if or if not we found artist
    
    for(size_t i = 0; i < db-> n_artists; i++){
        if (strcmp(db->artists[i]->name, artist) == 0){ //Match
            found = db->artists[i]; //remember the address of the match
            break;
        }
    }

    Song *s = malloc(sizeof(Song)); //memory for Song struct 
    if (s == NULL) return -1;   

    s->title = malloc(strlen(title) + 1);  //give space for title string
    strcpy(s->title, title);

    if (s->title == NULL) {
        free(s);
        return -1;
    }

    //if no artist exists, create one
    if (found == NULL){ //no match was found
        found = calloc(1, sizeof(Artist)); //free up space for artist
        
        //check if memory is full full
        if (found == NULL) { //we couldn't free up space; computer memory is full
            free(s->title);   // undo the title copy 
            free(s);          // undo the Song 
            return -1;
        }

        found->name = malloc(strlen(artist) + 1); //give space for artist string
        strcpy(found->name, artist); 

        if (found->name == NULL){  //slowly back away
            free(found);  
            free(s->title);
            free(s);
            return -1;
        }

        created = 1;

    }
    //else, artist was already in there

    //handle growing the arrays
    if(db->n_songs == db->cap_songs){ //if we hit the capacity of songs
        size_t capNew = db->cap_songs ? db->cap_songs * 2 : 16; //handle that first time that cap_songs = 0
        Song **tmp = realloc(db->songs, capNew * sizeof(*tmp));
        
        db->songs = tmp;
        db->cap_songs = capNew;
    }

    if (found->n_songs == found->cap) { //if we hit the capacity of artists songs
        size_t capNew = found->cap ? found->cap * 2 : 4; 
        Song **tmp = realloc(found->songs, capNew * sizeof(*tmp));
        found->songs = tmp;
        found->cap = capNew;
    }

    if (created && db->n_artists == db->cap_artists) { //if the artist is new, make space for them
        size_t capNew = db->cap_artists ? db->cap_artists * 2 : 16;
        Artist **tmp = realloc(db->artists, capNew * sizeof(*tmp));
        db->artists = tmp;
        db->cap_artists = capNew;
    }

    //create the song
    s->year     = year;
    s->rating   = rating;
    s->num_rated= num_rated;
    s->id       = db->n_songs;
    s->artist   = found;

    //counts increase by exactly one song
    db->songs[db->n_songs++] = s;
    found->songs[found->n_songs++] =s;
    if (created){
        db->artists[db->n_artists++] = found;
    }

    return 0;
}

int db_load(MusicDB *db, const char *dir){ // that char is just a string

    const char *names[5] = { "songs.txt", "artists.txt", "year.txt", "ratings.txt", "people_rated.txt" };
    FILE *files[5] = { NULL, NULL, NULL, NULL, NULL}; //store files here
    char path [4096]; //max size of path

    for (int i =0; i < 5; i++){
        strcpy(path, dir);
        strcat(path, "/"); //add / 
        strcat(path, names[i]); //ex: dir/songs.txt
        files[i] = fopen(path, "r");

        if (files[i] == NULL){
            goto done; //skip the loop
        }
    }
    /*
    files[0] = songs.txt
    files[1] = artists.txt
    files[2] = year.txt
    etc.
    */


    char   *lines[5] = { NULL, NULL, NULL, NULL, NULL }; //the line's text
    size_t  caps[5]  = { 0, 0, 0, 0, 0 };  //max size of each string in line
    ssize_t n[5];
    char *f[5]; //trimmed version of lines
    char *end;
    int t = -1; //return -1 unless we finish correctly 

    while(1){ //do on each line

        int ended = 0;
        for (int i =0; i < 5; i++){
            n[i] = getline(&lines[i], &caps[i],files[i]); //get this line; we will push 
            if (n[i] == -1) ended++; //say a file ended
        }

        if (ended == 5) {
            t = 0;
            break;
        } //all files are the same size, everything went normally
        if (ended > 0) break;

        //remove whitespace
        for (int i = 0; i < 5; i++) {
            char *s = lines[i];
            while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++; //walk to end
            size_t len = strlen(s);
            while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n'))  s[--len] = '\0';
        
            f[i] = s; //initialize f before we start checking
        }
        //if empty reject it
        if (f[0][0] == '\0' || f[1][0] == '\0') break; 

        //check if valid: falls under the correct conditions
        long year = strtol(f[2], &end, 10);
        if ( *end != '\0' || f[2][0] == '\0' || year < 1000 || year > 3000) break;

        double rating = strtod(f[3], &end);
        if ( *end != '\0' || f[3][0] == '\0' || rating < 0.0 || rating > 5.0) break;

        if (f[4][0] == '-') break;
        unsigned long raters = strtoul(f[4], &end, 10);
        if ( *end != '\0' || f[4][0] == '\0') break;


        //push
        if (db_push(db, f[0], f[1], rating, (int)year, raters) != 0) break; //break when db_push fails
    }/* Loop breaks when: all five files end together, only some
   files end, a field is empty or a number is invalid,
   or db_push fails (rc stays -1). */

    done:
        for (int i = 0; i < 5; i++) { //CLOSE EVERYTHING    
            if (files[i]) fclose(files[i]);
            free(lines[i]); 
        }
        return t; //mention this as an error
} 



/* ----------------------Lookup functions--------------------------*/
const Song *db_get(const MusicDB *db, size_t i) { //Returns the song at position i in the current order, or NULL if i is out of range.
    if (db == NULL || i >= db -> n_songs) //if no database or i is out of range
        return NULL;
    
    return db->songs[i];
}

size_t db_song_count(const MusicDB *db)
{
    if (db == NULL){
        return 0;
    }else{
        return db->n_songs;
    }
}

size_t db_artist_count(const MusicDB *db)
{
    if (db == NULL){
        return 0;
    }else{
        return db->n_artists;
    }
}


/*------------------------------Search Functions----------------------*/

/* PTR-ONLY */
const Artist *db_top_artist(const MusicDB *db){

    if (db == NULL || db->n_artists == 0) return NULL;

    Artist **p = db->artists; //*p is one artists address, **p is the artist   
    Artist **end = db->artists + db->n_artists;  //address of 1 past the last slot
    const Artist *best = *p;


    while(p < end){

        if (best == NULL || (*p)->n_songs > best->n_songs || ((*p)->n_songs == best->n_songs && strcmp (best->name, (*p)->name) > 0 )){ 
            best = *p;
        }
        p++;
    }
    return best;
}

/* PTR-ONLY */
size_t db_count_decade(const MusicDB *db, int start, const Song **best){

    if (db == NULL) return 0;

    if (best != NULL) *best = NULL; //address of best song of that decade

    size_t count = 0;
    Song **p = db->songs; //p is the database. *p is the address of a song
    Song **end = db->songs + db->n_songs;  //address of 1 past the last slot
    Song *top = NULL; //address of top song


    while (p < end){
        if ((*p)-> year >= start && (*p)->year <= start + 9){
            count++;
            if (top == NULL || (*p)->num_rated > top->num_rated || ((*p)->num_rated == top->num_rated && (*p)->id < top->id)){ //highest rating, OR lowest id
                top = *p;
            }
        }
        p++;
    }

    if (best != NULL) *best = top;
    return count;
}

//helper functions for dbsort
static int cmp3(long a, long b)
{
    return (a > b) - (a < b);                    // -1= a < b, 0= a = b, or 1= a > b
}

static int hundredths(const Song *s)
{
    return (int)(s->rating * 100.0f + 0.5f);     // 4.10 becomes 410 */
}

//For qsort, we need to create 4 different ways to sort: cases 0-3
static int by_card0(const void *a, const void *b){ //rating, num_rated

    const Song *x = *(const Song **)a; //x is the address to a song
    const Song *y = *(const Song **)b;

    int c = cmp3(hundredths(y), hundredths(x)); //compare and PRAY they're not the same

    if (c == 0) c = cmp3(y->num_rated, x->num_rated); 
    if (c == 0) c = cmp3(x->id, y->id);
    return c;
}

static int by_card1(const void *a, const void *b){ //year, title

    const Song *x = *(const Song **)a; //x is the address to a song
    const Song *y = *(const Song **)b;

    int c = cmp3(x->year, y->year); //compare and PRAY they're not the same

    if (c == 0) c = strcmp(x->title, y->title);
    if (c == 0) c = cmp3(x->id, y->id);
    return c;
}

static int by_card2(const void *a, const void *b){ //artistname, year

    const Song *x = *(const Song **)a; //x is the address to a song
    const Song *y = *(const Song **)b;

    int c =  strcmp(x->artist->name, y->artist->name); //compare name

    if (c == 0) c = cmp3(y->year, x->year);
    if (c == 0) c = cmp3(x->id, y->id);
    return c;
}

static int by_card3(const void *a, const void *b){ //numrated, rated

    const Song *x = *(const Song **)a; //x is the address to a song
    const Song *y = *(const Song **)b;

    int c =  cmp3(y->num_rated, x->num_rated); //compare name

    if (c == 0) c = cmp3(hundredths(y),hundredths(x));
    if (c == 0) c = cmp3(x->id, y->id);
    return c;
}

void db_sort(MusicDB *db, int card){
//Depending on s:
/*
0	rating descending, then num_rated descending
1	year ascending, then title ascending (strcmp byte order)
2	artist name ascending (strcmp), then year descending
3	num_rated descending, then rating descending */

    if (db == NULL) return;

    Song **p = db->songs; 

    switch (card){
        case 0: qsort(p,db->n_songs, sizeof(*p), by_card0);
            break;
        case 1: qsort(p,db->n_songs, sizeof(*p), by_card1);
            break;
        case 2: qsort(p,db->n_songs, sizeof(*p), by_card2);
            break;
        case 3: qsort(p,db->n_songs, sizeof(*p), by_card3);
            break;

        default: return;
    }

}

size_t db_search(const MusicDB *db, song_pred pred, const void *key, const Song ***out){

    if (out == NULL) return 0;
    *out = NULL;
    if (db == NULL || pred == NULL || db-> n_songs == 0) return 0;

    //array of pointers (addresses) of matching songs
    const Song **matches = malloc(db->n_songs * sizeof(const Song *)); /* we want our array of addresses, matches, to be
    able to fit n_songs songs and each has the size of an address of a song*/
    if (matches == NULL) return 0;

    size_t count = 0;
    
    Song **end = db->songs + db->n_songs; //open db->songs as *s and walk n_songs away
    Song **s = db->songs; //*s is the address of a song

    while (s < end){
        if (pred(*s, key)){ //if they match (true)
            matches[count] = *s; //add *s to the end of matches
            count++;
        }
        s++;
    }
    if (count == 0) {
        free(matches);
        return 0;
    }

    *out = matches;
    return count;
}



void db_disp(const MusicDB *db, FILE *out, size_t lim)
{
     if (db == NULL || out == NULL) return;
    /* print the header line to out with fprintf */
    fprintf(out, "%-35s %-25s %-5s %-8s %s\n", "Song Title", "Artist", "Year", "Rating", "People Rated");
        
    size_t n = (lim == 0 || lim > db->n_songs) ? db->n_songs : lim; //if larger or 0, print all the songs
    
    for (size_t i = 0; i < n; i++){ 
        const Song *s = db->songs[i]; //address of song[i]
        fprintf(out, "%-35.35s %-25.25s %-5d %-8.2f %u\n", s->title, s->artist->name, s->year, s->rating, s->num_rated);

    }
    
}