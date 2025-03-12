#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>
#include <dolphin/dolphin.h>
#include <flipper_format/flipper_format.h>
/* generated by fbt from .png files in images folder */
#include <f0blackjack_icons.h>

typedef struct {
    bool backpressed;
    bool okpressed;
    bool firsttime;
    bool menu;
    bool tutorial;
} GameState;

void render_callback(Canvas* canvas, void* context) {
    GameState* state = (GameState*)context;
    if (state->menu) {
        canvas_draw_icon(canvas, 0, 0, &I_title);
    } else if (state->tutorial) {
          canvas_draw_icon(canvas, 0, 0, &I_tutorial);
    }
}

void input_callback(InputEvent* event, void* context) {
    GameState* state = (GameState*)context;

    if(event->type == InputTypePress) {
        if(event->key == InputKeyUp) {
            FURI_LOG_I("INPUT", "Up Pressed");
        }
        if (event->key == InputKeyBack) {
            state->backpressed = true;
        }
        if (event->key == InputKeyOk) {
            state->okpressed = true;
        }
    }
}

void tutpopup(GameState* state) {
    UNUSED(state);
    state->menu = false;
    while(state->tutorial) {
        furi_delay_ms(10);

    }
}

int32_t f0blackjack_app(void* p) {
    UNUSED(p);

    Gui* gui = furi_record_open("gui");

    ViewPort* viewport = view_port_alloc();

    GameState game_state = {false, false, true, true, false};

    view_port_draw_callback_set(viewport, render_callback, &game_state);
    view_port_input_callback_set(viewport, input_callback, &game_state);

    gui_add_view_port(gui, viewport, GuiLayerFullscreen);

    while (true) {
        if (game_state.menu){
            if (game_state.backpressed && game_state.menu) {
                break;
            }
            if (game_state.okpressed && game_state.menu) {
                game_state.okpressed = false;
                tutpopup(game_state);
            }
        }
        furi_delay_ms(10);
    }

    // Cleanup before exit
    gui_remove_view_port(gui, viewport);
    view_port_free(viewport);
    furi_record_close("gui");

    return 0;
}
