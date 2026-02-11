
struct ArchipelagoState {
    ap_connected: bool,

    last_level_unlocked: int,
    grass: int,
    wall_jump: int,
    ledge_grab: bool,
    swim: bool,
    jump_pads: bool,
    pipes: bool,
    lifts: bool,
    required_grass: int,
    final_platform_c: int,
    final_platform_p: int,
    final_platform_known: bool,
    received_items: List<int>,
    started: int,
    triggered_clusters: List<int>,
    stepped_on_platforms: List<int>,
    collected_cubes: List<int>,
    cubes_options: int,
    extra_cubes_options: int,
    
    highest_index_received: int,
    has_goaled: bool,
    gamemode: int,

    red_cubes_bag: bool,
    green_cubes_bag: bool,
    blue_cubes_bag: bool,

    extra_cubes_locs: List<int>,
    extra_cubes_int_ids: List<int>,

    unlock_vanilla_minigame: bool,
    done_vanilla_minigame: bool,
    progress_vanilla_minigame: string,

    unlock_seeker_minigame: bool,
    done_seeker_minigame: bool,
    progress_seeker_minigame: string,
    seeker_pressed_platforms: List<int>,
    seeker_extra_pressed: List<int>,
    seeker_done_triggering: int,

    unlock_button_galore_minigame: bool,
    done_button_galore_minigame: bool,
    progress_button_galore_minigame: string,

    unlock_OG_randomizer: bool,
    done_OG_randomizer_minigame: bool,
    progress_OG_randomizer_minigame: string,
    og_randomizer_order: List<int>,
    og_randomizer_index: int,

    score_block_brawl_reds: int,
    score_block_brawl_blues: int,
    score_block_brawl_greens: int,
    score_block_brawl_yellows: int,
    in_logic_block_brawl_reds: int,
    in_logic_block_brawl_blues: int,
    in_logic_block_brawl_greens: int,
    in_logic_block_brawl_yellows: int,
    score_for_next_block: int,
    block_brawl_cubes_collected: int,
    block_brawl_cubes_total: int,
    unlock_block_brawl_reds: bool,
    unlock_block_brawl_blues: bool,
    unlock_block_brawl_greens: bool,
    unlock_block_brawl_yellows: bool,
    block_brawl_red_ids: List<int>,
    block_brawl_blue_ids: List<int>,
    block_brawl_green_ids: List<int>,
    block_brawl_yellow_ids: List<int>,
    done_block_brawl_minigame: bool,
    block_brawl_check_in_logic: int,

    last_platform_c: Option<int>,
    last_platform_p: Option<int>,
    checked_locations: List<int>,
    mod_version: string,
    apworld_version: string,

    triggering_clusters: List<int>,
    last_tick_time: int,
}

fn fresh_archipelago_state() -> ArchipelagoState {
    ArchipelagoState {
        ap_connected: false,

        last_level_unlocked: 1,
        grass: -10000,
        wall_jump: 0,
        ledge_grab: false,
        swim: false,
        jump_pads: false,
        pipes: false,
        lifts: false,
        required_grass: 1000,
        final_platform_c: 100,
        final_platform_p: 1,
        final_platform_known: false,
        received_items: List::new(),
        started: 0,
        triggered_clusters: List::new(),
        stepped_on_platforms: List::new(),
        collected_cubes: List::new(),
        cubes_options: -1,
        extra_cubes_options: -1,
        
        highest_index_received: -1,
        has_goaled: false,
        gamemode: 0,

        red_cubes_bag: false,
        green_cubes_bag: false,
        blue_cubes_bag: false,

        extra_cubes_locs: List::new(),
        extra_cubes_int_ids: List::new(),

        unlock_vanilla_minigame: false,
        done_vanilla_minigame: false,
        progress_vanilla_minigame: "0/37",

        unlock_seeker_minigame: false,
        done_seeker_minigame: false,
        progress_seeker_minigame: "0/10",
        seeker_pressed_platforms: List::new(),
        seeker_extra_pressed: List::new(),
        seeker_done_triggering: 0,

        unlock_button_galore_minigame: false,
        done_button_galore_minigame: false,
        progress_button_galore_minigame: "0/37",

        unlock_OG_randomizer: false,
        done_OG_randomizer_minigame: false,
        progress_OG_randomizer_minigame: "0/37",
        og_randomizer_index: -1,
        og_randomizer_order: List::new(),

        score_block_brawl_reds: 0,
        score_block_brawl_blues: 0,
        score_block_brawl_greens: 0,
        score_block_brawl_yellows: 0,
        in_logic_block_brawl_reds: 0,
        in_logic_block_brawl_blues: 0,
        in_logic_block_brawl_greens: 0,
        in_logic_block_brawl_yellows: 0,
        score_for_next_block: 1,
        block_brawl_cubes_collected: 0,
        block_brawl_cubes_total: 0,
        unlock_block_brawl_reds: false,
        unlock_block_brawl_blues: false,
        unlock_block_brawl_greens: false,
        unlock_block_brawl_yellows: false,
        block_brawl_red_ids: List::new(),
        block_brawl_blue_ids: List::new(),
        block_brawl_green_ids: List::new(),
        block_brawl_yellow_ids: List::new(),
        done_block_brawl_minigame: false,
        block_brawl_check_in_logic: 0,

        last_platform_c: Option::None,
        last_platform_p: Option::None,
        checked_locations: List::new(),
        mod_version: "0.8.2",
        apworld_version: "",

        triggering_clusters: List::new(),
        last_tick_time: 0,
    }
}

