#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "musicdb.h"

static int q2_pred(const Song *s, const void *key){
    int T = *(const int *)key;
    int r = (int)(s->rating *100.0f + 0.5f); 

    return r >= 370 +20 * T && s->num_rated >= 1000000u +10000u * T; //card conditions dependent on T
}


int main(int argc, char **argv)
{
    if (argc != 3 || strlen(argv[2]) < 2) {
        fprintf(stderr, "usage: %s <folder> <last 4 digits of ID>\n", argv[0]);
        return 1;
    }

    //define the cards
    size_t len = strlen(argv[2]);
    int a = argv[2][len - 2] - '0';
    int b = argv[2][len - 1] - '0';
    int L = (a + b) % 4;
    int D = a % 5;
    int T = b % 5;
    int S = (a + 2 * b) % 4;
    
    MusicDB *db = db_create(); //sometimes it's weird seeing your life's work staring at you
     if (db == NULL || db_load(db, argv[1]) != 0) { //if failed
        fprintf(stderr, "failed to load database\n"); 
        db_destroy(db);
        return 1;
    }

    printf("ENEB344 REPORT\n");
    printf("card: L=%d D=%d T=%d S=%d\n", L, D, T, S); //based on call

    // ------PART A -------: DATA MODEL, LAYOUT
    size_t tail = sizeof(Song) - (offsetof(Song, num_rated) + sizeof(((Song *)0)->num_rated));
    printf("[A] sizeof(Song)=%zu tail_padding=%zu\n", sizeof(Song), tail);

    printf("[A] offsetof year=%zu title=%zu id=%zu artist=%zu rating=%zu num_rated=%zu\n", //offsets
       offsetof(Song, year), offsetof(Song, title), offsetof(Song, id),
       offsetof(Song, artist), offsetof(Song, rating), offsetof(Song, num_rated));

    // ------PART B -------: DATA OF ARTISTS AND SONGS ARE CORRECTY
    printf("[B] songs=%zu artists=%zu\n", db_song_count(db), db_artist_count(db));
    const Song *s0 = db_get(db, 0); //address of song 0
    if (s0 != NULL) {//print the first song
        printf("[B] song[0]: %s | %s | %d | %.2f | %u\n",  s0->title, s0->artist->name, s0->year, s0->rating, s0->num_rated);
    }  
    const Artist *top = db_top_artist(db);
    if (top != NULL){ //print the top artist
        printf("[B] top_artist: %s (%zu songs)\n", top->name, top->n_songs);
    }

    // ------PART C -------: PUSH 2 SONGS

    char title1[64], artist1[64], title2[80];
    snprintf(title1, sizeof title1, "Exam Song %s", argv[2]); //snprintf allows no overflow
    snprintf(artist1, sizeof artist1, "Exam Artist %s", argv[2]);
    snprintf(title2, sizeof title2, "Exam Song %s (Reprise)", argv[2]);

    db_push(db, title1, artist1, 4.10, 1985, 1250000);
    printf("[C] push 1: songs=%zu artists=%zu\n", db_song_count(db), db_artist_count(db));

    db_push(db, title2, "The Beatles", 3.70, 2010, 1250000);
    printf("[C] push 2: songs=%zu artists=%zu\n", db_song_count(db), db_artist_count(db));

    const Song *last = db_get(db, db_song_count(db) - 1); //db_get the last song we pushed
    if (last != NULL) printf("[C] beatles_songs=%zu\n", last->artist->n_songs);


    // ------PART D -------: SORTING and PTR-ONLY 
    //Q1: db_count_decade   
    const Song *best = NULL;
    size_t count = db_count_decade(db, 1960 + 10 * D, &best); //D from card
    printf("[D] Q1 decade=%d count=%zu best_id=%u best_title=%s\n", 
        1960 + 10 * D, 
        count, 
        best->id, 
        best->title); 



    //Q2: song_pred
    const Song **hits = NULL;
    size_t nh = db_search(db, q2_pred, &T, &hits);
    printf("[D] Q2 matches=%zu first_id=%u last_id=%u\n", 
        nh, 
        hits[0]->id, 
        hits[nh - 1]->id);
    free(hits);
    
    //-------- Sort: final part
    db_sort(db, S);
    printf("[D] sort card=%d top5 ids:", S);
    for (size_t i = 0; i < 5; i++) { //top5 list
        const Song *s = db_get(db, i);
        if (s != NULL) printf(" %u", s->id); //if it worked print the id
    }

    printf("\n");
    db_disp(db, stdout, 3);

    db_destroy(db);
    return 0;
} 



