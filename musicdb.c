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

// -----------FUNCTIONS -------------------------
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
        strcpy(s->title, artist); 

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
    Artist **end = db->artists + db->n_artists;  //1 past the last slot
    const Artist *best = *p;


    for(p = p+1; p < end ; p++){

        if (best == NULL || (*p)->n_songs > best->n_songs || ((*p)->n_songs == best->n_songs && strcmp (best->name, (*p)->name) > 0 )){ 
            best = *p;
        }

    }
    return best;
}