static mut ARCHIPELAGO_STATE = fresh_archipelago_state();

static platforms_with_buttons = List::of(10010101,10010203,10010302,10010404,10010501,10010601,10010701,10010702,10010811,10010906,10011001,10011003,10011108,10011201,10011301,10011403,10011502,10011601,10011702,10011802,10011801,10011905,10012007,10012108,10012202,10012303,10012401,10012505,10012612,10012607,10012609,10012708,10012818,10012808,10012904,10013012,10013102);

static mut ARCHIPELAGO_COMPONENT = Component {
    id: ARCHIPELAGO_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID, ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: archipelago_hud_text,
    draw_hud_always: archipelago_hud_color_coded,
    on_new_game: fn() {
        Tas::destroy_cubes(true, true);
        Tas::destroy_platforms(true, true);
        ARCHIPELAGO_STATE.extra_cubes_locs.clear();
        ARCHIPELAGO_STATE.extra_cubes_int_ids.clear();

        if ARCHIPELAGO_STATE.gamemode == 4 {
            ARCHIPELAGO_STATE.started = 0;
            Tas::set_level(0);
            ARCHIPELAGO_STATE.og_randomizer_index = -1;
        }

        Tas::abilities_set_swim(true);
        // log("[AP] on_new_game called");
        ARCHIPELAGO_STATE.started = 1;
        ARCHIPELAGO_STATE.triggered_clusters.clear();

        ARCHIPELAGO_STATE.og_randomizer_index = -1;

        update_block_brawl_in_logic_counts();
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
        if index.cluster_index == 9999 {
            got_cube_block_brawl(index.element_index);
            got_extra_cube_ap(index.element_index);
            return;
        }
        
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
                archipelago_send_check(10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1);

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

            if index.element_type == ElementType::Cube {
                /*
                In Vanilla:
                    Cube 4-1 is on Platform 4-2
                    Cube 7-1 is on Platform 7-3
                    Cube 8-1 is on Platform 8-10
                    Cube 8-2 is on Platform 8-4
                    Cube 9-1 is on Platform 9-8
                    Cube 10-1 is on Platform 10-2
                    Cube 12-1 is on Platform 12-3
                    Cube 13-1 is on Platform 13-3
                    Cube 14-1 is on Platform 14-2
                    Cube 18-1 is on Platform 18-3
                    Cube 21-1 is on Platform 21-5
                    Cube 21-2 is on Platform 21-9
                    Cube 23-1 is on Platform 23-2
                    Cube 27-1 is on Platform 27-1
                    Cube 28-1 is on Platform 28-11
                    Cube 29-1 is on Platform 29-2
                    Cube 29-2 is on Platform 29-9
                    Cube 30-1 is on Platform 30-11

                    @Spineraks this is here so that you can modify the .apworld easier :)
                    @Nielrenned thank you for this <3
                */
                if index.cluster_index == 9999 {
                    // We picked up a non-vanilla cube
                    // log(f"Picked up Spawned Cube (index: {index.element_index})");
                } else {
                    // We picked up a vanilla cube
                    // log(f"Picked up Vanilla Cube {index.cluster_index + 1}-{index.element_index + 1}");
                    archipelago_send_check(10060000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                }
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 1 {
            // log(f"[AP] Pressed {index.element_type} {index.element_index} in cluster {index.cluster_index}");
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                archipelago_send_check(10020000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                // log(f"Vanilla mode - sending button press {10020000 + (index.cluster_index + 1) * 100 + index.element_index + 1}");
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 2 {
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                archipelago_send_check(10040000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
                // log(f"Vanilla mode - sending button press {10040000 + (index.cluster_index + 1) * 100 + index.element_index + 1}");
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 3 {
            if index.element_type == ElementType::Platform {
                let loc_id = 10010000 + (index.cluster_index + 1) * 100 + index.element_index + 1;
                if !platforms_with_buttons.contains(loc_id) && !ARCHIPELAGO_STATE.seeker_pressed_platforms.contains(loc_id) && !ARCHIPELAGO_STATE.seeker_extra_pressed.contains(loc_id) {
                    // log(f"$Seeker Platform {index.cluster_index + 1}-{index.element_index + 1}");
                    ARCHIPELAGO_STATE.seeker_extra_pressed.push(loc_id);
                    archipelago_send_check(10030000 + ARCHIPELAGO_STATE.seeker_extra_pressed.len());
                }
            }
        }

        if ARCHIPELAGO_STATE.gamemode == 4 {
            if ARCHIPELAGO_STATE.og_randomizer_index == -1 {
                ap_OG_randomizer_change_level();
            }
            if index.element_type == ElementType::Button {
                // log(f"$Button {index.cluster_index + 1}-{index.element_index + 1}");
                archipelago_send_check(10050000 + (index.cluster_index + 1) * 100 + index.element_index + 1);
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
    on_key_char: fn(c: string) {},
    on_key_char_always: fn(c: string) {},
    on_mouse_move: fn(x: int, y: int) {},
    on_component_enter: fn() {},
    on_component_exit: fn() { Tas::archipelago_disconnect(); },
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};

static mut ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT = Component {
    id: ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT_ID,
    conflicts_with: List::of(MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID, ARCHIPELAGO_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string { text },
    draw_hud_always: archipelago_disconnected_info_hud,
    on_new_game: fn() {},
    on_level_change: fn(old: int, new: int) {},
    on_buttons_change: fn(old: int, new: int) {},
    on_cubes_change: fn(old: int, new: int) {},
    on_platforms_change: fn(old: int, new: int) {},
    on_reset: fn(old: int, new: int) {},
    on_element_pressed: fn(index: ElementIndex) {},
    on_element_released: fn(index: ElementIndex) {},
    on_key_down: fn(key: KeyCode, is_repeat: bool) {},
    on_key_down_always: fn(key: KeyCode, is_repeat: bool) {},
    on_key_up: fn(key: KeyCode) {},
    on_key_up_always: fn(key: KeyCode) {},
    on_key_char: fn(c: string) {},
    on_key_char_always: fn(c: string) {},
    on_mouse_move: fn(x: int, y: int) {},
    on_component_enter: fn() {},
    on_component_exit: fn() {},
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};

fn archipelago_disconnected() {
    ap_log_error("Disconnected from Archipelago server");
    remove_component(ARCHIPELAGO_COMPONENT);
    add_component(ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT);
    ARCHIPELAGO_STATE.ap_connected = false;
};

fn archipelago_process_item(item_id: int, starting_index: int, item_index: int) {
    
    if item_id == 9999999 {  // Grass
        archipelago_got_grass();
        return;
    }

    if ARCHIPELAGO_STATE.gamemode == 0 {
        // log(f"Processing received item index {item_id}");
        if item_id == 9999990 {  // Ledge Grab
            // log("Received Ledge Grab!");
            ARCHIPELAGO_STATE.ledge_grab = true;
            Tas::abilities_set_ledge_grab(true);
        }
        if item_id == 9999991 {  // Wall Jump
            // log("Received Wall Jump!");
            ARCHIPELAGO_STATE.wall_jump += 1;
            Tas::abilities_set_wall_jump(ARCHIPELAGO_STATE.wall_jump, true);
        }
        if item_id == 9999992 {  // Swim
            // log("Received Swim!");
            ARCHIPELAGO_STATE.swim = true;
            Tas::abilities_set_swim(true);
        }
        if item_id == 9999993 {  // jump_pads
            // log("Received jump_pads!");
            ARCHIPELAGO_STATE.jump_pads = true;
            Tas::abilities_set_jump_pads(true);
        }
        if item_id == 9999994 {  // Pipes
            // log("Received Pipes!");
            ARCHIPELAGO_STATE.pipes = true;
            Tas::abilities_set_pipes(true);
        }
        if item_id == 9999995 {  // Lifts
            // log("Received Lifts!");
            ARCHIPELAGO_STATE.lifts = true;
            Tas::abilities_set_lifts(true);
        }
        if item_id == 9999997 {  // Final Platform Known
            ARCHIPELAGO_STATE.final_platform_known = true;
        }

        if item_id == 9999989 && !ARCHIPELAGO_STATE.red_cubes_bag {  // Red Cubes Bag
            ARCHIPELAGO_STATE.red_cubes_bag = true;
            if ARCHIPELAGO_STATE.gamemode == 0 {
                if ARCHIPELAGO_STATE.cubes_options == 1 {  // red
                    for cube in Tas::get_vanilla_cubes() {
                        Tas::set_cube_collision(cube, true);
                        Tas::set_cube_color(cube, Color { red: 1., green: 0., blue: 0., alpha: 1. });
                    }
                }
                if ARCHIPELAGO_STATE.extra_cubes_options == 1 {
                    for cube in ARCHIPELAGO_STATE.extra_cubes_int_ids {
                        Tas::set_cube_collision(cube, true);
                        Tas::set_cube_color(cube, Color { red: 1., green: 0., blue: 0., alpha: 1. });
                    }
                }
            }
        }
        if item_id == 9999988 && !ARCHIPELAGO_STATE.green_cubes_bag {  // Green Cubes Bag
            ARCHIPELAGO_STATE.green_cubes_bag = true;
            if ARCHIPELAGO_STATE.gamemode == 0 {
                if ARCHIPELAGO_STATE.extra_cubes_options == 2 {
                    for cube in ARCHIPELAGO_STATE.extra_cubes_int_ids {
                        Tas::set_cube_collision(cube, true);
                        Tas::set_cube_color(cube, Color { red: 0., green: 1., blue: 0., alpha: 1. });
                    }
                }
            }
        }
        if item_id == 9999987 && !ARCHIPELAGO_STATE.blue_cubes_bag {  // Blue Cubes Bag
            ARCHIPELAGO_STATE.blue_cubes_bag = true;
        }

        let clusterindex = item_id - 10000000;
        if clusterindex >= 2 && clusterindex < 32 {
            if !ARCHIPELAGO_STATE.triggered_clusters.contains(clusterindex) {
                ARCHIPELAGO_STATE.triggering_clusters.push(clusterindex);
            }
        }
    }

    if starting_index > 0 {
        if item_id == 9999001{
            Tas::set_fog_enabled(false, SETTINGS.fog_enabled);
        }
        if item_id == 9999002{
            Tas::set_sky_light_enabled(false, SETTINGS.sky_light_enabled);
        }
        if item_id == 9999003{
            Tas::set_time_dilation(0.5, SETTINGS.time_dilation);
        }
        if item_id == 9999004{
            Tas::set_time_dilation(1.5, SETTINGS.time_dilation);
        }
        // if item_index == 9999005{
        //     log("Setting sky time speed from item speed: 7500.");
        //     Tas::set_sky_time_speed(7500., SETTINGS.sky_time_speed);
        // }
        if item_id == 9999006{
            Tas::set_stars_brightness(1000., SETTINGS.day_stars_brightness);
        }
        if item_id == 9999007{
            Tas::set_sun_redness(20., SETTINGS.sun_redness);
            Tas::set_cloud_redness(20., SETTINGS.cloud_redness);
        }
        if item_id == 9999008{
            Tas::set_cloud_speed(200., SETTINGS.cloud_speed);
        }
        if item_id == 9999009{
            Tas::set_screen_percentage(10., SETTINGS.screen_percentage);
        }
    }

}

fn archipelago_tick(time: int) {
    if time - ARCHIPELAGO_STATE.last_tick_time < 400 {
        return;
    }
    if ARCHIPELAGO_STATE.triggering_clusters.len() == 0 {
        if ARCHIPELAGO_STATE.gamemode == 3 && ARCHIPELAGO_STATE.seeker_done_triggering == 1 {
            Tas::archipelago_activate_buttons_ap();
            ARCHIPELAGO_STATE.seeker_done_triggering = 2;
        }
        return;
    }
    if ARCHIPELAGO_STATE.gamemode == 3 && ARCHIPELAGO_STATE.seeker_done_triggering == 0 {
        ARCHIPELAGO_STATE.seeker_done_triggering = 1;
    }
    ARCHIPELAGO_STATE.last_tick_time = time;

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
    if item_id == 9999941 {
        ARCHIPELAGO_STATE.unlock_block_brawl_reds = true;
        update_block_brawl_in_logic_counts();
        return;
    }
    if item_id == 9999942 {
        ARCHIPELAGO_STATE.unlock_block_brawl_blues = true;
        update_block_brawl_in_logic_counts();
        return;
    }
    if item_id == 9999943 {
        ARCHIPELAGO_STATE.unlock_block_brawl_greens = true;
        update_block_brawl_in_logic_counts();
        return;
    }
    if item_id == 9999944 {
        ARCHIPELAGO_STATE.unlock_block_brawl_yellows = true;
        update_block_brawl_in_logic_counts();
        return;
    }

    ARCHIPELAGO_STATE.received_items.push(item_id);
    if ARCHIPELAGO_STATE.started < 2 {
        return;
    }
    archipelago_process_item(item_id, starting_index, index);
}

fn update_block_brawl_in_logic_counts(){
    if ARCHIPELAGO_STATE.done_block_brawl_minigame {
        return;
    }
    let mut number_of_colors = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_reds {
        number_of_colors += 1;
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_blues {
        number_of_colors += 1;
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_greens {
        number_of_colors += 1;
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_yellows {
        number_of_colors += 1;
    }
    let scores_in_logic = List::of(0,15,30,60,120);
    ARCHIPELAGO_STATE.in_logic_block_brawl_reds = 0;
    ARCHIPELAGO_STATE.in_logic_block_brawl_blues = 0;
    ARCHIPELAGO_STATE.in_logic_block_brawl_greens = 0;
    ARCHIPELAGO_STATE.in_logic_block_brawl_yellows = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_reds {
        ARCHIPELAGO_STATE.in_logic_block_brawl_reds = scores_in_logic.get(number_of_colors).unwrap();
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_blues {
        ARCHIPELAGO_STATE.in_logic_block_brawl_blues = scores_in_logic.get(number_of_colors).unwrap();
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_greens {
        ARCHIPELAGO_STATE.in_logic_block_brawl_greens = scores_in_logic.get(number_of_colors).unwrap();
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_yellows {
        ARCHIPELAGO_STATE.in_logic_block_brawl_yellows = scores_in_logic.get(number_of_colors).unwrap();
    }

    let block_brawl_locations = List::of( 
        10071001,10071003,10071006,10071010,10071015,10071018,10071021,10071024,10071027,10071030,10071036,10071042,10071048,10071054,10071060,10071072,10071084,10071096,10071108,10071120,
        10072001,10072003,10072006,10072010,10072015,10072018,10072021,10072024,10072027,10072030,10072036,10072042,10072048,10072054,10072060,10072072,10072084,10072096,10072108,10072120,
        10073001,10073003,10073006,10073010,10073015,10073018,10073021,10073024,10073027,10073030,10073036,10073042,10073048,10073054,10073060,10073072,10073084,10073096,10073108,10073120,
        10074001,10074003,10074006,10074010,10074015,10074018,10074021,10074024,10074027,10074030,10074036,10074042,10074048,10074054,10074060,10074072,10074084,10074096,10074108,10074120,
    );
    let mut block_brawl_locations_in_logic = List::new();

    let mut ncolors = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_reds {
        ncolors += 1;
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_blues {
        ncolors += 1;
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_greens {
        ncolors += 1;
    }
    if ARCHIPELAGO_STATE.unlock_block_brawl_yellows {
        ncolors += 1;
    }
    let check_points = List::of(0,15,30,60,120);
    let max_in_logic = check_points.get(ncolors).unwrap();

    for lid in block_brawl_locations {
        let color = (lid-10070000)/1000;
        let score = lid % 1000;
        if (color == 1 && ARCHIPELAGO_STATE.unlock_block_brawl_reds) ||
              (color == 2 && ARCHIPELAGO_STATE.unlock_block_brawl_blues) ||
              (color == 3 && ARCHIPELAGO_STATE.unlock_block_brawl_greens) ||
              (color == 4 && ARCHIPELAGO_STATE.unlock_block_brawl_yellows) {
                if score <= max_in_logic {
                    block_brawl_locations_in_logic.push(lid);
                }
          }
    }

    let mut number_pressed = 0;
    let mut number_pressed_in_logic = 0;
    for lid in block_brawl_locations {
        if ARCHIPELAGO_STATE.checked_locations.contains(lid) {
            number_pressed += 1;
            if block_brawl_locations_in_logic.contains(lid) {
                number_pressed_in_logic += 1;
            }
        }
    }
    if number_pressed == block_brawl_locations.len() {
        ARCHIPELAGO_STATE.done_block_brawl_minigame = true;
        ap_log(List::of(ColorfulText { text:"Completed Block Brawl Minigame!", color: AP_COLOR_GREEN }));
    }

    ARCHIPELAGO_STATE.block_brawl_check_in_logic = block_brawl_locations_in_logic.len() - number_pressed_in_logic;

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
    ARCHIPELAGO_STATE.grass = 0;
    
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
    if ARCHIPELAGO_STATE.gamemode == 5 {
        // log("Starting Block Brawl Minigame gamemode");
        archipelago_block_brawl_start();
    }
    
    let mut i = 0;
    for item in ARCHIPELAGO_STATE.received_items {
        archipelago_process_item(item, 0, i);
        i += 1;
    }
    ARCHIPELAGO_STATE.started = 2;

}

fn archipelago_main_start(){
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.wall_jump = 0;
    ARCHIPELAGO_STATE.ledge_grab = false;
    ARCHIPELAGO_STATE.swim = false;
    ARCHIPELAGO_STATE.jump_pads = false;

    ARCHIPELAGO_STATE.triggered_clusters = List::new();
    ARCHIPELAGO_STATE.has_goaled = false;

    Tas::abilities_set_swim(false);
    Tas::abilities_set_wall_jump(0, true);
    Tas::abilities_set_ledge_grab(false);
    Tas::abilities_set_jump_pads(false);
    Tas::abilities_set_pipes(false);
    Tas::abilities_set_lifts(false);

    Tas::archipelago_deactivate_buttons_ap();

    Tas::reset_cubes(true, true);

    spawn_extra_cubes();
    
    archipelago_activate_stepped_on_platforms();
    archipelago_collect_collected_cubes();
}

fn spawn_extra_cubes(){    
    if ARCHIPELAGO_STATE.extra_cubes_options == 9{
        return;
    }
    let cubes_to_spawn_locs = List::of(
        -4562.5,  -875.,  1375.,  
        -1985.,  -2262.5,  2090.,  
        -5062.5,  -3312.5,  62.5,  
        -3237.5,  -3744.,  35.,  
        625.,  1625.,  800.,  
        1415.,  2625.,  215.,  
        6.3806667,  2880.,  965.,  
        1250.,  -3750.,  62.5,  
        1695.,  4636.,  2000.,  
        2187.5,  6375.,  62.5
    );

    let cubes_to_spawn_ids = List::of(
        10080501,
        10080602,
        10080702,
        10080803,
        10081302,
        10081402,
        10081403,
        10081901,
        10082501,
        10082601
    );

    let mut i = 0;
    while i < cubes_to_spawn_ids.len() {
        let x = cubes_to_spawn_locs.get(i*3).unwrap();
        let y = cubes_to_spawn_locs.get(i*3 + 1).unwrap();
        let z = cubes_to_spawn_locs.get(i*3 + 2).unwrap();
        let loc_id = cubes_to_spawn_ids.get(i).unwrap();

        if ARCHIPELAGO_STATE.checked_locations.contains(loc_id) {
            i += 1;
            continue;
        }

        let mut col = Color { red: 1., green: 0., blue: 0., alpha: 1. };
        let mut ready = true;
        if ARCHIPELAGO_STATE.extra_cubes_options == 1 {
            if ARCHIPELAGO_STATE.red_cubes_bag {
                col.red = 1.;
                col.green = 0.;
                col.blue = 0.;
            }else{
                col.red = 0.;
                col.green = 0.;
                col.blue = 0.;
                ready = false;
            }
        }
        if ARCHIPELAGO_STATE.extra_cubes_options == 2 {
            if ARCHIPELAGO_STATE.green_cubes_bag {
                col.red = 0.;
                col.green = 1.;
                col.blue = 0.;
            }else{
                col.red = 0.;
                col.green = 0.;
                col.blue = 0.;
                ready = false;
            }
        }
        let id = Tas::set_cube_scale(Tas::set_cube_color(Tas::spawn_cube(Location { x: x, y: y, z: z - 162.5 }), col), 1.0);
        if !ready {
            Tas::set_cube_collision(id, false);
        }
        ARCHIPELAGO_STATE.extra_cubes_int_ids.push(id);
        ARCHIPELAGO_STATE.extra_cubes_locs.push(loc_id);

        i += 1;
    }
    
}



fn got_extra_cube_ap(element_index: int){
    let mut found_index = Option::None;
    let mut i = 0;
    for id in ARCHIPELAGO_STATE.extra_cubes_int_ids {
        if id == element_index {
            found_index = Option::Some(i);
            break;
        }
        i += 1;
    }
    match found_index {
        Option::Some(pos) => {
            let loc = ARCHIPELAGO_STATE.extra_cubes_locs.get(pos).unwrap();
            archipelago_send_check(loc);
        },
        Option::None => {}
    }
}

fn archipelago_vanilla_start(){
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(2, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    collect_all_vanilla_cubes();
    ARCHIPELAGO_STATE.last_level_unlocked = 1;

    Tas::set_platform_movement_path(
        Tas::spawn_platform(Location { x: 1000., y: 1000., z: 1000. }, Rotation {pitch : 0., yaw: 0., roll: 0. }),
        500., List::of(List::of(1000., 1000., 1000.), List::of(500., -500., -100.), List::of(300., 100., -200.)), 3
    );
}

fn archipelago_seeker_start(){
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(2, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    collect_all_vanilla_cubes();
    ARCHIPELAGO_STATE.last_level_unlocked = 1;

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
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(2, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    collect_all_vanilla_cubes();
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
}

fn archipelago_og_randomizer_start(){
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(2, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    collect_all_vanilla_cubes();
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
}

fn archipelago_block_brawl_start(){
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(2, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    Tas::archipelago_deactivate_buttons_ap();
    collect_all_vanilla_cubes();
    ARCHIPELAGO_STATE.last_level_unlocked = 1;
    ARCHIPELAGO_STATE.block_brawl_cubes_collected = 0;
    ARCHIPELAGO_STATE.block_brawl_cubes_total = 0;

    ARCHIPELAGO_STATE.block_brawl_red_ids.clear();
    ARCHIPELAGO_STATE.block_brawl_blue_ids.clear();
    ARCHIPELAGO_STATE.block_brawl_green_ids.clear();
    ARCHIPELAGO_STATE.block_brawl_yellow_ids.clear();
    ARCHIPELAGO_STATE.block_brawl_cubes_collected = 0;
    ARCHIPELAGO_STATE.score_for_next_block = 1;

    Tas::archipelago_ds_get(f"RFBB_r_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}");
    Tas::archipelago_ds_get(f"RFBB_b_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}");
    Tas::archipelago_ds_get(f"RFBB_g_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}");
    Tas::archipelago_ds_get(f"RFBB_y_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}");

    let mut i = 0;
    while i < 50 {
        Tas::spawn_platform_rando_location(3000., 10.);
        i += 1;
    }

    // log(f"Spawning Block Brawl cubes, colors unlocked: {ARCHIPELAGO_STATE.unlock_block_brawl_reds}, {ARCHIPELAGO_STATE.unlock_block_brawl_greens}, {ARCHIPELAGO_STATE.unlock_block_brawl_blues}, {ARCHIPELAGO_STATE.unlock_block_brawl_yellows}");

    let mut j = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_reds {
        // log("Spawning Block Brawl Reds");
        while j < 5 {
            let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., true),2.), Color { red: 1., green: 0., blue: 0., alpha: 1. });
            ARCHIPELAGO_STATE.block_brawl_red_ids.push(id);
            j += 1;
        }
        ARCHIPELAGO_STATE.block_brawl_cubes_total += 5;
    }
    j = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_greens {
        // log("Spawning Block Brawl Greens");
        while j < 5 {
            let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., true),2.), Color { red: 0., green: 1., blue: 0., alpha: 1. });
            ARCHIPELAGO_STATE.block_brawl_green_ids.push(id);
            j += 1;
        }
        ARCHIPELAGO_STATE.block_brawl_cubes_total += 5;
    }
    j = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_blues {
        // log("Spawning Block Brawl Blues");
        while j < 5 {
            let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., true),2.), Color { red: 0., green: 0., blue: 1., alpha: 1. });
            ARCHIPELAGO_STATE.block_brawl_blue_ids.push(id);
            j += 1;
        }
        ARCHIPELAGO_STATE.block_brawl_cubes_total += 5;
    }
    j = 0;
    if ARCHIPELAGO_STATE.unlock_block_brawl_yellows {
        // log("Spawning Block Brawl Yellows");
        while j < 5 {
            let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., true),2.), Color { red: 1., green: 1., blue: 0., alpha: 1. });
            ARCHIPELAGO_STATE.block_brawl_yellow_ids.push(id);
            j += 1;
        }
        ARCHIPELAGO_STATE.block_brawl_cubes_total += 5;
    }
}

fn got_cube_block_brawl(id: int){
    let score_list_based_on_number_of_cubes = List::of(1,2,3,4,5,8,11,14,17,20,26,32,38,44,50,60,70,80,90,100);
    let score_to_add = score_list_based_on_number_of_cubes.get(ARCHIPELAGO_STATE.block_brawl_cubes_collected).unwrap_or(1);
    
    let check_points = List::of(1,3,6,10,15,18,21,24,27,30,36,42,48,54,60,72,84,96,108,120);
    if ARCHIPELAGO_STATE.block_brawl_red_ids.contains(id) {
        ARCHIPELAGO_STATE.block_brawl_red_ids.remove(id);
        ARCHIPELAGO_STATE.score_block_brawl_reds += score_to_add;
        for cp in check_points {
            if ARCHIPELAGO_STATE.score_block_brawl_reds >= cp {
                Tas::archipelago_send_check(10071000 + cp);
            }
        }
        Tas::archipelago_ds_set_max(f"RFBB_r_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}",0, ARCHIPELAGO_STATE.score_block_brawl_reds);
    }
    if ARCHIPELAGO_STATE.block_brawl_blue_ids.contains(id) {
        ARCHIPELAGO_STATE.block_brawl_blue_ids.remove(id);
        ARCHIPELAGO_STATE.score_block_brawl_blues += score_to_add;
        for cp in check_points {
            if ARCHIPELAGO_STATE.score_block_brawl_blues >= cp {
                Tas::archipelago_send_check(10072000 + cp);
            }
        }
        Tas::archipelago_ds_set_max(f"RFBB_b_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}",0, ARCHIPELAGO_STATE.score_block_brawl_blues);
    }
    if ARCHIPELAGO_STATE.block_brawl_green_ids.contains(id) {
        ARCHIPELAGO_STATE.block_brawl_green_ids.remove(id);
        ARCHIPELAGO_STATE.score_block_brawl_greens += score_to_add;
        for cp in check_points {
            if ARCHIPELAGO_STATE.score_block_brawl_greens >= cp {
                Tas::archipelago_send_check(10073000 + cp);
            }
        }
        Tas::archipelago_ds_set_max(f"RFBB_g_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}",0, ARCHIPELAGO_STATE.score_block_brawl_greens);
    }
    if ARCHIPELAGO_STATE.block_brawl_yellow_ids.contains(id) {
        ARCHIPELAGO_STATE.block_brawl_yellow_ids.remove(id);
        ARCHIPELAGO_STATE.score_block_brawl_yellows += score_to_add;
        for cp in check_points {
            if ARCHIPELAGO_STATE.score_block_brawl_yellows >= cp {
                Tas::archipelago_send_check(10074000 + cp);
            }
        }
        Tas::archipelago_ds_set_max(f"RFBB_y_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}",0, ARCHIPELAGO_STATE.score_block_brawl_yellows);
    }
    ARCHIPELAGO_STATE.block_brawl_cubes_collected += 1;
    let next_score = score_list_based_on_number_of_cubes.get(ARCHIPELAGO_STATE.block_brawl_cubes_collected).unwrap_or(1);
    ARCHIPELAGO_STATE.score_for_next_block = next_score;

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
    if id >= 10060000 && id < 10070000 {
        ARCHIPELAGO_STATE.collected_cubes.push(id);
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
            ap_log(List::of(ColorfulText { text:"Completed Button Galore Minigame!", color: AP_COLOR_GREEN }));
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
    let block_brawl_locations = List::of( 
        10071001,10071003,10071006,10071010,10071015,10071018,10071021,10071024,10071027,10071030,10071036,10071042,10071048,10071054,10071060,10071072,10071084,10071096,10071108,10071120,
        10072001,10072003,10072006,10072010,10072015,10072018,10072021,10072024,10072027,10072030,10072036,10072042,10072048,10072054,10072060,10072072,10072084,10072096,10072108,10072120,
        10073001,10073003,10073006,10073010,10073015,10073018,10073021,10073024,10073027,10073030,10073036,10073042,10073048,10073054,10073060,10073072,10073084,10073096,10073108,10073120,
        10074001,10074003,10074006,10074010,10074015,10074018,10074021,10074024,10074027,10074030,10074036,10074042,10074048,10074054,10074060,10074072,10074084,10074096,10074108,10074120,
    );
    if block_brawl_locations.contains(id) {
        update_block_brawl_in_logic_counts();
    }
}
fn archipelago_retrieved(key: string, value: string){
    if key == f"RFBB_r_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}" {
        let v = value.parse_int().unwrap();
        if v > ARCHIPELAGO_STATE.score_block_brawl_reds {
            ARCHIPELAGO_STATE.score_block_brawl_reds = v;
        }
    }
    if key == f"RFBB_b_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}" {
        let v = value.parse_int().unwrap();
        if v > ARCHIPELAGO_STATE.score_block_brawl_blues {
            ARCHIPELAGO_STATE.score_block_brawl_blues = v;
        }
    }
    if key == f"RFBB_g_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}" {
        let v = value.parse_int().unwrap();
        if v > ARCHIPELAGO_STATE.score_block_brawl_greens {
            ARCHIPELAGO_STATE.score_block_brawl_greens = v;
        }
    }
    if key == f"RFBB_y_{ARCHIPELAGO_ROOM_INFO.this_player_team}_{ARCHIPELAGO_ROOM_INFO.this_player_slot}" {
        let v = value.parse_int().unwrap();
        if v > ARCHIPELAGO_STATE.score_block_brawl_yellows {
            ARCHIPELAGO_STATE.score_block_brawl_yellows = v;
        }
    }
}

fn archipelago_send_check(id: int){
    if ARCHIPELAGO_STATE.checked_locations.contains(id) {
        return;
    }
    Tas::archipelago_send_check(id);
    archipelago_checked_location(id);
}
fn archipelago_activate_stepped_on_platforms(){
    for id in ARCHIPELAGO_STATE.stepped_on_platforms {
        let cluster = (id - 10010000) / 100;
        let plat = (id - 10010000) % 100;

        Tas::trigger_element_by_type(cluster-1, "Platform", plat-1);
    }
}
fn collect_all_vanilla_cubes(){
    let all_cubes = Tas::get_vanilla_cubes();
    for cube in all_cubes {
        Tas::collect_cube(cube);
    }
}
fn archipelago_collect_collected_cubes(){
    if ARCHIPELAGO_STATE.cubes_options == 9{
        collect_all_vanilla_cubes();
    }else{
        for id in ARCHIPELAGO_STATE.collected_cubes {
            let cluster = (id - 10060000) / 100;
            let plat = (id - 10060000) % 100;

            match Tas::get_vanilla_cube(cluster-1, plat-1) {
                Option::Some(cube) => { Tas::collect_cube(cube); },
                Option::None => {}
            }
        }

        let all_cubes = Tas::get_vanilla_cubes();
        for cube in all_cubes {
            Tas::set_cube_color(cube, Color { red: 1., green: 0., blue: 0., alpha: 1. });
            Tas::set_cube_scale(cube, 2.0);
        }

        if ARCHIPELAGO_STATE.cubes_options == 1 && !ARCHIPELAGO_STATE.red_cubes_bag {  // red
            for cube in all_cubes {
                Tas::set_cube_color(cube, Color { red: 0., green: 0., blue: 0., alpha: 1. });
                Tas::set_cube_collision(cube, false);
            }
        }
        
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

    if key == "cubes" {
        ARCHIPELAGO_STATE.cubes_options = value.parse_int().unwrap();
    }
    if key == "extra_cubes" {
        ARCHIPELAGO_STATE.extra_cubes_options = value.parse_int().unwrap();
    }
}
