#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <dolphin/dolphin.h>
#include <flipper_format/flipper_format.h>
#include <blackjack_icons.h>

#define DEALER_MAX_HAND 17
#define PLAYER_MAX_HAND 21
#define CARD_RANK_NUM   13
#define SUITS           4
#define SAVE_FILE       "blackjack_settings.ff"

typedef struct {
    const Icon* heart = &I_heart;
    const Icon* diamond = &I_diamond;
    const Icon* spade = &I_spade;
    const Icon* club = &I_club;
} Suits;

typedef struct {
    const Icon* ace = &I_1;
    const Icon* two = &I_2;
    const Icon* three = &I_3;
    const Icon* four = &I_4;
    const Icon* five = &I_5;
    const Icon* six = &I_6;
    const Icon* seven = &I_7;
    const Icon* eight = &I_8;
    const Icon* nine = &I_9;
    const Icon* ten = &I_10;
    const Icon* jack = &I_11;
    const Icon* queen = &I_12;
    const Icon* king = &I_13;

} Ranks;

typedef struct {
    bool backpressed;
    bool okpressed;
    bool firsttime;
    bool menu;
    bool tutorial;
    bool game;
    bool paused;
    int bet;
    int money;
    const Icon* suits[SUITS];
    const Icon* ranks[CARD_RANK_NUM];
    const Icon* cards[2];
    const Ranks* rank;
    const Suits* card;
} GameState;

void rendercard(Canvas* canvas, Icon* suit, Icon* rank, int x, int y) {
    canvas_draw_icon(canvas, &I_card1, x, y);
    canvas_draw_icon(canvas, x + 3, y - 3, rank);
    canvas_draw_icon(canvas, x + 3, y - 12, suit);
}

/*
void renderblankcard(Canvas* canvas, int x, int y) {

} */

int has_tutorial_been_completed() {
    Storage* storage = furi_record_open("storage");
    FlipperFormat* file = flipper_format_file_alloc(storage);
    int32_t tutorial_done = 0;

    if(flipper_format_file_open_existing(file, SAVE_FILE)) {
        flipper_format_read_int32(file, "tutorial_done", &tutorial_done, 1);
        flipper_format_file_close(file);
    }

    flipper_format_file_close(file);
    furi_record_close("storage");
    return tutorial_done;
}

void mark_tutorial_completed() {
    Storage* storage = furi_record_open("storage");
    FlipperFormat* file = flipper_format_file_alloc(storage);
    int32_t tutorial_done = 1;

    if(flipper_format_file_open_always(file, SAVE_FILE)) {
        flipper_format_write_int32(file, "tutorial_done", &tutorial_done, 1);
        flipper_format_file_close(file);
    }

    flipper_format_file_close(file);
    furi_record_close("storage");
}

void render_callback(Canvas* canvas, void* context) {
    GameState* state = (GameState*)context;
    if(state->menu) {
        canvas_draw_icon(canvas, 0, 0, &I_title);
    } else if(state->tutorial) {
        canvas_draw_icon(canvas, 0, 0, &I_tutorial);
    } else if(state->game) {
        canvas_clear(canvas);
        canvas_draw_str(canvas, 30, 10, "Render test");
        for(int i = 0; i < SUITS; i++) {
            canvas_draw_icon(canvas, i * 10, 10, state->suits[i]);
        }
        for(int i = 0; i < CARD_RANK_NUM; i++) {
            if (i == 10) {canvas_draw_icon(canvas, i * 5 + 2, 20, state->ranks[i]);} else {canvas_draw_icon(canvas, i * 5, 20, state->ranks[i]);}
        }
        for(int i = 0; i < 2; i++) {
            canvas_draw_icon(canvas, i * 15, 32, state->cards[i]);
        }
        canvas_draw_icon(canvas, 2, 34, state->ranks[0]);
        canvas_draw_icon(canvas, 2, 42, state->suits[0]);
    }
}

void input_callback(InputEvent* event, void* context) {
    GameState* state = (GameState*)context;
    if(event->type == InputTypePress) {
        if(event->key == InputKeyBack) {
            state->backpressed = true;
        }
        if(event->key == InputKeyOk) {
            state->okpressed = true;
        }
    }
}

int32_t f0blackjack_app(void* p) {
    UNUSED(p);

    Gui* gui = furi_record_open("gui");
    ViewPort* viewport = view_port_alloc();

    GameState game_state = {
        .backpressed = false,
        .okpressed = false,
        .firsttime = true,
        .menu = true,
        .tutorial = !has_tutorial_been_completed(),
        .game = false,
        .paused = false,
        .bet = 10,
        .money = 50,
        .suits = {&I_club, &I_spade, &I_heart, &I_diamond},
        .ranks = {&I_1, &I_2, &I_3, &I_4, &I_5, &I_6, &I_7, &I_8, &I_9, &I_10, &I_11, &I_12, &I_13},
        .cards = {&I_card1, &I_card2},
        .suits = NULL,
        .rank = NULL,
    };

    view_port_draw_callback_set(viewport, render_callback, &game_state);
    view_port_input_callback_set(viewport, input_callback, &game_state);
    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    while(true) {
        if(game_state.backpressed) {
            if(game_state.menu) {
                break;
            } else if(game_state.tutorial) {
                game_state.tutorial = false;
                game_state.menu = true;
            }
            game_state.backpressed = false;
        }
        if(game_state.okpressed) {
            if(game_state.menu) {
                game_state.menu = false;
            } else if(game_state.tutorial) {
                game_state.tutorial = false;
                game_state.game = true;
            } else if(game_state.game) {
                break;
            }
            game_state.okpressed = false;
        }
        furi_delay_ms(10);
    }

    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close("gui");

    return 0;
}
