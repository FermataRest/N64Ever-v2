#include <stdbool.h>
#include <stdio.h>
#include <strings.h>
#include <libdragon.h>
#include "../bgm.h"
#include "../sound.h"
#include "../settings.h"
#include "../ui_components/constants.h"
#include "../fonts.h"
#include "views.h"

/* ---------- helpers ---------- */

static const char *on_off(bool v) { return v ? "On" : "Off"; }

static const char *screensaver_name(screensaver_mode_t m) {
    return (m == SCREENSAVER_OFF) ? "Off" : "On";
}

/* Allowed idle-timeout values for the screensaver (seconds). */
static const int ss_timer_values[] = { 30, 60, 120, 180, 240, 300, 600, 1200, 1800, 3600 };
#define SS_TIMER_COUNT ((int)(sizeof(ss_timer_values) / sizeof(ss_timer_values[0])))

static const char *fmt_ss_timer(int sec) {
    static char buf[8];
    if (sec < 60) snprintf(buf, sizeof(buf), "%ds", sec);
    else          snprintf(buf, sizeof(buf), "%dm", sec / 60);
    return buf;
}
/* ---------- static menu label ----------- */
static const char lbl_bgm_selection[] = "Background Music Selection";
static const char lbl_border_color_selection[] = "Border Color Selection";
static char lbl_border_effects[28];
static char lbl_rgb_selector[28];
static char lbl_border_red[16];
static char lbl_border_green[16];
static char lbl_border_blue[16];

/* ---------- dynamic menu labels ---------- */

static char lbl_hidden[30];
static char lbl_soundfx[24];
static char lbl_bgm[24];
static char lbl_splash[20];
static char lbl_bg_image[26];
static char lbl_custom_files[30];
static char lbl_saves_use[24];
static char lbl_saves_show[26];
static char lbl_save_files[22];
static char lbl_cheat_files[22];
static char lbl_pal60[20];
static char lbl_font[20];
static char lbl_screensaver[28];
static char lbl_ss_fav[36];
static char lbl_ss_timer[30];
static char lbl_boot_opt[26];
#ifdef BETA_SETTINGS
static char lbl_rom_ext[26];
static char lbl_rom_tags[20];
static char lbl_rumble[22];
#endif

