struct ArchipelagoRoomInfo {
    this_player_team: int,
    this_player_slot: int,
    slots: Map<int, ArchipelagoSlot>,
    players: Map<ArchipelagoPlayerId, ArchipelagoPlayer>,
    games: Map<string, ArchipelagoGame>
}

static mut ARCHIPELAGO_ROOM_INFO = ArchipelagoRoomInfo {
    this_player_team: -1,
    this_player_slot: -1,
    slots: Map::new(),
    players: Map::new(),
    games: Map::new()
};

struct ArchipelagoPlayer {
    id: ArchipelagoPlayerId,
    alias: string,
    name: string
}

struct ArchipelagoPlayerId {
    team: int,
    slot: int
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
    item_name_to_id: Map<string, string>,
    item_id_to_name: Map<string, string>,
    location_name_to_id: Map<string, string>,
    location_id_to_name: Map<string, string>
}

static UNKNOWN_PLAYER = ArchipelagoPlayer { id: ArchipelagoPlayerId { team: -1, slot: -1 }, alias: "someone", name: "someone" };
static UNKNOWN_ITEM = "something";
static UNKNOWN_LOCATION = "somewhere";

fn archipelago_set_own_id(team: int, slot: int) {
    ARCHIPELAGO_ROOM_INFO.this_player_team = team;
    ARCHIPELAGO_ROOM_INFO.this_player_slot = slot;
}

fn archipelago_register_slot(index: int, name: string, game: string, type: int, group_members: List<int>) {
    ARCHIPELAGO_ROOM_INFO.slots.insert(index, ArchipelagoSlot {
        slot: index,
        name: name,
        game: game,
        type: type,
        group_members: group_members
    });
}

fn archipelago_register_player(team: int, slot: int, alias: string, name: string) {
    let id = ArchipelagoPlayerId { team: team, slot: slot };
    ARCHIPELAGO_ROOM_INFO.players.insert(id, ArchipelagoPlayer {
        id: id,
        alias: alias,
        name: name,
    });
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
            game
        }
    }
};

fn archipelago_register_game_item(game_name: string, item_name: string, item_id: string) {
    let game_info = archipelago_get_or_create_game_info(game_name);
    let item_name_to_id = game_info.item_name_to_id.insert(item_name, item_id);
    let item_id_to_name = game_info.item_id_to_name.insert(item_id, item_name);
}

fn archipelago_register_game_location(game_name: string, location_name: string, location_id: string) {
    let game_info = archipelago_get_or_create_game_info(game_name);
    let location_name_to_id = game_info.location_name_to_id.insert(location_name, location_id);
    let location_id_to_name = game_info.location_id_to_name.insert(location_id, location_name);
}

fn get_team_player(slot: int) -> ArchipelagoPlayer {
    if slot == 0 {
        ArchipelagoPlayer {
            id: ArchipelagoPlayerId { team: -1, slot: 0 },
            alias: "server",
            name: "server"
        }
    } else {
        let id = ArchipelagoPlayerId {team: ARCHIPELAGO_ROOM_INFO.this_player_team, slot: slot};
        match ARCHIPELAGO_ROOM_INFO.players.get(id) {
            Option::Some(player) => player,
            Option::None => UNKNOWN_PLAYER
        }
    }
}

fn get_item_name_for_slot(slot: int, item_id: string) -> string {
    let game_name = match ARCHIPELAGO_ROOM_INFO.slots.get(slot) {
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

fn get_location_name_for_slot(slot: int, location_id: string) -> string {
    let game_name = match ARCHIPELAGO_ROOM_INFO.slots.get(slot) {
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