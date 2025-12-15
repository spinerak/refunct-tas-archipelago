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
// - Parse some types of messages separately (namely those with a NetworkItem)
//   - Add field to ReboJSONMessage to support this
//   - This will allow filtering messages only relevant to you, etc.
// - Add Message filtering option to menu (List::of(All, OnlyYou, OnlyProgressive, OnlyYouAndProgressive)?)
// - Add prettier logging and a menu option to keep logs for a certain amount of time
//   - Options: Forever, Duration, Off; Duration is controlled by a float setting (10s default?)
//   - Another option to control maximum number of messages shown?
//   - Should the logs have a separate UI scale option? Probably not.
//   - Logs should appear permanently when the player opens the AP menu (maybe?)
//     - Annoyingly, you can leave the menu open. We should maybe fix this?
// - Delete lots of the old logging (no longer needed)

fn archipelago_print_json_message(json_message: ReboJSONMessage) {
    let mut message = "";
    for part in json_message.parts {
        let text = match part.message_type {
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
}