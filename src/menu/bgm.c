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

/** @brief Scan BGM_SD_DIRECTORY and build the playlist of .mp3 paths. */
static void build_playlist (const char *storage_prefix) {
    path_t *dir = path_init((char *) storage_prefix, BGM_SD_DIRECTORY);

    if (!directory_exists(path_get(dir))) {
        path_free(dir);
        return;
    }

    dir_t info;
    int result = dir_findfirst(path_get(dir), &info);
    int capacity = 8;
    playlist = malloc(capacity * sizeof(char *));

    while (result == 0) {
        if (info.d_type != DT_DIR && file_has_extensions(info.d_name, mp3_extensions)) {
            if (playlist_count >= capacity) {
                capacity *= 2;
                playlist = realloc(playlist, capacity * sizeof(char *));
            }
            path_t *entry = path_clone_push(dir, info.d_name);
            playlist[playlist_count++] = strdup(path_get(entry));
            path_free(entry);
        }
        result = dir_findnext(path_get(dir), &info);
    }

    path_free(dir);
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

void bgm_start (const char *storage_prefix) {
    if (active) {
        return;
    }

    build_playlist(storage_prefix);
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
}

void bgm_process (void) {
    if (!active) {
        return;
    }

    mp3player_process();

    if (mp3player_is_finished()) {
        if (!load_and_play(playlist_index + 1)) {
            bgm_stop();
        }
    }
}

bool bgm_is_active (void) {
    return active;
}