static void update_settings_labels(menu_t *menu) {
    snprintf(lbl_hidden,      sizeof(lbl_hidden),      "Show Hidden: %s",      on_off(menu->settings.show_protected_entries));
    snprintf(lbl_soundfx,     sizeof(lbl_soundfx),     "Sound Effects: %s",    on_off(menu->settings.soundfx_enabled));
    snprintf(lbl_bgm,         sizeof(lbl_bgm),         "Background Music: %s", on_off(menu->settings.bgm_enabled));
    snprintf(lbl_splash,      sizeof(lbl_splash),      "Boot Splash: %s",      on_off(menu->settings.splash_enabled));
    snprintf(lbl_bg_image,    sizeof(lbl_bg_image),    "Custom Splash: %s", on_off(menu->settings.custom_splash_enabled));
    snprintf(lbl_custom_files,sizeof(lbl_custom_files), "Use Custom Files: %s", on_off(menu->settings.use_custom_files));
    snprintf(lbl_saves_use,   sizeof(lbl_saves_use),   "Use Saves Folder: %s", on_off(menu->settings.use_saves_folder));
    snprintf(lbl_saves_show,  sizeof(lbl_saves_show),  "Show Saves Folder: %s",on_off(menu->settings.show_saves_folder));
    snprintf(lbl_save_files,  sizeof(lbl_save_files),  "Show Save Files: %s",  on_off(menu->settings.show_save_files));
    snprintf(lbl_cheat_files, sizeof(lbl_cheat_files), "Show Cheat Files: %s", on_off(menu->settings.show_cheat_files));
    snprintf(lbl_pal60,       sizeof(lbl_pal60),       "PAL60 Mode: %s",       on_off(menu->settings.pal60_enabled));
    snprintf(lbl_font,        sizeof(lbl_font),        "Font: %s",            menu->settings.use_legacy_font ? "Classic" : "Pixel");
    snprintf(lbl_screensaver, sizeof(lbl_screensaver), "Screensaver: %s",     screensaver_name(menu->settings.screensaver_mode));
    snprintf(lbl_ss_fav,      sizeof(lbl_ss_fav),      "Favorite Screensaver: %s", on_off(menu->settings.screensaver_favorites_only));
    snprintf(lbl_ss_timer,    sizeof(lbl_ss_timer),    "Screensaver Timer: %s", fmt_ss_timer(menu->settings.screensaver_timeout_sec));
    snprintf(lbl_border_effects, sizeof(lbl_border_effects), "Border Effects: %s", menu->settings.border_rainbow_enabled ? "Rainbow" : "Off");
    if (menu->settings.border_rainbow_enabled) {
        lbl_rgb_selector[0] = '\0';   /* hidden while rainbow is active (blank rows are skipped in navigation) */
    } else {
        snprintf(lbl_rgb_selector, sizeof(lbl_rgb_selector), "RGB Border Selector");
    }
    snprintf(lbl_border_red,   sizeof(lbl_border_red),   "Red: %d",   menu->settings.border_color_r);
    snprintf(lbl_border_green, sizeof(lbl_border_green), "Green: %d", menu->settings.border_color_g);
    snprintf(lbl_border_blue,  sizeof(lbl_border_blue),  "Blue: %d",  menu->settings.border_color_b);

#ifdef FEATURE_AUTOLOAD_ROM_ENABLED
    snprintf(lbl_boot_opt, sizeof(lbl_boot_opt), "ROM Loading Bar: %s",  on_off(menu->settings.loading_progress_bar_enabled));
#else
    snprintf(lbl_boot_opt, sizeof(lbl_boot_opt), "Fast Reboot ROM: %s",  on_off(menu->settings.rom_fast_reboot_enabled));
#endif
#ifdef BETA_SETTINGS
    snprintf(lbl_rom_ext,  sizeof(lbl_rom_ext),  "Hide ROM Extension: %s", on_off(menu->settings.show_browser_file_extensions));
    snprintf(lbl_rom_tags, sizeof(lbl_rom_tags), "Hide ROM Tags: %s",      on_off(menu->settings.show_browser_rom_tags));
    snprintf(lbl_rumble,   sizeof(lbl_rumble),   "Rumble Feedback: %s",    on_off(menu->settings.rumble_enabled));
#endif
}

/* ---------- toggle actions ---------- */

static bool settings_action_fired = false;
static bool settings_was_open     = false;
static int  settings_saved_row    = 0;
static int rgb_editing_field = -1;  /* -1 = not editing 0=R, 1=G, 2=B*/

