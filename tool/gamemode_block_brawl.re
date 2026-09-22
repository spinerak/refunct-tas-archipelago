
struct BlockBrawlState {
    started: int,
    score_block_brawl_reds: int,
    score_block_brawl_blues: int,
    score_block_brawl_greens: int,
    score_block_brawl_yellows: int,
    score_for_next_block_brawl: int,
    block_brawl_cubes_collected: int,
    block_brawl_cubes_total: int,
    block_brawl_red_ids: List<int>,
    block_brawl_blue_ids: List<int>,
    block_brawl_green_ids: List<int>,
    block_brawl_yellow_ids: List<int>,
    block_brawl_time: int,
    block_brawl_previous_time: int,
    setting_up: bool,
    dash: bool,
    finish_result: string,
}

fn fresh_block_brawl_state() -> BlockBrawlState {
    BlockBrawlState {
        started: 0,
        score_block_brawl_reds: 0,
        score_block_brawl_blues: 0,
        score_block_brawl_greens: 0,
        score_block_brawl_yellows: 0,
        score_for_next_block_brawl: 1,
        block_brawl_cubes_collected: 0,
        block_brawl_cubes_total: 0,
        block_brawl_red_ids: List::new(),
        block_brawl_blue_ids: List::new(),
        block_brawl_green_ids: List::new(),
        block_brawl_yellow_ids: List::new(),
        block_brawl_time: 0,
        block_brawl_previous_time: 0,
        setting_up: false,
        dash: false,
        finish_result: "",
    }
}

struct BlockBrawlSettings {
    preset: string, // "Default Brawl", "Default Blub", "Custom"
    brawl_or_blub: string, // "Block Brawl" or "Block Blub"
    alt: bool, // false means normal, true means crazy
    wall_jump_or_dash: string, // "Wall jump", "Dash", or "Toggle"
    count_down_time: int, // in seconds
    goal_score: int, // in points
    number_of_colors: int, // 1-4
    number_of_cubes_per_color: int, // 1-10
    number_of_platforms: int, // 100-400
    scoring_method: string, // "Sum + lowest", "Sum", "Lowest"
}

fn default_brawl_settings() -> BlockBrawlSettings {
    BlockBrawlSettings {
        preset: "Default Brawl",
        brawl_or_blub: "Block Brawl",
        alt: false,
        wall_jump_or_dash: "Wall jump",
        count_down_time: 0,
        goal_score: 3600,
        number_of_colors: 4,
        number_of_cubes_per_color: 5,
        number_of_platforms: 200,
        scoring_method: "Sum + lowest",
    }
}
fn default_blub_settings() -> BlockBrawlSettings {
    BlockBrawlSettings {
        preset: "Default Blub",
        brawl_or_blub: "Block Blub",
        alt: false,
        wall_jump_or_dash: "Wall jump",
        count_down_time: 0,
        goal_score: 3600,
        number_of_colors: 4,
        number_of_cubes_per_color: 5,
        number_of_platforms: 200,
        scoring_method: "Sum + lowest",
    }
}

static mut BLOCK_BRAWL_STATE = fresh_block_brawl_state();
static mut BLOCK_BRAWL_SETTINGS = default_brawl_settings();

