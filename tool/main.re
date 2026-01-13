include "log.re";
include "settings.re";
include "keys.re";
include "component.re"
include "prelude.re";
include "ui.re";
include "teleport.re";
include "randomizer.re";
include "newgame.re";
include "practice.re";
include "windshieldwipers.re";
include "tas.re";
include "timer.re";
include "minimap.re";
include "multiplayer.re";
include "archipelago_gameplay.re";
include "archipelago_log.re"
include "archipelago_meta.re"
include "archipelago_ui.re";
include "movement.re";
include "misc.re";
include "mapeditor.re";
include "world_options.re";
include "player.re";

static mut NEW_VERSION: Option<string> = Tas::new_version_string();

fn create_start_menu() -> Ui {
    let mut start_menu_text = Text { text: "Press 'm' for menu." };
    add_component(AP_LOG_COMPONENT);
    Ui {
        name: start_menu_text,
        elements: List::new(),
        on_draw: Option::Some(fn() {
            let mut text = "Press 'm' for menu.";
            for comp in CURRENT_COMPONENTS {
                let draw_hud_text = comp.draw_hud_text;
                text = draw_hud_text(text);
            }
            start_menu_text.text = text;
        }),
        selected: 0,
    }
}

fn create_base_menu() -> Ui {
    create_archipelago_menu()
}

enter_ui(create_start_menu());

loop {
    let mut tick_mode = TickMode::DontCare;
    for comp in CURRENT_COMPONENTS {
        match comp.tick_mode {
            TickMode::DontCare => (),
            TickMode::Yield => tick_mode = TickMode::Yield,
        }
    }
    Tas::show_hud();
    Tas::set_sky_time_speed(SETTINGS.sky_time_speed, SETTINGS.sky_time_speed);
    step_frame(tick_mode);
}
