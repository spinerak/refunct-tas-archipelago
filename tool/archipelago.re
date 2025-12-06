fn create_archipelago_menu() -> Ui {
    let elements = List::new();

    if !ARCHIPELAGO_STATE.ap_connected {
        elements.push(UiElement::Input(Input {
            label: Text { text: "Connect (server:port,slot[,password])" },
            input: "",
            onclick: fn(input: string) {
                if ARCHIPELAGO_STATE.ap_connected {
                    log("Already connected to Archipelago!");
                    return;
                }
                if input.len_utf8() == 0 {
                    return;
                }
                ARCHIPELAGO_STATE = fresh_archipelago_state();
                let args = input.split(",");
                let server_and_port = match args.get(0) { Option::Some(s) => s.trim(), Option::None => return };
                let slot = match args.get(1) { Option::Some(s) => s.trim(), Option::None => return };
                let password = args.get(2);
                Tas::archipelago_connect(server_and_port, "Refunct", slot, password);
                add_component(ARCHIPELAGO_COMPONENT);
                leave_ui();
            },
            onchange: fn(input: string) {},
        }));
    }

    if ARCHIPELAGO_STATE.ap_connected {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Change gamemode" },
            onclick: fn(label: Text) {
                enter_ui(create_archipelago_gamemodes_menu());
            },
        }));
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Disconnect" },
            onclick: fn(label: Text) {
                remove_component(ARCHIPELAGO_COMPONENT);
                ARCHIPELAGO_STATE = fresh_archipelago_state();
                leave_ui();
            },
        }));
    }

    elements.push(UiElement::Button(UiButton {
            label: Text { text: "Back" },
            onclick: fn(label: Text) { leave_ui(); },
        }));

    Ui::new("Archipelago:", elements)
}



struct ArchipelagoState {
    ap_connected: bool,
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
    received_items: List<int>,
    started: int,
    triggered_clusters: List<int>,
    stepped_on_platforms: List<int>,
    highest_index_received: int,
    has_goaled: bool,
    gamemode: int,
    unlock_vanilla_minigame: bool,
    unlock_seeker_minigame: bool,
    seeker_pressed_platforms: List<int>,
    seeker_extra_pressed: List<int>,
}

fn fresh_archipelago_state() -> ArchipelagoState {
    ArchipelagoState {
        ap_connected: false,
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
        received_items: List::new(),
        started: 0,
        triggered_clusters: List::new(),
        stepped_on_platforms: List::new(),
        highest_index_received: -1,
        has_goaled: false,
        gamemode: 0,
        unlock_vanilla_minigame: false,
        unlock_seeker_minigame: false,
        seeker_pressed_platforms: List::new(),
        seeker_extra_pressed: List::new(),
    }
}

static mut ARCHIPELAGO_STATE = fresh_archipelago_state();

static platforms_with_buttons = List::of(10010101,10010203,10010302,10010404,10010501,10010601,10010701,10010702,10010811,10010906,10011001,10011003,10011108,10011201,10011301,10011403,10011502,10011601,10011702,10011802,10011801,10011905,10012007,10012108,10012202,10012303,10012401,10012505,10012612,10012607,10012609,10012708,10012818,10012808,10012904,10013012,10013102);