static mut BLOCK_BRAWL_COMPONENT = Component {
    id: BLOCK_BRAWL_COMPONENT_ID,
    conflicts_with: List::of(BLOCK_BRAWL_COMPONENT_ID, MAP_EDITOR_COMPONENT_ID, ARCHIPELAGO_COMPONENT_ID, BLOCK_BRAWL_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID, ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    error_message: "",
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: block_brawl_hud_text,
    draw_hud_always: block_brawl_hub_color_coded,
    on_new_game: fn() {
        Tas::set_goal_animation_should_play(true); // so it works for minigames :3
        Tas::enable_all_buttons();

        Tas::destroy_cubes(true, true);
        Tas::destroy_platforms(true, 2);
        Tas::destroy_spawners();

        Tas::abilities_set_swim(true);
        BLOCK_BRAWL_STATE.started = 1;

        update_block_brawl_in_logic_counts();

        if BLOCK_BRAWL_SETTINGS.wall_jump_or_dash == "Dash" {
            BLOCK_BRAWL_STATE.dash = true;
            Tas::abilities_set_wall_jump(0, false);
        } else {
            BLOCK_BRAWL_STATE.dash = false;
            Tas::abilities_set_wall_jump(2, false);
        }
    },
    on_level_change: ap_on_level_change_function,
    on_buttons_change: fn(old: int, new: int) {},
    on_cubes_change: fn(old: int, new: int) {},
    on_platforms_change: fn(old: int, new: int) {},
    on_reset: fn(old: int, new: int) {},
    on_element_pressed: fn(index: ElementIndex) {
        if BLOCK_BRAWL_STATE.started == 0 {
            return;
        }
        if BLOCK_BRAWL_STATE.started == 1 {
            block_brawl_start();
        }    
        
        if index.cluster_index == 9999 {
            got_cube_block_brawl_solo(index.element_index);
            return;
        }
    },
    on_element_released: fn(index: ElementIndex) {},
    on_key_down: fn(key: KeyCode, is_repeat: bool) {
        if BLOCK_BRAWL_STATE.dash {
            if key.to_small() == KEY_E.to_small() {
                Tas::dash(1000.0, SETTINGS.downward_dash_enabled);
            }
        }
        if key.to_small() == KEY_T.to_small() && BLOCK_BRAWL_SETTINGS.wall_jump_or_dash == "Toggle" {
            BLOCK_BRAWL_STATE.dash = !BLOCK_BRAWL_STATE.dash;
            Tas::abilities_set_wall_jump(if BLOCK_BRAWL_STATE.dash { 0 } else { 2 }, false);
        }
    },
    on_key_down_always: fn(key: KeyCode, is_repeat: bool) {},
    on_key_up: fn(key: KeyCode) {},
    on_key_up_always: fn(key: KeyCode) {},
    on_key_char: fn(c: string) {},
    on_key_char_always: fn(c: string) {},
    on_mouse_move: fn(x: int, y: int) {},
    on_component_enter: fn() {},
    on_component_exit: fn() { BLOCK_BRAWL_STATE = fresh_block_brawl_state(); },
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};

fn block_brawl_wall_jump_input() {
    if BLOCK_BRAWL_STATE.dash {
        Tas::dash(1000.0, SETTINGS.downward_dash_enabled);
    }
}


fn block_brawl_start(){
    BLOCK_BRAWL_STATE.setting_up = true;
    Tas::abilities_set_swim(true);
    Tas::abilities_set_wall_jump(if BLOCK_BRAWL_STATE.dash { 0 } else { 2 }, false);
    Tas::abilities_set_ledge_grab(true);
    Tas::abilities_set_jump_pads(true);
    Tas::abilities_set_pipes(true);
    Tas::abilities_set_lifts(true);
    Tas::deactivate_all_buttons();
    collect_all_vanilla_cubes();
    BLOCK_BRAWL_STATE.block_brawl_cubes_collected = 0;
    BLOCK_BRAWL_STATE.block_brawl_cubes_total = 0;

    BLOCK_BRAWL_STATE.block_brawl_red_ids.clear();
    BLOCK_BRAWL_STATE.block_brawl_blue_ids.clear();
    BLOCK_BRAWL_STATE.block_brawl_green_ids.clear();
    BLOCK_BRAWL_STATE.block_brawl_yellow_ids.clear();
    BLOCK_BRAWL_STATE.block_brawl_cubes_collected = 0;
    BLOCK_BRAWL_STATE.score_for_next_block_brawl = 1;

    let mut i = 0;
    if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Brawl" {
        if !BLOCK_BRAWL_SETTINGS.alt {
            while i < BLOCK_BRAWL_SETTINGS.number_of_platforms {
                Tas::spawn_platform_rando_location(3000., 3000., 10.);
                i += 1;
            }
        } else {
            while i < BLOCK_BRAWL_SETTINGS.number_of_platforms {
                Tas::spawn_platform_rando_location_crazy(3000.);
                i += 1;
            }
        }
    }
    if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Blub" {
        while i < BLOCK_BRAWL_SETTINGS.number_of_platforms {
            Tas::spawn_platform_rando_location_uw();
            i += 1;
        }
    }

        let mut j = 0;
        while j < BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color {
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Brawl" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., 3000., true),2.), Color { red: 1., green: 0., blue: 0., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_red_ids.push(id);
            }
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Blub" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location_uw(),4.), Color { red: 1., green: 0., blue: 0., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_red_ids.push(id);
            }
            j += 1;
        }
        BLOCK_BRAWL_STATE.block_brawl_cubes_total += BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color;

    if BLOCK_BRAWL_SETTINGS.number_of_colors >= 2 {
        j = 0;
        while j < BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color {
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Brawl" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., 3000., true),2.), Color { red: 0., green: 1., blue: 0., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_green_ids.push(id);
            }
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Blub" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location_uw(),4.), Color { red: 0., green: 0.9, blue: 0., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_green_ids.push(id);
            }
            j += 1;
        }
        BLOCK_BRAWL_STATE.block_brawl_cubes_total += BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color;
    }
    
    if BLOCK_BRAWL_SETTINGS.number_of_colors >= 3 {
        j = 0;
        while j < BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color {
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Brawl" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., 3000., true),2.), Color { red: 0., green: 0., blue: 1., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_blue_ids.push(id);
            }
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Blub" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location_uw(),4.), Color { red: 0., green: 0., blue: 1., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_blue_ids.push(id);
            }
            j += 1;
        }
        BLOCK_BRAWL_STATE.block_brawl_cubes_total += BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color;
    }
    
    if BLOCK_BRAWL_SETTINGS.number_of_colors >= 4 {
        j = 0;
        while j < BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color {
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Brawl" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location(3000., 3000., true),2.), Color { red: 1., green: 1., blue: 0., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_yellow_ids.push(id);
            }
            if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Blub" {
                let id = Tas::set_cube_color(Tas::set_cube_scale(Tas::spawn_cube_rando_location_uw(),4.), Color { red: 1., green: 1., blue: 0., alpha: 1. });
                BLOCK_BRAWL_STATE.block_brawl_yellow_ids.push(id);
            }
            j += 1;
        }
        BLOCK_BRAWL_STATE.block_brawl_cubes_total += BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color;
    }
        
    BLOCK_BRAWL_STATE.started = 2;
}

