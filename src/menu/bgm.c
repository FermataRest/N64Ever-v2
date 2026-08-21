/**
 * @file bgm.c
 * @brief Background Music
 * @ingroup menu
 */

#include <stdlib.h>
#include <string.h>
#include <libdragon.h>

#include "bgm.h"
#include "mp3_player.h"
#include "path.h"
#include "utils/fs.h"

static const char *mp3_extensions[] = { "mp3", NULL };

static char **playlist = NULL;
static int playlist_count = 0;
static int playlist_index = 0;
static bool active = false;
static bool single_mode = false;

static void free_playlist (void) {
    if (playlist) {
        for (int i = 0; i < playlist_count; i++) {
            free(playlist[i]);
        }
        free(playlist);
        playlist = NULL;
    }
    playlist_count = 0;
    playlist_index = 0;
}

int bgm_scan_track_names (const char *storage_prefix, char ***out_names) {
    *out_names = NULL;

    path_t *dir = path_init((char *) storage_prefix, BGM_SD_DIRECTORY);
    if (!directory_exists(path_get(dir))) {
        path_free(dir);
        return 0;
    }

    int capacity = 8;
    int count = 0;
    char **names = malloc(capacity * sizeof(char *));

    dir_t info;
    int result = dir_findfirst(path_get(dir), &info);
    while (result == 0) {
        if (info.d_type != DT_DIR && file_has_extensions(info.d_name, mp3_extensions)) {
            if (count >= capacity) {
                capacity *= 2;
                names = realloc(names, capacity * sizeof(char *));
            }
            names[count++] = strdup(info.d_name);
        }
        result = dir_findnext(path_get(dir), &info);
    }

    path_free(dir);

    if (count == 0) {
        free(names);
        return 0;
    }

    *out_names = names;
    return count;
}

void bgm_free_track_names (char **names, int count) {
    if (!names) {
        return;
    }
    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
}

/** @brief Build the internal playlist: either every .mp3 in the folder, or just one. */
static void build_playlist (const char *storage_prefix, const char *selected_filename) {
    if (selected_filename && selected_filename[0] != '\0') {
        path_t *dir = path_init((char *) storage_prefix, BGM_SD_DIRECTORY);
        path_t *entry = path_clone_push(dir, (char *) selected_filename);
        if (file_exists(path_get(entry))) {
            playlist = malloc(sizeof(char *));
            playlist[0] = strdup(path_get(entry));
            playlist_count = 1;
        }
        path_free(entry);
        path_free(dir);
        single_mode = true;
        return;
    }

    single_mode = false;

    char **names;
    int count = bgm_scan_track_names(storage_prefix, &names);
    if (count == 0) {
        return;
    }

    path_t *dir = path_init((char *) storage_prefix, BGM_SD_DIRECTORY);
    playlist = malloc(count * sizeof(char *));
    for (int i = 0; i < count; i++) {
        path_t *entry = path_clone_push(dir, names[i]);
        playlist[i] = strdup(path_get(entry));
        path_free(entry);
    }
    playlist_count = count;
    path_free(dir);
    bgm_free_track_names(names, count);
}

static bool load_and_play (int index) {
    if (playlist_count == 0) {
        return false;
    }

    playlist_index = ((index % playlist_count) + playlist_count) % playlist_count;

    if (mp3player_load(playlist[playlist_index]) != MP3PLAYER_OK) {
        return false;
    }
    if (mp3player_play() != MP3PLAYER_OK) {
        return false;
    }

    mp3player_mute(false);
    return true;
}

void bgm_start (const char *storage_prefix, const char *selected_filename) {
    if (active) {
        return;
    }

    build_playlist(storage_prefix, selected_filename);
    if (playlist_count == 0) {
        free_playlist();
        return;
    }

    if (mp3player_init() != MP3PLAYER_OK) {
        free_playlist();
        return;
    }

    if (!load_and_play(0)) {
        mp3player_deinit();
        free_playlist();
        return;
    }

    active = true;
}

void bgm_stop (void) {
    if (!active) {
        return;
    }
    mp3player_stop();
    mp3player_deinit();
    free_playlist();
    active = false;
    single_mode = false;
}

void bgm_process (void) {
    if (!active) {
        return;
    }

    mp3player_process();

    if (mp3player_is_finished()) {
        int next_index = single_mode ? playlist_index : (playlist_index + 1);
        if (!load_and_play(next_index)) {
            bgm_stop();
        }
    }
}

bool bgm_is_active (void) {
    return active;
}
