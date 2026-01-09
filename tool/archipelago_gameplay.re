
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
    done_vanilla_minigame: bool,
    progress_vanilla_minigame: string,

    unlock_seeker_minigame: bool,
    done_seeker_minigame: bool,
    progress_seeker_minigame: string,
    seeker_pressed_platforms: List<int>,
    seeker_extra_pressed: List<int>,

    unlock_button_galore_minigame: bool,
    done_button_galore_minigame: bool,
    progress_button_galore_minigame: string,

    unlock_OG_randomizer: bool,
    done_OG_randomizer_minigame: bool,
    progress_OG_randomizer_minigame: string,
    og_randomizer_order: List<int>,
    og_randomizer_index: int,
    og_randomizer_need_another_new_game: bool,

    last_platform_c: Option<int>,
    last_platform_p: Option<int>,
    checked_locations: List<int>,
    mod_version: string,
    apworld_version: string,

    triggering_clusters: List<int>,
    triggering_clusters_counter: int,
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
        done_vanilla_minigame: false,
        progress_vanilla_minigame: "0/37",

        unlock_seeker_minigame: false,
        done_seeker_minigame: false,
        progress_seeker_minigame: "0/10",
        seeker_pressed_platforms: List::new(),
        seeker_extra_pressed: List::new(),

        unlock_button_galore_minigame: false,
        done_button_galore_minigame: false,
        progress_button_galore_minigame: "0/37",

        unlock_OG_randomizer: false,
        done_OG_randomizer_minigame: false,
        progress_OG_randomizer_minigame: "0/37",
        og_randomizer_index: -1,
        og_randomizer_order: List::new(),
        og_randomizer_need_another_new_game: false,

        last_platform_c: Option::None,
        last_platform_p: Option::None,
        checked_locations: List::new(),
        mod_version: "0.3.2",
        apworld_version: "",

        triggering_clusters: List::new(),
        triggering_clusters_counter: 0,
    }
}

static mut ARCHIPELAGO_STATE = fresh_archipelago_state();

static platforms_with_buttons = List::of(10010101,10010203,10010302,10010404,10010501,10010601,10010701,10010702,10010811,10010906,10011001,10011003,10011108,10011201,10011301,10011403,10011502,10011601,10011702,10011802,10011801,10011905,10012007,10012108,10012202,10012303,10012401,10012505,10012612,10012607,10012609,10012708,10012818,10012808,10012904,10013012,10013102);

