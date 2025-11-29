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
            label: Text { text: "Want to disconnect? -> RESTART GAME" },
            onclick: fn(label: Text) {
                // remove_component(ARCHIPELAGO_COMPONENT);
                // leave_ui();
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
    wall_jump: int,
    ledge_grab: int,
    swim: int,
    jumppads: int,
    required_grass: int,
    final_platform_c: int,
    final_platform_p: int,
    final_platform_known: bool,
    triggered_platforms: Set<int>,
    highest_index_received: int,
}
static mut ARCHIPELAGO_STATE = ArchipelagoState {
    last_level_unlocked: 1,
    grass: 0,
    wall_jump: 0,
    ledge_grab: 0,
    swim: 0,
    jumppads: 0,
    required_grass: 1000,
    final_platform_c: 100,
    final_platform_p: 1,
    final_platform_known: false,
    triggered_platforms: Set::new(),
    highest_index_received: -1,
};

static mut ARCHIPELAGO_COMPONENT = Component {
    id: ARCHIPELAGO_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: update_players,
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string {
        let ledge_grab = if ARCHIPELAGO_STATE.ledge_grab > 0 { "YES" } else { "NO" };
        let wall_jump = if ARCHIPELAGO_STATE.wall_jump >= 2 { "INF" } else if ARCHIPELAGO_STATE.wall_jump == 1 { "ONE" } else { "NO" };
        let jumppads = if ARCHIPELAGO_STATE.jumppads > 0 { "YES" } else { "NO" };
        let swim = if ARCHIPELAGO_STATE.swim > 0 { "YES" } else { "NO" };
        let final_platform = if ARCHIPELAGO_STATE.final_platform_known { f"{ARCHIPELAGO_STATE.final_platform_c}-{ARCHIPELAGO_STATE.final_platform_p}" } else { "????" };
        return f"Archipelago Randomizer\nGoal: get grass {ARCHIPELAGO_STATE.grass}/{ARCHIPELAGO_STATE.required_grass}
-> go to Platform {final_platform}

Abilities
Ledge Grab: {ledge_grab}
Wall Jump: {wall_jump}
Jumppads: {jumppads}
Swim: {swim}"
    },
    draw_hud_always: fn() {},
    on_new_game: fn() {
        ARCHIPELAGO_STATE.last_level_unlocked = 1;
        ARCHIPELAGO_STATE.grass = 0;
        ARCHIPELAGO_STATE.ledge_grab = 0;
        ARCHIPELAGO_STATE.wall_jump = 0;
        ARCHIPELAGO_STATE.swim = 0;
        ARCHIPELAGO_STATE.jumppads = 0;
        Tas::archipelago_deactivate_all_buttons(-1);
        log("Reset last_level_unlocked");
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
            log(f"$Platform {index.cluster_index + 1}-{index.element_index + 1}");
            Tas::archipelago_send_check(10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1);

            if index.cluster_index == ARCHIPELAGO_STATE.final_platform_c - 1 && index.element_index == ARCHIPELAGO_STATE.final_platform_p - 1 && ARCHIPELAGO_STATE.grass >= ARCHIPELAGO_STATE.required_grass {
                Tas::archipelago_goal();
                Tas::archipelago_trigger_goal_animation();
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
fn archipelago_received_item(index: int, item_index: int){
    if index <= ARCHIPELAGO_STATE.highest_index_received {
        log(f"Ignoring duplicate or out-of-order item index {index} (highest received: {ARCHIPELAGO_STATE.highest_index_received})");
        return;
    }
    ARCHIPELAGO_STATE.highest_index_received = index;
    if item_index == 9999990 {  // Ledge Grab
        log("Received Ledge Grab!");
        ARCHIPELAGO_STATE.ledge_grab += 1;
        Tas::archipelago_set_wall_jump_and_ledge_grab(-1, 1);
    }
    if item_index == 9999991 {  // Wall Jump
        log("Received Wall Jump!");
        ARCHIPELAGO_STATE.wall_jump += 1;
        Tas::archipelago_set_wall_jump_and_ledge_grab(ARCHIPELAGO_STATE.wall_jump, -1);
    }
    if item_index == 9999992 {  // Swim
        log("Received Swim!");
        ARCHIPELAGO_STATE.swim += 1;
        Tas::set_kill_z(-6000.);
    }
    if item_index == 9999993 {  // Jumppads
        log("Received Jumppads!");
        ARCHIPELAGO_STATE.jumppads += 1;
        Tas::archipelago_set_jump_pads(1);
    }
    if item_index == 9999997 {  // Final Platform Known
        ARCHIPELAGO_STATE.final_platform_known = true;
    }
    if item_index == 9999999 {  // Grass
        archipelago_got_grass();
    }
    let clusterindex = item_index - 10000000;
    if clusterindex >= 2 && clusterindex < 32 {
        if !ARCHIPELAGO_STATE.triggered_platforms.contains(clusterindex) {
            Tas::archipelago_activate_all_buttons(-1);
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
            Tas::archipelago_deactivate_all_buttons(-1);

            ARCHIPELAGO_STATE.triggered_platforms.insert(clusterindex);
        }
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
        Tas::archipelago_activate_all_buttons(item_index-40000000);
    }
    if item_index >= 50000000 && item_index < 60000000{
        log(f"DEBUG deactivate button {item_index - 50000000} Button 0");
        Tas::archipelago_deactivate_all_buttons(item_index-50000000);
    }
    if item_index == 60000000{
        Tas::archipelago_set_wall_jump_and_ledge_grab(0, 0);
    }
    if item_index == 60000001{
        Tas::archipelago_set_wall_jump_and_ledge_grab(1, -1);
    }
    if item_index == 60000005{
        Tas::set_kill_z(-60.);
    }
    if item_index == 60000010{
        Tas::archipelago_set_jump_pads(0);
    }
    if item_index == 60000015{
        Tas::archipelago_trigger_goal_animation();
    }
}

fn archipelago_got_grass(){
    ARCHIPELAGO_STATE.grass += 1;
    log("Got grass!");
}

fn archipelago_start(){
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.grass = 0;
    ARCHIPELAGO_STATE.wall_jump = 0;
    Tas::set_kill_z(-60.);
    Tas::archipelago_set_wall_jump_and_ledge_grab(0, 0);
    Tas::archipelago_set_jump_pads(0);
    log("Archipelago started");
}

fn archipelago_checked_location(cluster: int, platform: int){
    log(f"Already checked locations {cluster} {platform}");
    Tas::trigger_element(ElementIndex { cluster_index: cluster-1, element_type: ElementType::Platform, element_index: platform-1 });
}

fn archipelago_received_slot_data(key: string, value: string){
    if key == "ap_world_version" {
        log(f"Archipelago World Version: {value}");
    }
    
    if key == "required_grass" {
        ARCHIPELAGO_STATE.required_grass = value.parse_int().unwrap();
    }

    if key == "finish_platform_c" {
        ARCHIPELAGO_STATE.final_platform_c = value.parse_int().unwrap();
    }

    if key == "finish_platform_p" {
        ARCHIPELAGO_STATE.final_platform_p = value.parse_int().unwrap();
    }

    if key == "final_platform_known" {
        let known = value == "true";
        if known {
            ARCHIPELAGO_STATE.final_platform_known = true;
        } else {
            ARCHIPELAGO_STATE.final_platform_known = false;
        }
    }
}