static void toggle_hidden(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.show_protected_entries = !menu->settings.show_protected_entries;
    settings_save(&menu->settings);
    menu->browser.reload = true;
    update_settings_labels(menu);
}
static void toggle_soundfx(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.soundfx_enabled = !menu->settings.soundfx_enabled;
    sound_use_sfx(menu->settings.soundfx_enabled);
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_bgm(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.bgm_enabled = !menu->settings.bgm_enabled;
    if (menu->settings.bgm_enabled) {
	    bgm_start(menu->storage_prefix, menu->settings.bgm_selected_track);
    } else {
	   bgm_stop();
    }
    settings_save(&menu->settings);
    update_settings_labels(menu);
}

static void toggle_border_effects(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.border_rainbow_enabled = !menu->settings.border_rainbow_enabled;
    ui_components_set_border_mode(menu->settings.border_rainbow_enabled, menu->settings.border_color_r, menu->settings.border_color_g, menu->settings.border_color_b);
    settings_save(&menu->settings);
    update_settings_labels(menu);
}

static void enter_edit_red(menu_t *menu, void *arg) {
    settings_action_fired = true;
    rgb_editing_field = 0;
    sound_play_effect(SFX_SETTING);
}
static void enter_edit_green(menu_t *menu, void *arg) {
    settings_action_fired = true;
    rgb_editing_field = 1;
    sound_play_effect(SFX_SETTING);
}
static void enter_edit_blue(menu_t *menu, void *arg) {
    settings_action_fired = true;
    rgb_editing_field = 2;
    sound_play_effect(SFX_SETTING);
}

static int *rgb_field_ptr(menu_t *menu, int field) {
    switch (field) {
        case 0:  return &menu->settings.border_color_r;
        case 1:  return &menu->settings.border_color_g;
        default: return &menu->settings.border_color_b;
    }
}

static uint32_t rgb_hold_start_ms = 0;
static uint32_t rgb_last_step_ms  = 0;

static void process_rgb_editor(menu_t *menu) {
    if (menu->actions.back || menu->actions.enter) {
        rgb_editing_field = -1;
        rgb_hold_start_ms = 0;
        settings_save(&menu->settings);
        sound_play_effect(SFX_SETTING);
        update_settings_labels(menu);
        return;
    }

    if (!menu->actions.go_up && !menu->actions.go_down) {
        rgb_hold_start_ms = 0;
        return;
    }

    uint32_t now = (uint32_t) get_ticks_ms();
    if (rgb_hold_start_ms == 0) {
        rgb_hold_start_ms = now;
        rgb_last_step_ms  = 0;   /* force an immediate first step */
    }

    uint32_t held = now - rgb_hold_start_ms;
    /* Acceleration: slow at first, faster the longer the button is held. */
    uint32_t step_interval = held > 2000 ? 20 : held > 800 ? 60 : 150;

    if (rgb_last_step_ms != 0 && (now - rgb_last_step_ms) < step_interval) {
        return;
    }
    rgb_last_step_ms = now;

    int *val = rgb_field_ptr(menu, rgb_editing_field);
    *val += menu->actions.go_up ? 1 : -1;
    if (*val < 0)   *val = 0;
    if (*val > 255) *val = 255;

    ui_components_set_border_mode(menu->settings.border_rainbow_enabled, menu->settings.border_color_r, menu->settings.border_color_g, menu->settings.border_color_b);
    update_settings_labels(menu);
}

static void toggle_splash(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.splash_enabled = !menu->settings.splash_enabled;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_bg_image(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.custom_splash_enabled = !menu->settings.custom_splash_enabled;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_custom_files(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.use_custom_files = !menu->settings.use_custom_files;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_font(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.use_legacy_font = !menu->settings.use_legacy_font;
    settings_save(&menu->settings);
    /* Free the grid cover cache first: the Firple font is a 486KB contiguous alloc that a
       full+fragmented cache (cap 80) can't make room for -> asset_load asserts ("Out of memory"
       toggling the font). The grid isn't visible from Settings and reloads covers on return. */
    view_grid_release_boxart();
    fonts_reload(menu->settings.use_legacy_font);   /* apply live */
    update_settings_labels(menu);
}
static void cycle_screensaver(menu_t *menu, void *arg) {
    (void)arg;
    settings_action_fired = true;
    int m = (int)menu->settings.screensaver_mode + 1;
    if (m >= SCREENSAVER_COUNT) m = SCREENSAVER_OFF;
    menu->settings.screensaver_mode = (screensaver_mode_t)m;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_ss_fav(menu_t *menu, void *arg) {
    (void)arg;
    settings_action_fired = true;
    menu->settings.screensaver_favorites_only = !menu->settings.screensaver_favorites_only;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void cycle_ss_timer(menu_t *menu, void *arg) {
    (void)arg;
    settings_action_fired = true;
    int idx = 0;
    for (int i = 0; i < SS_TIMER_COUNT; i++) {
        if (ss_timer_values[i] == menu->settings.screensaver_timeout_sec) { idx = i; break; }
    }
    idx = (idx + 1) % SS_TIMER_COUNT;
    menu->settings.screensaver_timeout_sec = ss_timer_values[idx];
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_saves_use(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.use_saves_folder = !menu->settings.use_saves_folder;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_saves_show(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.show_saves_folder = !menu->settings.show_saves_folder;
    settings_save(&menu->settings);
    menu->browser.reload = true;
    update_settings_labels(menu);
}
static void toggle_save_files(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.show_save_files = !menu->settings.show_save_files;
    settings_save(&menu->settings);
    menu->browser.reload = true;
    update_settings_labels(menu);
}
static void toggle_cheat_files(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.show_cheat_files = !menu->settings.show_cheat_files;
    settings_save(&menu->settings);
    menu->browser.reload = true;
    update_settings_labels(menu);
}

/* PAL60 — handled separately: confirm countdown popup */
static bool     pal60_confirm_active = false;
static uint32_t pal60_deadline_ms    = 0;   /* wall-clock ms when the auto-revert fires (fps-independent) */
static bool     pal60_prev_value     = false;
static int  pal60_choice         = 0;     /* 0 = Keep, 1 = Revert */

static void pal60_apply(bool enable) {
    if (get_tv_type() == TV_PAL) {
        vi_set_timing_preset(enable ? &VI_TIMING_PAL60 : &VI_TIMING_PAL);
    }
}

static void toggle_pal60(menu_t *menu, void *arg) {
    bool next = !menu->settings.pal60_enabled;
    if (get_tv_type() != TV_PAL) {
        /* Non-PAL console: cycle the value but no timing change; reopen normally. */
        settings_action_fired = true;
        menu->settings.pal60_enabled = next;
        settings_save(&menu->settings);
        update_settings_labels(menu);
        return;
    }
    pal60_prev_value = menu->settings.pal60_enabled;
    pal60_apply(next);
    menu->settings.pal60_enabled = next;
    settings_save(&menu->settings);
    update_settings_labels(menu);
    pal60_confirm_active = true;
    pal60_deadline_ms    = (uint32_t)get_ticks_ms() + 10000;   /* 10 s to confirm, wall-clock */
    pal60_choice         = 0;
    /* Do NOT set settings_action_fired — the confirm popup reopens the menu. */
}

#ifdef FEATURE_AUTOLOAD_ROM_ENABLED
static void toggle_loading_bar(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.loading_progress_bar_enabled = !menu->settings.loading_progress_bar_enabled;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
#else
static void toggle_fast_reboot(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.rom_fast_reboot_enabled = !menu->settings.rom_fast_reboot_enabled;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
#endif

#ifdef BETA_SETTINGS
static void toggle_rom_ext(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.show_browser_file_extensions = !menu->settings.show_browser_file_extensions;
    settings_save(&menu->settings);
    menu->browser.reload = true;
    update_settings_labels(menu);
}
static void toggle_rom_tags(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.show_browser_rom_tags = !menu->settings.show_browser_rom_tags;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
static void toggle_rumble(menu_t *menu, void *arg) {
    settings_action_fired = true;
    menu->settings.rumble_enabled = !menu->settings.rumble_enabled;
    settings_save(&menu->settings);
    update_settings_labels(menu);
}
#endif

/* ---------- background music track selection ---------- */

static component_context_menu_t *bgm_track_submenu = NULL;
static char **bgm_track_names = NULL;
static char **bgm_track_labels = NULL;
static int bgm_track_count = 0;

#define BGM_LABEL_MAXLEN 28

static void format_track_label(char *out, size_t out_size, const char *filename) {
    char name[256];
    strncpy(name, filename, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    size_t len = strlen(name);
    if (len > 4 && strcasecmp(name + len - 4, ".mp3") == 0) {
        name[len - 4] = '\0';
        len -= 4;
    }

    if (len > BGM_LABEL_MAXLEN) {
        snprintf(out, out_size, "%.*s...", BGM_LABEL_MAXLEN - 3, name);
    } else {
        snprintf(out, out_size, "%s", name);
    }
}

static void select_bgm_track(menu_t *menu, void *arg) {
    settings_action_fired = true;
    char *filename = (char *) arg;
    free(menu->settings.bgm_selected_track);
    menu->settings.bgm_selected_track = strdup(filename);
    settings_save(&menu->settings);
    bgm_stop();
    bgm_start(menu->storage_prefix, menu->settings.bgm_selected_track);
    update_settings_labels(menu);
}

static void free_bgm_track_submenu(void) {
    if (bgm_track_submenu) {
        free(bgm_track_submenu);
        bgm_track_submenu = NULL;
    }
    if (bgm_track_labels) {
        for (int i = 0; i < bgm_track_count; i++) {
            free(bgm_track_labels[i]);
        }
        free(bgm_track_labels);
        bgm_track_labels = NULL;
    }
    if (bgm_track_names) {
        bgm_free_track_names(bgm_track_names, bgm_track_count);
        bgm_track_names = NULL;
        bgm_track_count = 0;
    }
}

/* ---------- context menu ---------- */

static bool show_message_reset_settings = false;
static bool show_message_clear_favorites = false;
static bool show_message_clear_history = false;

static void gs_reset (menu_t *menu, void *arg) {
    (void)menu; (void)arg;
    show_message_reset_settings = true;
}

static void gs_clear_favorites (menu_t *menu, void *arg) {
    (void)menu; (void)arg;
    show_message_clear_favorites = true;
}

static void gs_clear_history (menu_t *menu, void *arg) {
    (void)menu; (void)arg;
    show_message_clear_history = true;
}

static component_context_menu_t rgb_border_menu = { .list = {
    { .text = lbl_border_red,   .action = enter_edit_red,   .keep_open = true},
    { .text = lbl_border_green, .action = enter_edit_green, .keep_open = true},
    { .text = lbl_border_blue,  .action = enter_edit_blue, .keep_open = true},
    COMPONENT_CONTEXT_MENU_LIST_END,
}};

static component_context_menu_t border_color_menu = { .list = {
    { .text = lbl_border_effects, .action = toggle_border_effects, .keep_open = true },
    { .text = lbl_rgb_selector,   .submenu = &rgb_border_menu     },
    COMPONENT_CONTEXT_MENU_LIST_END,
}};

static component_context_menu_t options_context_menu = { .list = {
    { .text = lbl_hidden,      .action = toggle_hidden      },
    { .text = lbl_soundfx,     .action = toggle_soundfx     },
    { .text = lbl_bgm,         .action = toggle_bgm         },
    { .text = lbl_bgm_selection, .action = NULL		    },
    { .text = lbl_border_color_selection, .submenu = &border_color_menu },
    { .text = lbl_splash,      .action = toggle_splash      },
    { .text = lbl_bg_image,    .action = toggle_bg_image    },
    { .text = lbl_custom_files, .action = toggle_custom_files },
    { .text = lbl_saves_use,   .action = toggle_saves_use   },
    { .text = lbl_saves_show,  .action = toggle_saves_show  },
    { .text = lbl_save_files,  .action = toggle_save_files  },
    { .text = lbl_cheat_files, .action = toggle_cheat_files },
    { .text = lbl_pal60,       .action = toggle_pal60       },
    { .text = lbl_font,        .action = toggle_font        },
    { .text = lbl_screensaver, .action = cycle_screensaver  },
    { .text = lbl_ss_fav,      .action = toggle_ss_fav      },
    { .text = lbl_ss_timer,    .action = cycle_ss_timer     },
#ifdef FEATURE_AUTOLOAD_ROM_ENABLED
    { .text = lbl_boot_opt,    .action = toggle_loading_bar },
#else
    { .text = lbl_boot_opt,    .action = toggle_fast_reboot },
#endif
#ifdef BETA_SETTINGS
    { .text = lbl_rom_ext,     .action = toggle_rom_ext     },
    { .text = lbl_rom_tags,    .action = toggle_rom_tags    },
    { .text = lbl_rumble,      .action = toggle_rumble      },
#endif
    { .text = "" },
    { .text = "Clear all favorites", .action = gs_clear_favorites },
    { .text = "Clear all history",   .action = gs_clear_history   },
    { .text = "Reset settings",      .action = gs_reset           },
    COMPONENT_CONTEXT_MENU_LIST_END,
}};

static void build_bgm_track_submenu(menu_t *menu) {
    free_bgm_track_submenu();

    bgm_track_count = bgm_scan_track_names(menu->storage_prefix, &bgm_track_names);
    int rows = bgm_track_count > 0 ? bgm_track_count : 1;

    size_t total_size = sizeof(component_context_menu_t) + sizeof(bgm_track_submenu->list[0]) * (rows + 1);
    bgm_track_submenu = calloc(1, total_size);

    if (bgm_track_count == 0) {
        bgm_track_submenu->list[0].text = "No tracks found in bgm folder";
        bgm_track_submenu->list[1].text = NULL;
    } else {
        bgm_track_labels = malloc(bgm_track_count * sizeof(char *));
        for (int i = 0; i < bgm_track_count; i++) {
            char buf[40];
            format_track_label(buf, sizeof(buf), bgm_track_names[i]);
            bgm_track_labels[i] = strdup(buf);

            bgm_track_submenu->list[i].text = bgm_track_labels[i];
            bgm_track_submenu->list[i].action = select_bgm_track;
            bgm_track_submenu->list[i].arg = bgm_track_names[i];
        }
        bgm_track_submenu->list[bgm_track_count].text = NULL;
    }

    for (int i = 0; options_context_menu.list[i].text; i++) {
        if (options_context_menu.list[i].text == lbl_bgm_selection) {
            options_context_menu.list[i].submenu = bgm_track_submenu;
            break;
        }
    }
}


/* ---------- process ---------- */

static void process(menu_t *menu) {
    /* Track the open row every frame so we can restore it after a toggle. */
    if (options_context_menu.row_selected >= 0) {
        settings_saved_row = options_context_menu.row_selected;
    }

    if (rgb_editing_field >= 0) {
        process_rgb_editor(menu);
        return;
    }   

    /* PAL60 countdown confirm — takes over all input. */
    if (pal60_confirm_active) {
        bool revert = false;
        if ((int32_t)((uint32_t)get_ticks_ms() - pal60_deadline_ms) >= 0 || menu->actions.back) {
            revert = true;
        } else if (menu->actions.go_up || menu->actions.go_down) {
            pal60_choice = !pal60_choice;
            sound_play_effect(SFX_CURSOR);
        } else if (menu->actions.enter) {
            if (pal60_choice == 1) {
                revert = true;
            } else {
                /* Keep — just close the confirm and reopen the settings menu */
                pal60_confirm_active = false;
                sound_play_effect(SFX_ENTER);
                ui_components_context_menu_show(&options_context_menu);
                options_context_menu.row_selected = settings_saved_row;
            }
        }
        if (revert) {
            menu->settings.pal60_enabled = pal60_prev_value;
            settings_save(&menu->settings);
            pal60_apply(pal60_prev_value);
            update_settings_labels(menu);
            pal60_confirm_active = false;
            sound_play_effect(SFX_EXIT);
            ui_components_context_menu_show(&options_context_menu);
            options_context_menu.row_selected = settings_saved_row;
        }
        return;
    }

    /* Reset-settings confirmation takes over input. */
    if (show_message_reset_settings) {
        if (menu->actions.enter) {
            settings_reset_to_defaults();
            menu_show_error(menu, "Reboot N64 to take effect!");
            show_message_reset_settings = false;
            sound_play_effect(SFX_SETTING);
        } else if (menu->actions.back) {
            show_message_reset_settings = false;
            sound_play_effect(SFX_EXIT);
        }
        return;
    }

    /* Clear-all-favorites confirmation takes over input. */
    if (show_message_clear_favorites) {
        if (menu->actions.enter) {
            bookkeeping_favorite_clear_all(&menu->bookkeeping);
            show_message_clear_favorites = false;
            sound_play_effect(SFX_SETTING);
        } else if (menu->actions.back) {
            show_message_clear_favorites = false;
            sound_play_effect(SFX_EXIT);
        }
        return;
    }

    /* Clear-all-history confirmation takes over input. */
    if (show_message_clear_history) {
        if (menu->actions.enter) {
            bookkeeping_history_clear_all(&menu->bookkeeping);
            show_message_clear_history = false;
            sound_play_effect(SFX_SETTING);
        } else if (menu->actions.back) {
            show_message_clear_history = false;
            sound_play_effect(SFX_EXIT);
        }
        return;
    }

    /* The settings popup is always open and directly editable. B leaves the editor.
       (Intercepted before the context menu so B doesn't merely close the list.) */
    if (menu->actions.back && !options_context_menu.submenu) {
        menu->next_mode = menu->load.load_return_mode ? menu->load.load_return_mode : MENU_MODE_BROWSER;
        menu->load.load_return_mode = 0;
        sound_play_effect(SFX_EXIT);
        return;
    }

    /* Up/Down/A drive the toggles (and the Reset settings item). */
    ui_components_context_menu_process(menu, &options_context_menu);

    /* Keep the popup open: a toggle action auto-closes the list one frame later;
       reopen it at the same row. Don't reopen while a confirm popup is up. */
    bool is_open = (options_context_menu.row_selected >= 0);
    if (settings_was_open && !is_open && !pal60_confirm_active && !show_message_reset_settings) {
        settings_action_fired = false;
        ui_components_context_menu_show(&options_context_menu);
        options_context_menu.row_selected = settings_saved_row;
        is_open = true;
    }
    settings_was_open = is_open;
}

/* ---------- draw ---------- */

static void draw(menu_t *menu, surface_t *d) {
    rdpq_attach(d, NULL);

    /* Standard: draw over the grid. */
    view_games_grid_draw_background(menu, d);

    /* The editable settings popup (always open) — it IS the editor. The default
       directory shows as the last row inside the box. */
    ui_components_context_menu_draw(&options_context_menu);

    if (show_message_reset_settings) {
        ui_components_messagebox_draw(
            "Reset ALL settings?\n"
            "This cannot be undone.\n\n"
            "A: Yes, B: Back"
        );
    }

    if (show_message_clear_favorites) {
        ui_components_messagebox_draw(
            "Clear ALL favorites?\n"
            "This cannot be undone.\n\n"
            "A: Yes, B: Back"
        );
    }

    if (show_message_clear_history) {
        ui_components_messagebox_draw(
            "Clear ALL history?\n"
            "This cannot be undone.\n\n"
            "A: Yes, B: Back"
        );
    }

    if (pal60_confirm_active) {
        uint32_t now = (uint32_t)get_ticks_ms();
        int secs = (pal60_deadline_ms > now) ? (int)((pal60_deadline_ms - now + 999) / 1000) : 0;
        ui_components_messagebox_draw(
            "PAL60 Mode changed to %s\n"
            "Is your display OK?\n\n"
            "%s Keep setting\n"
            "%s Revert setting\n\n"
            "Auto-reverting in %d s...",
            menu->settings.pal60_enabled ? "ON" : "OFF",
            pal60_choice == 0 ? ">" : " ",
            pal60_choice == 1 ? ">" : " ",
            secs
        );
    }

    rdpq_detach_show();
}


void view_settings_init(menu_t *menu) {
    update_settings_labels(menu);
    build_bgm_track_submenu(menu);
    settings_action_fired = false;
    pal60_confirm_active  = false;
    show_message_reset_settings = false;
    ui_components_context_menu_init(&options_context_menu);
    ui_components_context_menu_init(&border_color_menu);
    ui_components_context_menu_init(&rgb_border_menu);
    /* Open the settings list immediately — it IS the editor (no separate box). */
    ui_components_context_menu_show(&options_context_menu);
    settings_saved_row = 0;
    settings_was_open  = true;
}

void view_settings_display(menu_t *menu, surface_t *display) {
    process(menu);
    draw(menu, display);
}