fn got_cube_block_brawl_solo(id: int){
    let score_list_based_on_number_of_cubes = List::of(1,2,3,4,5,8,11,14,17,20,26,32,38,44,50,60,70,80,90,100);
    let score_to_add = score_list_based_on_number_of_cubes.get(BLOCK_BRAWL_STATE.block_brawl_cubes_collected).unwrap_or(100);
    
    
    if BLOCK_BRAWL_STATE.block_brawl_red_ids.contains(id) {
        BLOCK_BRAWL_STATE.block_brawl_red_ids.remove(id);
        BLOCK_BRAWL_STATE.score_block_brawl_reds += score_to_add;
    }
    if BLOCK_BRAWL_STATE.block_brawl_blue_ids.contains(id) {
        BLOCK_BRAWL_STATE.block_brawl_blue_ids.remove(id);
        BLOCK_BRAWL_STATE.score_block_brawl_blues += score_to_add;
    }
    if BLOCK_BRAWL_STATE.block_brawl_green_ids.contains(id) {
        BLOCK_BRAWL_STATE.block_brawl_green_ids.remove(id);
        BLOCK_BRAWL_STATE.score_block_brawl_greens += score_to_add;
    }
    if BLOCK_BRAWL_STATE.block_brawl_yellow_ids.contains(id) {
        BLOCK_BRAWL_STATE.block_brawl_yellow_ids.remove(id);
        BLOCK_BRAWL_STATE.score_block_brawl_yellows += score_to_add;
    }
    BLOCK_BRAWL_STATE.block_brawl_cubes_collected += 1;
    let next_score = score_list_based_on_number_of_cubes.get(BLOCK_BRAWL_STATE.block_brawl_cubes_collected).unwrap_or(100);
    BLOCK_BRAWL_STATE.score_for_next_block_brawl = next_score;
}

fn block_brawl_tick(time: int) {
    if BLOCK_BRAWL_STATE.block_brawl_previous_time == 0 || BLOCK_BRAWL_STATE.started == 0 {
        BLOCK_BRAWL_STATE.block_brawl_previous_time = time;
        return;
    }
    if BLOCK_BRAWL_STATE.setting_up {
        BLOCK_BRAWL_STATE.block_brawl_previous_time = time;
        BLOCK_BRAWL_STATE.setting_up = false;
        return;
    }
    BLOCK_BRAWL_STATE.block_brawl_time += time - BLOCK_BRAWL_STATE.block_brawl_previous_time;
    BLOCK_BRAWL_STATE.block_brawl_previous_time = time;
}