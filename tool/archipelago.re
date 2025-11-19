fn create_archipelago_menu() -> Ui {
    Ui::new("Archipelago:", List::of(
        UiElement::Input(Input {
            label: Text { text: "Connect (server:port,slot[,password])" },
            input: "",
            onclick: fn(input: string) {
                if input.len_utf8() == 0 {
                    return;
                }
                let args = input.split(",");
                let server_and_port = match args.get(0) { Option::Some(s) => s.trim(), Option::None => return };
                let slot = match args.get(1) { Option::Some(s) => s.trim(), Option::None => return };
                let password = args.get(2);
                Tas::archipelago_connect(server_and_port, "Refunct", slot, password);
                add_component(ARCHIPELAGO_COMPONENT);
                leave_ui();
            },
            onchange: fn(input: string) {},
        }),
        UiElement::Button(UiButton {
            label: Text { text: "Disconnect" },
            onclick: fn(label: Text) {
                remove_component(ARCHIPELAGO_COMPONENT);
                leave_ui();
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: "Back" },
            onclick: fn(label: Text) { leave_ui(); },
        }),
    ))
}

struct ArchipelagoState {
    last_level_unlocked: int,
    grass: int,
}
static mut ARCHIPELAGO_STATE = ArchipelagoState {
    last_level_unlocked: 1,
    grass: 0,
};

static mut ARCHIPELAGO_COMPONENT = Component {
    id: ARCHIPELAGO_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: update_players,
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string {
        return f"{text}\nArchipelago running\nGoal: get grass {ARCHIPELAGO_STATE.grass}/100\n-> go to Platform 21-1"
    },
    draw_hud_always: fn() {},
    on_new_game: fn() {
        ARCHIPELAGO_STATE.last_level_unlocked = 1;
        ARCHIPELAGO_STATE.grass = 0;
        // log("Reset last_level_unlocked");
    },
    on_level_change: fn(old: int, new: int) {},
    on_buttons_change: fn(old: int, new: int) {
        // log(f"[AP] # buttons changed: {old} -> {new}");
    },
    on_cubes_change: fn(old: int, new: int) {
        // log(f"[AP] # cubes changed: {old} -> {new}");
    },
    on_platforms_change: fn(old: int, new: int) {
        // log(f"[AP] # platforms changed: {old} -> {new}");
    },
    on_reset: fn(old: int, new: int) {},
    on_element_pressed: fn(index: ElementIndex) {
        // log(f"[AP] Pressed {index.element_type} {index.element_index} in cluster {index.cluster_index}");
        if index.element_type == ElementType::Button {
            // log(f"Send location check {10000000 + (index.cluster_index + 1) * 100 + index.element_index + 1} to Archipelago server");
            Tas::archipelago_send_check(10000000 + (index.cluster_index + 1) * 100 + index.element_index + 1);

            // if index.cluster_index == 30 { // 31 - 1
            //     Tas::archipelago_goal();
            // }
        }
        if index.element_type == ElementType::Platform {
            log(f"Platform {index.cluster_index + 1}-{index.element_index + 1}");
            Tas::archipelago_send_check(10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1);

            if index.cluster_index == 20 && index.element_index == 0 && ARCHIPELAGO_STATE.grass >= 100 {
                Tas::archipelago_goal();
            }
        }
    },
    on_element_released: fn(index: ElementIndex) {},
    on_key_down: fn(key: KeyCode, is_repeat: bool) {},
    on_key_down_always: fn(key: KeyCode, is_repeat: bool) {},
    on_key_up: fn(key: KeyCode) {},
    on_key_up_always: fn(key: KeyCode) {},
    on_mouse_move: fn(x: int, y: int) {},
    on_component_enter: fn() {},
    on_component_exit: fn() { Tas::archipelago_disconnect(); },
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};

fn archipelago_disconnected() {
    remove_component(ARCHIPELAGO_COMPONENT);
};

// triggers cluster clusterindex
fn archipelago_trigger_cluster(item_index: int){
    let clusterindex = item_index - 10000000;
    if clusterindex < 32 {
        Tas::activate_all_buttons(-1);
        let last_unlocked = ARCHIPELAGO_STATE.last_level_unlocked;
        log(f"Received Trigger Cluster {clusterindex}");
        Tas::set_level(clusterindex - 2);
        if last_unlocked == 7 {
            Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 1 });
        }
        if last_unlocked == 10 {
            Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 1 });
        }
        if last_unlocked == 18 {
            Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 1 });
        }
        if last_unlocked == 26 {
            Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 1 });
            Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 2 });
        }
        if last_unlocked == 28 {
            Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 1 });
        }
        Tas::trigger_element(ElementIndex { cluster_index: last_unlocked - 1, element_type: ElementType::Button, element_index: 0 });
        ARCHIPELAGO_STATE.last_level_unlocked = clusterindex;
        Tas::deactivate_all_buttons(-1);
    }

    if item_index >= 20000000 && item_index < 30000000 {
        log(f"DEBUG set_level {item_index - 20000000}");
        Tas::set_level(item_index - 20000000);
    }
    if item_index >= 30000000 && item_index < 40000000{
        log(f"DEBUG trigger_element {item_index - 30000000} Button 0");
        Tas::trigger_element(ElementIndex { cluster_index: item_index - 30000000, element_type: ElementType::Button, element_index: 0 });
    }
    if item_index >= 40000000 && item_index < 50000000{
        log(f"DEBUG activate button {item_index - 40000000} Button 0");
        Tas::activate_all_buttons(item_index-40000000);
    }
    if item_index >= 50000000 && item_index < 60000000{
        log(f"DEBUG deactivate button {item_index - 50000000} Button 0");
        Tas::deactivate_all_buttons(item_index-50000000);
    }
}

fn got_grass(){
    ARCHIPELAGO_STATE.grass += 1;
    log("Got grass!");
}