static mut ARCHIPELAGO_COMPONENT = Component {
    id: ARCHIPELAGO_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: archipelago_hud_text,
    draw_hud_always: archipelago_hud_color_coded,
    on_new_game: fn() {
        if ARCHIPELAGO_STATE.gamemode == 4 {
            ARCHIPELAGO_STATE.og_randomizer_need_another_new_game = false;
            ARCHIPELAGO_STATE.started = 0;
            Tas::set_level(0);
            ARCHIPELAGO_STATE.og_randomizer_index = -1;
        }

        Tas::set_kill_z(-6000.);
        // log("[AP] on_new_game called");
        ARCHIPELAGO_STATE.started = 1;
        ARCHIPELAGO_STATE.triggered_clusters.clear();

        ARCHIPELAGO_STATE.og_randomizer_index = -1;
    },
    on_level_change: ap_on_level_change_function,
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
        if index.element_type == ElementType::Platform {
            ARCHIPELAGO_STATE.last_platform_c = Option::Some(index.cluster_index + 1);
            ARCHIPELAGO_STATE.last_platform_p = Option::Some(index.element_index + 1);
        }

        if ARCHIPELAGO_STATE.started == 0 {
            return;
        }
        if ARCHIPELAGO_STATE.started == 1 {
            archipelago_start();
            // log("Archipelago started!");
            // no return on purpose to allow first press to count
        }

        if ARCHIPELAGO_STATE.gamemode == 0 {
            if index.element_type == ElementType::Platform {

                // log(f"$Platform {index.cluster_index + 1}-{index.element_index + 1}");
                Tas::archipelago_send_check(10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1);

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
        }

        if ARCHIPELAGO_STATE.gamemode == 1 {
            // log(f"[AP] Pressed {index.element_type} {index.element_index} in cluster {index.cluster_index}");
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                Tas::archipelago_send_check(10020000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                // log(f"Vanilla mode - sending button press {10020000 + (index.cluster_index + 1) * 100 + index.element_index + 1}");
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 2 {
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                Tas::archipelago_send_check(10040000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                // log(f"Vanilla mode - sending button press {10040000 + (index.cluster_index + 1) * 100 + index.element_index + 1}");
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 3 {
            if index.element_type == ElementType::Platform {
                let loc_id = 10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1;
                if !platforms_with_buttons.contains(loc_id) && !ARCHIPELAGO_STATE.seeker_pressed_platforms.contains(loc_id) && !ARCHIPELAGO_STATE.seeker_extra_pressed.contains(loc_id) {
                    // log(f"$Seeker Platform {index.cluster_index + 1}-{index.element_index + 1}");
                    ARCHIPELAGO_STATE.seeker_extra_pressed.push(loc_id);
                    Tas::archipelago_send_check(10030000 + ARCHIPELAGO_STATE.seeker_extra_pressed.len());
                }
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 4 {
            if ARCHIPELAGO_STATE.og_randomizer_index == -1 {
                ap_OG_randomizer_change_level();
            }
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                Tas::archipelago_send_check(10050000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                // log(f"Vanilla mode - sending button press {10050000 + (index.cluster_index + 1) * 100 + index.element_index + 1}");
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
    ap_log_error("Disconnected from Archipelago server");
    remove_component(AP_LOG_COMPONENT);
    remove_component(ARCHIPELAGO_COMPONENT);
    ARCHIPELAGO_STATE.ap_connected = false;
};

fn archipelago_process_item(item_id: int, starting_index: int, item_index: int) {
    if ARCHIPELAGO_STATE.gamemode == 0 {
        // log(f"Processing received item index {item_id}");
        if item_id == 9999990 {  // Ledge Grab
            // log("Received Ledge Grab!");
            ARCHIPELAGO_STATE.ledge_grab += 1;
            Tas::archipelago_set_wall_jump_and_ledge_grab(-1, 1, true);
        }
        if item_id == 9999991 {  // Wall Jump
            // log("Received Wall Jump!");
            ARCHIPELAGO_STATE.wall_jump += 1;
            Tas::archipelago_set_wall_jump_and_ledge_grab(ARCHIPELAGO_STATE.wall_jump, -1, true);
        }
        if item_id == 9999992 {  // Swim
            // log("Received Swim!");
            ARCHIPELAGO_STATE.swim += 1;
            Tas::set_kill_z(-6000.);
        }
        if item_id == 9999993 {  // Jumppads
            // log("Received Jumppads!");
            ARCHIPELAGO_STATE.jumppads += 1;
            Tas::archipelago_set_jump_pads(1);
        }
        if item_id == 9999997 {  // Final Platform Known
            ARCHIPELAGO_STATE.final_platform_known = true;
        }
        if item_id == 9999999 {  // Grass
            archipelago_got_grass();
        }
        let clusterindex = item_id - 10000000;
        if clusterindex >= 2 && clusterindex < 32 {
            if !ARCHIPELAGO_STATE.triggered_clusters.contains(clusterindex) {
                ARCHIPELAGO_STATE.triggering_clusters.push(clusterindex);
            }
        }
    }

    // DEBUG items:
    if item_id >= 20000000 && item_id < 30000000 {
        log(f"DEBUG set_level {item_id - 20000000}");
        Tas::set_level(item_id - 20000000);
    }
    if item_id >= 30000000 && item_id < 40000000{
        log(f"DEBUG trigger_element {item_id - 30000000} Button 0");
        Tas::trigger_element_by_type(item_id - 30000000, "Button", 0);
    }
    if item_id == 60000000{
        Tas::archipelago_set_wall_jump_and_ledge_grab(0, 0, true);
    }
    if item_id == 60000001{
        Tas::archipelago_set_wall_jump_and_ledge_grab(1, -1, true);
    }
    if item_id == 60000005{
        Tas::set_kill_z(-60.);
    }
    if item_id == 60000010{
        Tas::archipelago_set_jump_pads(0);
    }
    if item_id == 60000015{
        Tas::archipelago_trigger_goal_animation();
    }
}

fn archipelago_trigger_one_cluster_now(){
    if ARCHIPELAGO_STATE.triggering_clusters.len() == 0 {
        return;
    }
    ARCHIPELAGO_STATE.triggering_clusters_counter += 1;
    if ARCHIPELAGO_STATE.triggering_clusters_counter % 50 != 0 {
        return;
    }

    let c = ARCHIPELAGO_STATE.triggering_clusters.get(0).unwrap();
    ARCHIPELAGO_STATE.triggering_clusters.remove(0);
    
    let last_unlocked = ARCHIPELAGO_STATE.last_level_unlocked;
    Tas::archipelago_raise_cluster(c - 2, last_unlocked - 1);

    ARCHIPELAGO_STATE.last_level_unlocked = c;

    if !ARCHIPELAGO_STATE.triggered_clusters.contains(c) {
        ARCHIPELAGO_STATE.triggered_clusters.push(c);
    }
}

// triggers cluster clusterindex
fn archipelago_received_item(index: int, item_id: int, starting_index: int) {
    // log(f"Received item index {item_id} (cluster index {index})");
    if index <= ARCHIPELAGO_STATE.highest_index_received {
        // log(f"Ignoring duplicate or out-of-order item index {index} (highest received: {ARCHIPELAGO_STATE.highest_index_received})");
        return;
    }else{
        if item_id < 10000000 {
            ARCHIPELAGO_STATE.highest_index_received = index;
        }
    }

    if item_id == 9999980 {  // Vanilla Minigame
        ARCHIPELAGO_STATE.unlock_vanilla_minigame = true;
        return;
    }
    if item_id == 9999970 {  // Seeker Minigame
        ARCHIPELAGO_STATE.unlock_seeker_minigame = true;
        return;
    }
    if item_id == 9999960 {  // Button Galore Minigame
        ARCHIPELAGO_STATE.unlock_button_galore_minigame = true;
        return;
    }
    if item_id == 9999950 {  // OG Randomizer Minigame
        ARCHIPELAGO_STATE.unlock_OG_randomizer = true;
        return;
    }

    ARCHIPELAGO_STATE.received_items.push(item_id);
    if ARCHIPELAGO_STATE.started < 2 {
        return;
    }
    archipelago_process_item(item_id, starting_index, index);
}

fn archipelago_got_grass(){
    ARCHIPELAGO_STATE.grass += 1;
    // log("Got grass!");
}

fn archipelago_init(gamemode: int){

    ARCHIPELAGO_STATE.ap_connected = true;
    // log("Archipelago started, waiting for new game");
    ARCHIPELAGO_STATE.started = 0;
    ARCHIPELAGO_STATE.gamemode = gamemode;
    ARCHIPELAGO_STATE.triggering_clusters.clear();

    if gamemode == 2 {
        Tas::set_level(30);
        // log("Setting level to 30 for Button Galore gamemode");
    }

    Tas::archipelago_gather_all_buttons();
}

fn archipelago_start(){
    if ARCHIPELAGO_STATE.gamemode == 0 {
        // log("Starting Move Rando gamemode");
        archipelago_main_start();
    }
    if ARCHIPELAGO_STATE.gamemode == 1 {
        // log("Starting Vanilla gamemode");
        archipelago_vanilla_start();
    }
    if ARCHIPELAGO_STATE.gamemode == 2 {
        // log("Starting Button Galore gamemode");
        archipelago_button_galore_start();
    }
    if ARCHIPELAGO_STATE.gamemode == 3 {
        // log("Starting Seeker gamemode");
        archipelago_seeker_start();
    }
    if ARCHIPELAGO_STATE.gamemode == 4 {
        // log("Starting OG Randomizer gamemode");
        archipelago_og_randomizer_start();
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
    let mut i = 0;
    for item in ARCHIPELAGO_STATE.received_items {
        archipelago_process_item(item, 0, i);
        i += 1;
    }
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
        ARCHIPELAGO_STATE.triggering_clusters.push(i);
    }

    for p in ARCHIPELAGO_STATE.seeker_pressed_platforms{
        let q = p;
        let cluster = (q - 10010000) / 100;
        let plat = (q - 10010000) % 100;
        Tas::trigger_element_by_type(cluster-1, "Platform", plat-1);
    }
    for p in ARCHIPELAGO_STATE.seeker_extra_pressed{
        let q = p;
        let cluster = (q - 10010000) / 100;
        let plat = (q - 10010000) % 100;
        Tas::trigger_element_by_type(cluster-1, "Platform", plat-1);
    }
}

fn archipelago_button_galore_start(){
    Tas::set_kill_z(-6000.);
    Tas::archipelago_set_wall_jump_and_ledge_grab(2, 1, false);
    Tas::archipelago_set_jump_pads(1);
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.started = 2;
}

fn archipelago_og_randomizer_start(){
    Tas::set_level(0);
    Tas::set_kill_z(-6000.);
    Tas::archipelago_set_wall_jump_and_ledge_grab(2, 1, false);
    Tas::archipelago_set_jump_pads(1);
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.started = 2;
}

fn ap_on_level_change_function(old: int, new: int) {
    if ARCHIPELAGO_STATE.gamemode == 2 {
        // log(f"[AP] on_level_change: {old} -> {new}");
        Tas::set_level(30);
    }
    if ARCHIPELAGO_STATE.gamemode == 4 && ARCHIPELAGO_STATE.started == 2 {
        ap_OG_randomizer_change_level();
    }
}

fn ap_OG_randomizer_change_level(){
    // log(f"[AP] OG Randomizer changing level from index {ARCHIPELAGO_STATE.og_randomizer_index}");
    ARCHIPELAGO_STATE.og_randomizer_index += 1;
    if ARCHIPELAGO_STATE.og_randomizer_index < ARCHIPELAGO_STATE.og_randomizer_order.len() {
        let next_level = ARCHIPELAGO_STATE.og_randomizer_order.get(ARCHIPELAGO_STATE.og_randomizer_index).unwrap();
        Tas::set_level(next_level - 2);
    }
}

fn archipelago_checked_location(id: int){
    if ARCHIPELAGO_STATE.checked_locations.contains(id) {
        return;
    }
    ARCHIPELAGO_STATE.checked_locations.push(id);
    if id >= 10010000 && id < 10020000 {
        ARCHIPELAGO_STATE.stepped_on_platforms.push(id);
    }
    let vanilla_locations = List::of(10020101, 10020201, 10020301, 10020401, 10020501, 10020601, 10020701, 10020702, 10020801, 10020901, 10021001, 10021002, 10021101, 10021201, 10021301, 10021401, 10021501, 10021601, 10021701, 10021801, 10021802, 10021901, 10022001, 10022101, 10022201, 10022301, 10022401, 10022501, 10022601, 10022602, 10022603, 10022701, 10022801, 10022802, 10022901, 10023001, 10023101);
    if vanilla_locations.contains(id) {
        let mut number_pressed = 0;
        for lid in vanilla_locations {
            if ARCHIPELAGO_STATE.checked_locations.contains(lid) {
                number_pressed += 1;
            }
        }
        if number_pressed == vanilla_locations.len() {
            ARCHIPELAGO_STATE.done_vanilla_minigame = true;
            ap_log(List::of(ColorfulText { text:"Completed Vanilla Minigame!", color: AP_COLOR_GREEN }));
        }
        ARCHIPELAGO_STATE.progress_vanilla_minigame = f"{number_pressed}/{vanilla_locations.len()}";
    }
    let seeker_locations = List::of(10030001, 10030002, 10030003, 10030004, 10030005, 10030006, 10030007, 10030008, 10030009, 10030010);
    if seeker_locations.contains(id) {
        let mut number_pressed = 0;
        for lid in seeker_locations {
            if ARCHIPELAGO_STATE.checked_locations.contains(lid) {
                number_pressed += 1;
            }
        }
        if number_pressed == seeker_locations.len() {
            ARCHIPELAGO_STATE.done_seeker_minigame = true;
            ap_log(List::of(ColorfulText { text:"Completed Seeker Minigame!", color: AP_COLOR_GREEN }));
        }
        ARCHIPELAGO_STATE.progress_seeker_minigame = f"{number_pressed}/{seeker_locations.len()}";
    }
    let button_galore_locations = List::of(10040101, 10040201, 10040301, 10040401, 10040501, 10040601, 10040701, 10040702, 10040801, 10040901, 10041001, 10041002, 10041101, 10041201, 10041301, 10041401, 10041501, 10041601, 10041701, 10041801, 10041802, 10041901, 10042001, 10042101, 10042201, 10042301, 10042401, 10042501, 10042601, 10042602, 10042603, 10042701, 10042801, 10042802, 10042901, 10043001, 10043101);
    if button_galore_locations.contains(id) {
        let mut number_pressed = 0;
        for lid in button_galore_locations {
            if ARCHIPELAGO_STATE.checked_locations.contains(lid) {
                number_pressed += 1;
            }else{
                // log(f"Button Galore - still need to press location {lid}");
            }
        }
        if number_pressed == button_galore_locations.len() {
            ARCHIPELAGO_STATE.done_button_galore_minigame = true;
            // log("Completed Button Galore Minigame!");
        }
        ARCHIPELAGO_STATE.progress_button_galore_minigame = f"{number_pressed}/{button_galore_locations.len()}";
    }
    let og_randomizer_locations = List::of(10050101, 10050201, 10050301, 10050401, 10050501, 10050601, 10050701, 10050702, 10050801, 10050901, 10051001, 10051002, 10051101, 10051201, 10051301, 10051401, 10051501, 10051601, 10051701, 10051801, 10051802, 10051901, 10052001, 10052101, 10052201, 10052301, 10052401, 10052501, 10052601, 10052602, 10052603, 10052701, 10052801, 10052802, 10052901, 10053001, 10053101);
    if og_randomizer_locations.contains(id) {
        let mut number_pressed = 0;
        for lid in og_randomizer_locations {
            if ARCHIPELAGO_STATE.checked_locations.contains(lid) {
                number_pressed += 1;
            }
        }
        if number_pressed == og_randomizer_locations.len() {
            ARCHIPELAGO_STATE.done_OG_randomizer_minigame = true;
            ap_log(List::of(ColorfulText { text:"Completed OG Randomizer Minigame!", color: AP_COLOR_GREEN }));
        }
        ARCHIPELAGO_STATE.progress_OG_randomizer_minigame = f"{number_pressed}/{og_randomizer_locations.len()}";
    }
}

fn archipelago_activate_stepped_on_platforms(){
    for id in ARCHIPELAGO_STATE.stepped_on_platforms {
        let cluster = (id - 10010000) / 100;
        let plat = (id - 10010000) % 100;

        Tas::trigger_element_by_type(cluster-1, "Platform", plat-1);
    }
}

fn archipelago_received_slot_data(key: string, value: string){
    if key == "ap_world_version" {
        ARCHIPELAGO_STATE.apworld_version = value.slice(1, -1);
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

    if key == "og_randomizer_order" {
        ARCHIPELAGO_STATE.og_randomizer_order = List::new();
        for p in value.slice(1, -1).split(",") {
            ARCHIPELAGO_STATE.og_randomizer_order.push(p.parse_int().unwrap());
        }
    }

    if key == "death_link" && value == "1" {
        Tas::set_death_link(true);
    }
}
