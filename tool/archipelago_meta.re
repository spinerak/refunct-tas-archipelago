struct ArchipelagoPlayer {
    team: int,
    slot: int,
    alias: string,
    name: string
}

struct ArchipelagoSlot {
    slot: int,
    name: string,
    game: string,
    type: int,
    group_members: List<int>
}

struct ArchipelagoGame {
    name: string,
    item_name_to_id: Map<string, int>,
    item_id_to_name: Map<int, string>,
    location_name_to_id: Map<string, int>,
    location_id_to_name: Map<int, string>
}

struct ArchipelagoRoomInfo {
    this_player_team: int,
    this_player_slot: int,
    slots: Map<int, ArchipelagoSlot>,
    players: Map<int, ArchipelagoPlayer>,
    games: Map<string, ArchipelagoGame>
}

static mut ARCHIPELAGO_ROOM_INFO = ArchipelagoRoomInfo {
    this_player_team: -1,
    this_player_slot: -1,
    slots: Map::new(),
    players: Map::new(),
    games: Map::new()
};

static UNKNOWN_PLAYER = "(someone?)";
static UNKNOWN_ITEM = "(something?)";
static UNKNOWN_LOCATION = "(somewhere?)";

fn archipelago_register_slot(index: int, name: string, game: string, type: int, group_members: List<int>) {
    ARCHIPELAGO_ROOM_INFO.slots.insert(index, ArchipelagoSlot {
        slot: index,
        name: name,
        game: game,
        type: type,
        group_members: group_members
    });
    log(f"Registered slot {index}: {ARCHIPELAGO_ROOM_INFO.slots.get(index).unwrap()}");
}

fn archipelago_register_player(team: int, slot: int, alias: string, name: string) {
    ARCHIPELAGO_ROOM_INFO.players.insert(slot, ArchipelagoPlayer {
        team: team,
        slot: slot,
        alias: alias,
        name: name,
    });
    log(f"Registered player {slot}: {ARCHIPELAGO_ROOM_INFO.players.get(slot).unwrap()}");
}

fn archipelago_get_or_create_game_info(game_name: string) -> ArchipelagoGame {
    match ARCHIPELAGO_ROOM_INFO.games.get(game_name) {
        Option::Some(game) => game,
        Option::None => {
            let game = ArchipelagoGame {
                name: game_name,
                item_name_to_id: Map::new(),
                item_id_to_name: Map::new(),
                location_name_to_id: Map::new(),
                location_id_to_name: Map::new()
            };
            ARCHIPELAGO_ROOM_INFO.games.insert(game_name, game);
            log(f"Registered game {game_name}");
            game
        }
    }
};

fn archipelago_register_game_item(game_name: string, item_name: string, item_id: int) {
    let game_info = archipelago_get_or_create_game_info(game_name);
    let item_name_to_id = game_info.item_name_to_id.insert(item_name, item_id);
    let item_id_to_name = game_info.item_id_to_name.insert(item_id, item_name);
}

fn archipelago_register_game_location(game_name: string, location_name: string, location_id: int) {
    let game_info = archipelago_get_or_create_game_info(game_name);
    let location_name_to_id = game_info.location_name_to_id.insert(location_name, location_id);
    let location_id_to_name = game_info.location_id_to_name.insert(location_id, location_name);
}

fn get_item_name_for_player_by_id(player_id: int, item_id: int) -> string {
    let slot_id = match ARCHIPELAGO_ROOM_INFO.players.get(player_id) {
        Option::Some(player) => player.slot,
        Option::None => return UNKNOWN_ITEM,
    };

    let game_name = match ARCHIPELAGO_ROOM_INFO.slots.get(slot_id) {
        Option::Some(slot) => slot.game,
        Option::None => return UNKNOWN_ITEM,
    };

    let game = match ARCHIPELAGO_ROOM_INFO.games.get(game_name) {
        Option::Some(game) => game,
        Option::None => return UNKNOWN_ITEM,
    };

    match game.item_id_to_name.get(item_id) {
        Option::Some(item_name) => item_name,
        Option::None => UNKNOWN_ITEM
    }
}

fn get_location_name_for_player_by_id(player_id: int, location_id: int) -> string {
    let slot_id = match ARCHIPELAGO_ROOM_INFO.players.get(player_id) {
        Option::Some(player) => player.slot,
        Option::None => return UNKNOWN_LOCATION,
    };

    let game_name = match ARCHIPELAGO_ROOM_INFO.slots.get(slot_id) {
        Option::Some(slot) => slot.game,
        Option::None => return UNKNOWN_LOCATION,
    };

    let game = match ARCHIPELAGO_ROOM_INFO.games.get(game_name) {
        Option::Some(game) => game,
        Option::None => return UNKNOWN_LOCATION,
    };

    match game.location_id_to_name.get(location_id) {
        Option::Some(location_name) => location_name,
        Option::None => UNKNOWN_LOCATION
    }
}

