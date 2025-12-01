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
        elements.push(UiElement::Chooser(Chooser {
            label: Text { text: "Display Style" },
            options: List::of(
                Text { text: "Classic (shown on main menu)" },
                Text { text: "Color Coded" }
            ),
            selected: match SETTINGS.archipelago_display_style {
                ArchipelagoDisplayStyle::Classic => 0,
                ArchipelagoDisplayStyle::ColorCoded => 1
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_display_style = ArchipelagoDisplayStyle::Classic },
                    1 => { SETTINGS.archipelago_display_style = ArchipelagoDisplayStyle::ColorCoded },
                    _ => panic(f"unknown display_style index {index}"),
                }
                SETTINGS.store();
            }
        }));
        elements.push(UiElement::Chooser(Chooser {
            label: Text { text: "Position" },
            options: List::of(
                Text { text: "Top Left" },
                Text { text: "Top Center" },
                Text { text: "Top Right" },
                Text { text: "Center Right" },
                Text { text: "Bottom Right" },
                Text { text: "Bottom Center" },
                Text { text: "Bottom Left" },
                Text { text: "Center Left" }
            ),
            selected: match SETTINGS.archipelago_display_position {
                MinimapPosition::TopLeft => 0,
                MinimapPosition::TopCenter => 1,
                MinimapPosition::TopRight => 2,
                MinimapPosition::CenterRight => 3,
                MinimapPosition::BottomRight => 4,
                MinimapPosition::BottomCenter => 5,
                MinimapPosition::BottomLeft => 6,
                MinimapPosition::CenterLeft => 7,
                pos => panic(f"unknown archipelago display position: {pos}"),
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_display_position = MinimapPosition::TopLeft; },
                    1 => { SETTINGS.archipelago_display_position = MinimapPosition::TopCenter; },
                    2 => { SETTINGS.archipelago_display_position = MinimapPosition::TopRight; },
                    3 => { SETTINGS.archipelago_display_position = MinimapPosition::CenterRight; },
                    4 => { SETTINGS.archipelago_display_position = MinimapPosition::BottomRight; },
                    5 => { SETTINGS.archipelago_display_position = MinimapPosition::BottomCenter; },
                    6 => { SETTINGS.archipelago_display_position = MinimapPosition::BottomLeft; },
                    7 => { SETTINGS.archipelago_display_position = MinimapPosition::CenterLeft; },
                    pos => panic(f"unknown archipelago display position: {pos}"),
                }
                SETTINGS.store();
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

enum ArchipelagoDisplayStyle {
    Classic,
    ColorCoded
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

static AP_COLOR_RED    = Color { red: 0.600, green: 0.160, blue: 0.227, alpha: 1. };
static AP_COLOR_CYAN   = Color { red: 0.000, green: 0.627, blue: 0.698, alpha: 1. };
static AP_COLOR_GREEN  = Color { red: 0.231, green: 0.600, blue: 0.165, alpha: 1. };
static AP_COLOR_YELLOW = Color { red: 0.600, green: 0.533, blue: 0.165, alpha: 1. };
static AP_COLOR_BG     = Color { red: 0., green: 0., blue: 0., alpha: 0.6 };

struct ColorfulText {
    text: string,
    color: Color
}

fn get_status_text_lines() -> List<ColorfulText> {
    match ARCHIPELAGO_STATE.started {
        0 => List::of(
            ColorfulText { text: "Archipelago Randomizer", color: COLOR_WHITE },
            ColorfulText { text: "Press new game (in Refunct menu).", color: AP_COLOR_CYAN },
        ),
        1 => List::of(
            ColorfulText { text: "Archipelago Randomizer", color: COLOR_WHITE },
            ColorfulText { text: "Touch a platform to start!", color: AP_COLOR_CYAN },
        ),
        _ => match ARCHIPELAGO_STATE.gamemode {
            0 => get_move_rando_status_lines(),
            1 => List::of(
                ColorfulText { text: "Archipelago - Vanilla", color: COLOR_WHITE },
                ColorfulText { text: "Press the buttons!", color: AP_COLOR_CYAN },
            ),
            // 2 => List::of(
            //     ColorfulText { text: "Archipelago - Original Rando", color: COLOR_WHITE },
            //     ColorfulText { text: "Touch a platform to start!", color: AP_COLOR_CYAN },
            // ),
            3 => List::of(
                ColorfulText { text: "Archipelago - Seeker", color: COLOR_WHITE },
                ColorfulText { text: "Find the empty platforms!", color: AP_COLOR_CYAN },
            ),
            _ => List::of(
                ColorfulText { text: "Archipelago", color: COLOR_WHITE },
                ColorfulText { text: "Unknown Gamemode", color: AP_COLOR_RED },
            ),
        }
    }
}

fn get_move_rando_status_lines() -> List<ColorfulText> {
    let final_platform = if ARCHIPELAGO_STATE.final_platform_known { f"{ARCHIPELAGO_STATE.final_platform_c}-{ARCHIPELAGO_STATE.final_platform_p}" } else { "??-??" };
    let ledge_grab_state = if ARCHIPELAGO_STATE.ledge_grab > 0 { "YES" } else { "NO" };
    let wall_jump_state = if ARCHIPELAGO_STATE.wall_jump >= 2 { "INF" } else if ARCHIPELAGO_STATE.wall_jump == 1 { "ONE" } else { "NO" };
    let jump_pad_state = if ARCHIPELAGO_STATE.jumppads > 0 { "YES" } else { "NO" };
    let swim_state = if ARCHIPELAGO_STATE.swim > 0 { "YES" } else { "NO" };

    List::of(
        ColorfulText { text: "Archipelago - Move Rando", color: COLOR_WHITE },
        ColorfulText { text: "Goals", color: COLOR_WHITE },
        ColorfulText {
            text:  f"Get Grass: {ARCHIPELAGO_STATE.grass}/{ARCHIPELAGO_STATE.required_grass}",
            color: if ARCHIPELAGO_STATE.grass >= ARCHIPELAGO_STATE.required_grass { AP_COLOR_GREEN } else { AP_COLOR_CYAN }
        },
        ColorfulText {
            text:  f"Go to Platform {final_platform}",
            color: if ARCHIPELAGO_STATE.has_goaled { AP_COLOR_GREEN } else { AP_COLOR_CYAN }
        },
        ColorfulText { text: "", color: COLOR_WHITE },
        ColorfulText { text: "Abilities", color: COLOR_WHITE },
        ColorfulText {
            text:  f"Ledge Grab: { ledge_grab_state }",
            color: if ARCHIPELAGO_STATE.ledge_grab > 0 { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Wall Jump:  { wall_jump_state }",
            color: if ARCHIPELAGO_STATE.wall_jump >= 2 { AP_COLOR_GREEN } else if ARCHIPELAGO_STATE.wall_jump == 1 { AP_COLOR_YELLOW } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Jump Pads:  { jump_pad_state }",
            color: if ARCHIPELAGO_STATE.jumppads > 0 { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Swim:       { swim_state }",
            color: if ARCHIPELAGO_STATE.swim > 0 { AP_COLOR_GREEN } else { AP_COLOR_RED }
        }
    )
}

static mut ARCHIPELAGO_COMPONENT = Component {
    id: ARCHIPELAGO_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: update_players,
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string {
        if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::Classic { return text; }

        let mut hud_text = "";
        for line in get_status_text_lines() {
            let text = line.text;
            hud_text = f"{hud_text}{text}\n";
        }
        hud_text
    },
    draw_hud_always: fn() {
        if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::ColorCoded { return; }

        let text_lines = get_status_text_lines();

        // Then, draw the lines
        let mut text_width = 0.0;
        let mut line_height = 0.0;
        for text_line in text_lines {
            let text_size = Tas::get_text_size(text_line.text, SETTINGS.ui_scale);
            text_width = float::max(text_size.width, text_width);
            line_height = text_size.height;
        }
        let text_height = text_lines.len().to_float() * line_height;

        let viewport = Tas::get_viewport_size();
        let mut title_text_x = 0.0;
        let mut title_text_y = 0.0;

        match SETTINGS.archipelago_display_position {
            MinimapPosition::TopLeft => {
                title_text_x = 5.0;
                title_text_y = 0.0;

                // Make sure we don't overlap with the menu
                match UI_STACK.last() {
                    Option::Some(ui) => {
                        let elements = ui.elements;
                        title_text_y += line_height * (elements.len().to_float() + 1.5);
                    },
                    Option::None => (),
                }
            },
            MinimapPosition::TopCenter => {
                title_text_x = (viewport.width.to_float() - text_width)/2.0;
                title_text_y = 5.0;
            },
            MinimapPosition::TopRight => {
                title_text_x = viewport.width.to_float() - text_width - 5.0;
                title_text_y = 5.0;
            },
            MinimapPosition::CenterRight => {
                title_text_x = viewport.width.to_float() - text_width - 5.0;
                title_text_y = (viewport.height.to_float() - text_height)/2.0;
            },
            MinimapPosition::BottomRight => {
                title_text_x = viewport.width.to_float() - text_width - 5.0;
                title_text_y = viewport.height.to_float() - text_height - 5.0;
            },
            MinimapPosition::BottomCenter => {
                title_text_x = (viewport.width.to_float() - text_width)/2.0;
                title_text_y = viewport.height.to_float() - text_height - 5.0;
            },
            MinimapPosition::BottomLeft => {
                title_text_x = 5.0;
                title_text_y = viewport.height.to_float() - text_height - 5.0;
            },
            MinimapPosition::CenterLeft => {
                title_text_x = 5.0;
                title_text_y = (viewport.height.to_float() - text_height)/2.0;
            },
            pos => panic(f"unknown/invalid archipelago display position: {pos}"),
        };

        // Draw background rectangle for visibility
        Tas::draw_rect(AP_COLOR_BG, title_text_x - 5.0, title_text_y - 5.0, text_width + 10.0, text_height + 10.0);

        let mut i = 0.0;
        for text_line in text_lines {
            Tas::draw_text(DrawText {
                text: text_line.text, color: text_line.color,
                x: title_text_x, y: title_text_y + i*line_height,
                scale: SETTINGS.ui_scale, scale_position: false
            });
            i += 1.0;
        }
    },
    on_new_game: fn() {
        Tas::set_kill_z(-6000.);
        // log("[AP] on_new_game called");
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
            // log("Archipelago started!");
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
                // log("Set gamemode to main game");
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
                    // log("Vanilla game gamemode is locked!");
                    return;
                }
                // log("Set gamemode to OG game");
                archipelago_init(1);
                leave_ui();
            },
        }),
//        UiElement::Button(UiButton {
//            label: Text { text: "Original randomizer" },
//            onclick: fn(label: Text) {
//                // log("Set gamemode to OG game");
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
                    // log("Seeker gamemode is locked!");
                    return;
                }
                // log("Set gamemode to Seeker");
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