#include <stdio.h>
#include <stdlib.h>
#include "musicdb.h"

struct MusicDB { 
    Song   **songs; //In order: year, title, artist, rating, num_rated
    size_t   n_songs; //just an int
    size_t   cap_songs;
    Artist **artists; //name, n_songs, cap, songs
    size_t   n_artists;
    size_t   cap_artists;
};

MusicDB *db_create(void){ //create empty database

    return calloc(1, sizeof(MusicDB)); //allocate memory the size of MusicDB


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