// TODO:
// - Add message filtering and corresponding menu option
//   - List::of(All, OnlyYou, OnlyProgressive, OnlyYouAndProgressive)?
// - Add prettier logging and a menu option to keep logs for a certain amount of time
//   - Options: Forever, Duration, Off; Duration is controlled by a float setting (10s default?)
//   - Another option to control maximum number of messages shown?
//   - Should the logs have a separate UI scale option? Probably not.
//   - Logs should appear permanently when the player opens the AP menu (maybe?)
//     - Annoyingly, you can leave the menu open. We should maybe fix this?
// - Delete lots of the old logging (no longer needed)

fn archipelago_print_json_message(json_message: ReboPrintJSONMessage) {
    let mut message = "";
    for part in json_message.data {
        let text = match part._type {
            "player_id" => {
                let message_text = part.text.unwrap();
                match message_text.parse_int() {
                    Result::Ok(player_id) => match ARCHIPELAGO_ROOM_INFO.players.get(player_id) {
                        Option::Some(player) => player.name,
                        Option::None => UNKNOWN_PLAYER,
                    },
                    Result::Err(e) => UNKNOWN_PLAYER
                }
            },
            "item_id" => {
                let message_text = part.text.unwrap();
                match message_text.parse_int() {
                    Result::Ok(item_id) => match part.player {
                        Option::Some(player_id) => get_item_name_for_player_by_id(player_id, item_id),
                        Option::None => UNKNOWN_ITEM
                    },
                    Result::Err(e) => UNKNOWN_ITEM
                }
            },
            "location_id" => {
                let message_text = part.text.unwrap();
                match message_text.parse_int() {
                    Result::Ok(location_id) => match part.player {
                        Option::Some(player_id) => get_location_name_for_player_by_id(player_id, location_id),
                        Option::None => UNKNOWN_LOCATION
                    },
                    Result::Err(e) => UNKNOWN_LOCATION
                }
            },
            _ =>  match part.text {
                Option::Some(text) => text,
                Option::None => "",
            }
        };
        message = f"{message}{text}";
    }
    log(message);
    ap_log(List::of(ColorfulText { text: message, color: COLOR_WHITE }));
}

struct ArchipelagoLogMessage {
    texts: List<ColorfulText>,
    timestamp: int
}

struct ArchipelagoLog {
    messages: List<ArchipelagoLogMessage>
}

static mut AP_LOG = ArchipelagoLog { messages: List::new() };

fn ap_log(texts: List<ColorfulText>) {
    // Filter any newlines
    let filtered_texts = List::new();
    for text in texts {
        filtered_texts.push(ColorfulText {
            text: text.text.replace("\n", ""),
            color: text.color
        });
    }

    AP_LOG.messages.push(ArchipelagoLogMessage {
        texts: filtered_texts,
        timestamp: current_time_millis(),
    });
}

static LOG_COUNT = 5;
static LOG_WIDTH = 750.;

static mut AP_LOG_COMPONENT = Component {
    id: ARCHIPELAGO_LOG_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_LOG_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string { text },
    draw_hud_always: fn() {
        let viewport = Tas::get_viewport_size();
        let w = viewport.width.to_float();
        let h = viewport.height.to_float();

        // ensure logs don't run over the width
        let mut i = int::max(0, AP_LOG.messages.len() - LOG_COUNT);
        let mut formatted_texts = List::new();
        while i < AP_LOG.messages.len() {
            let log = AP_LOG.messages.get(i).unwrap();
            for text in log.texts {
                let words = text.text.split(" ");
                let mut line_string = "";
                let mut j = 0;
                while j < words.len() {
                    let word = words.get(j).unwrap();
                    line_string = f"{line_string}{word}";
                    let dim = Tas::get_text_size(line_string, SETTINGS.ui_scale);
                    if dim.width > LOG_WIDTH {
                        line_string = "  {word}";
                        formatted_texts.push(ColorfulText { text: "\n  ", color: COLOR_WHITE });
                    }

                    if j != words.len() - 1 {
                        line_string = f"{line_string} ";
                        formatted_texts.push(ColorfulText { text: f"{word} ", color: text.color });
                    } else {
                        formatted_texts.push(ColorfulText { text: word, color: text.color });
                    }

                    j += 1;
                }
            }
            if i != AP_LOG.messages.len() - 1 {
                formatted_texts.push(ColorfulText { text: "\n", color: COLOR_WHITE });
            }
            i += 1;
        }

        ap_draw_colorful_text(formatted_texts, AP_COLOR_GRAY_BG, 0., h, Anchor::BottomLeft, 5.0);
    },
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
    on_mouse_move: fn(x: int, y: int) {},
    on_component_enter: fn() {},
    on_component_exit: fn() { Tas::archipelago_disconnect(); },
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};