static mut ARCHIPELAGO_COMPONENT = Component {
    id: ARCHIPELAGO_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: update_players,
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string {
        if ARCHIPELAGO_STATE.started == 0 {
            return "Archipelago\nPress new game (in Refunct menu).";
        }
        if ARCHIPELAGO_STATE.started == 1 {
            return "Archipelago\nTouch a platform to start!";
        }

        if ARCHIPELAGO_STATE.gamemode == 1 {
            return "Archipelago - Vanilla game\nPress the buttons!";
        }

        if ARCHIPELAGO_STATE.gamemode == 3 {
            return "Archipelago - Seeker\nFind the empty platforms!";
        }

        if ARCHIPELAGO_STATE.gamemode == 0 {        
        let ledge_grab = if ARCHIPELAGO_STATE.ledge_grab > 0 { "YES" } else { "NO" };
        let wall_jump = if ARCHIPELAGO_STATE.wall_jump >= 2 { "INF" } else if ARCHIPELAGO_STATE.wall_jump == 1 { "ONE" } else { "NO" };
        let jumppads = if ARCHIPELAGO_STATE.jumppads > 0 { "YES" } else { "NO" };
        let swim = if ARCHIPELAGO_STATE.swim > 0 { "YES" } else { "NO" };
        let final_platform = if ARCHIPELAGO_STATE.final_platform_known { f"{ARCHIPELAGO_STATE.final_platform_c}-{ARCHIPELAGO_STATE.final_platform_p}" } else { "????" };
        return f"Archipelago - Move rando\nGoal: get grass {ARCHIPELAGO_STATE.grass}/{ARCHIPELAGO_STATE.required_grass}
-> go to Platform {final_platform}

Abilities
Ledge Grab: {ledge_grab}
Wall Jump: {wall_jump}
Jumppads: {jumppads}
Swim: {swim}"
        }else{
            return "Unknown gamemode"
        }
        "X"
        
    },
    draw_hud_always: fn() {},
    on_new_game: fn() {
        Tas::set_kill_z(-6000.);
        log("[AP] on_new_game called");
        ARCHIPELAGO_STATE.started = 1;
        ARCHIPELAGO_STATE.triggered_clusters.clear();
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
        if ARCHIPELAGO_STATE.started == 0 {
            return;
        }
        if ARCHIPELAGO_STATE.started == 1 {
            archipelago_start();
            log("Archipelago started!");
        }
        
        if ARCHIPELAGO_STATE.gamemode == 1 {
            // log(f"[AP] Pressed {index.element_type} {index.element_index} in cluster {index.cluster_index}");
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                    Tas::archipelago_send_check(10020000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                    // log(f"Vanilla mode - sending button press {10020000 + (index.cluster_index + 1) * 100 + index.element_index + 1}");
                
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 3 {
            if index.element_type == ElementType::Platform {
                let loc_id = 10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1;
                if !platforms_with_buttons.contains(loc_id) && !ARCHIPELAGO_STATE.seeker_pressed_platforms.contains(loc_id) && !ARCHIPELAGO_STATE.seeker_extra_pressed.contains(loc_id) {
                    log(f"$Seeker Platform {index.cluster_index + 1}-{index.element_index + 1}");
                    ARCHIPELAGO_STATE.seeker_extra_pressed.push(loc_id);
                    Tas::archipelago_send_check(10030000 + ARCHIPELAGO_STATE.seeker_extra_pressed.len());
                }
            }
            return;
        }

        if ARCHIPELAGO_STATE.gamemode != 0 {
            return;
        }

        if index.element_type == ElementType::Platform {

            log(f"$Platform {index.cluster_index + 1}-{index.element_index + 1}");
            Tas::archipelago_send_check(10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
            ARCHIPELAGO_STATE.stepped_on_platforms.push(10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1);

            if index.cluster_index == ARCHIPELAGO_STATE.final_platform_c - 1 && index.element_index == ARCHIPELAGO_STATE.final_platform_p - 1 && ARCHIPELAGO_STATE.grass >= ARCHIPELAGO_STATE.required_grass {
                Tas::archipelago_goal();
                if !ARCHIPELAGO_STATE.has_goaled {
                    let loc = Location { x: 2625., y: -2250., z: 1357. };
                    Tas::set_location(loc);
                    ARCHIPELAGO_STATE.has_goaled = true;
                    Tas::archipelago_trigger_goal_animation();
                }
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
    ARCHIPELAGO_STATE.ap_connected = false;
};

fn archipelago_process_item(item_index: int, ignore_activate_and_deactivate: bool) {
    if ARCHIPELAGO_STATE.gamemode != 0 {
        // log("Vanilla mode - ignoring received item");
        return;
    }

    // log(f"Processing received item index {item_index}");
    if item_index == 9999990 {  // Ledge Grab
        log("Received Ledge Grab!");
        ARCHIPELAGO_STATE.ledge_grab += 1;
        Tas::archipelago_set_wall_jump_and_ledge_grab(-1, 1, true);
    }
    if item_index == 9999991 {  // Wall Jump
        log("Received Wall Jump!");
        ARCHIPELAGO_STATE.wall_jump += 1;
        Tas::archipelago_set_wall_jump_and_ledge_grab(ARCHIPELAGO_STATE.wall_jump, -1, true);
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
        if !ARCHIPELAGO_STATE.triggered_clusters.contains(clusterindex) {
            if !ignore_activate_and_deactivate {
                Tas::archipelago_activate_all_buttons(-1);
            }
            
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
            
            if !ignore_activate_and_deactivate {
                Tas::archipelago_deactivate_all_buttons(-1);
            }
            

            if !ARCHIPELAGO_STATE.triggered_clusters.contains(clusterindex) {
                ARCHIPELAGO_STATE.triggered_clusters.push(clusterindex);
            }
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
        Tas::archipelago_set_wall_jump_and_ledge_grab(0, 0, true);
    }
    if item_index == 60000001{
        Tas::archipelago_set_wall_jump_and_ledge_grab(1, -1, true);
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

// triggers cluster clusterindex
fn archipelago_received_item(index: int, item_index: int){
    // log(f"Received item index {item_index} (cluster index {index})");
    if index <= ARCHIPELAGO_STATE.highest_index_received {
        log(f"Ignoring duplicate or out-of-order item index {index} (highest received: {ARCHIPELAGO_STATE.highest_index_received})");
        return;
    }else{
        if item_index < 10000000 {
            ARCHIPELAGO_STATE.highest_index_received = index;
        }
    }

    if item_index == 9999980 {  // Vanilla Minigame
        ARCHIPELAGO_STATE.unlock_vanilla_minigame = true;
        return;
    }
    if item_index == 9999970 {  // Vanilla Minigame
        ARCHIPELAGO_STATE.unlock_seeker_minigame = true;
        return;
    }

    ARCHIPELAGO_STATE.received_items.push(item_index);
    if ARCHIPELAGO_STATE.started < 2 {
        return;
    }
    archipelago_process_item(item_index, false);
    
}

fn archipelago_got_grass(){
    ARCHIPELAGO_STATE.grass += 1;
    log("Got grass!");
}

fn archipelago_init(gamemode: int){
    ARCHIPELAGO_STATE.ap_connected = true;
    log("Archipelago started, waiting for new game");
    ARCHIPELAGO_STATE.started = 0;
    ARCHIPELAGO_STATE.gamemode = gamemode;
    // probably want to set speed to 0 here
}

fn archipelago_start(){
    if ARCHIPELAGO_STATE.gamemode == 0 {
        log("Starting Move Rando gamemode");
        archipelago_main_start();
    }
    if ARCHIPELAGO_STATE.gamemode == 1 {
        log("Starting Vanilla gamemode");
        archipelago_vanilla_start();
    }
    if ARCHIPELAGO_STATE.gamemode == 2 {
        log("Starting Original Randomizer gamemode");
        // original randomizer start
    }
    if ARCHIPELAGO_STATE.gamemode == 3 {
        log("Starting Seeker gamemode");
        archipelago_seeker_start();
    }
}

fn archipelago_main_start(){
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.grass = 0;
    ARCHIPELAGO_STATE.wall_jump = 0;
    ARCHIPELAGO_STATE.ledge_grab = 0;
    ARCHIPELAGO_STATE.swim = 0;
    ARCHIPELAGO_STATE.jumppads = 0;
    
    ARCHIPELAGO_STATE.triggered_clusters = List::new();
    ARCHIPELAGO_STATE.has_goaled = false;

    Tas::set_kill_z(-60.);
    Tas::archipelago_set_wall_jump_and_ledge_grab(0, 0, true);
    Tas::archipelago_set_jump_pads(0);

    archipelago_activate_stepped_on_platforms();

    ARCHIPELAGO_STATE.started = 2;

    Tas::archipelago_activate_all_buttons(-1);
    for item in ARCHIPELAGO_STATE.received_items {
        archipelago_process_item(item, true);
    }
    Tas::archipelago_deactivate_all_buttons(-1);

}

fn archipelago_vanilla_start(){
    Tas::set_kill_z(-6000.);
    Tas::archipelago_set_wall_jump_and_ledge_grab(2, 1, false);
    Tas::archipelago_set_jump_pads(1);
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.started = 2;
}

fn archipelago_seeker_start(){
    Tas::set_kill_z(-6000.);
    Tas::archipelago_set_wall_jump_and_ledge_grab(2, 1, false);
    Tas::archipelago_set_jump_pads(1);
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.started = 2;

    // for loop
    let list = List::of(2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31);

    for i in list {
        Tas::set_level(i - 2);
        let last_unlocked = i - 1;
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
    }

    for p in ARCHIPELAGO_STATE.seeker_pressed_platforms{
        let q = p;
        let cluster = (q - 10010000) / 100;
        let plat = (q - 10010000) % 100;
        Tas::trigger_element(ElementIndex { cluster_index: cluster-1, element_type: ElementType::Platform, element_index: plat-1 });
    }
    for p in ARCHIPELAGO_STATE.seeker_extra_pressed{
        let q = p;
        let cluster = (q - 10010000) / 100;
        let plat = (q - 10010000) % 100;
        Tas::trigger_element(ElementIndex { cluster_index: cluster-1, element_type: ElementType::Platform, element_index: plat-1 });
    }

}

fn archipelago_checked_location(id: int){
    if id >= 10010000 && id < 10020000 {
        if ARCHIPELAGO_STATE.stepped_on_platforms.contains(id) {
            return;
        }
        ARCHIPELAGO_STATE.stepped_on_platforms.push(id);
    }
}

fn archipelago_activate_stepped_on_platforms(){
    for id in ARCHIPELAGO_STATE.stepped_on_platforms {
        let cluster = (id - 10010000) / 100;
        let plat = (id - 10010000) % 100;

        Tas::trigger_element(ElementIndex { cluster_index: cluster-1, element_type: ElementType::Platform, element_index: plat-1 });
    }
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

    if key == "seeker_pressed_platforms" {
        ARCHIPELAGO_STATE.seeker_pressed_platforms = List::new();
        for p in value.slice(1, -1).split(",") {
            ARCHIPELAGO_STATE.seeker_pressed_platforms.push(p.parse_int().unwrap());
        }
    }
}

fn create_archipelago_gamemodes_menu() -> Ui {
    Ui::new("Archipelago:", List::of(
        UiElement::Button(UiButton {
            label: Text { text: "Move rando (main)" },
            onclick: fn(label: Text) {
                log("Set gamemode to main game");
                archipelago_init(0);
                leave_ui();
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: {
                if ARCHIPELAGO_STATE.unlock_vanilla_minigame {
                    "Vanilla game"
                } else {
                    "Vanilla game (locked)"
                }
            } },
            onclick: fn(label: Text) {
                if !ARCHIPELAGO_STATE.unlock_vanilla_minigame {
                    log("Vanilla game gamemode is locked!");
                    return;
                }
                log("Set gamemode to OG game");
                archipelago_init(1);
                leave_ui();
            },
        }),
//        UiElement::Button(UiButton {
//            label: Text { text: "Original randomizer" },
//            onclick: fn(label: Text) {
//                log("Set gamemode to OG game");
//                archipelago_init(2);
//                leave_ui();
//            },
//        }),
        UiElement::Button(UiButton {
            label: Text { text: {
                if ARCHIPELAGO_STATE.unlock_seeker_minigame {
                    "Seeker"
                } else {
                    "Seeker (locked)"
                }
            } },
            onclick: fn(label: Text) {
                if !ARCHIPELAGO_STATE.unlock_seeker_minigame {
                    log("Seeker gamemode is locked!");
                    return;
                }
                log("Set gamemode to Seeker");
                archipelago_init(3);
                leave_ui();
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: "Back" },
            onclick: fn(label: Text) { leave_ui(); },
        }),
        
    ))
}