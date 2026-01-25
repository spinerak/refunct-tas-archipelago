struct ArchipelagoLog {
    messages: List<ArchipelagoLogMessage>
}

struct ArchipelagoLogMessage {
    texts: List<ColorfulText>,
    timestamp: int,
    force: bool
}

enum ArchipelagoLogDisplay {
    Off,
    Temporary,
    On
}

enum ArchipelagoLogLevel {
    AllMessages,
    OnlyProgressive,
    NoMessages
}

static mut AP_LOG = ArchipelagoLog { messages: List::new() };

static AP_ITEM_COLORS = List::of(
    Color { red: 0.024, green: 0.851, blue: 0.851, alpha: 1.0 }, // normal
    Color { red: 0.659, green: 0.576, blue: 0.894, alpha: 1.0 }, // progressive
    Color { red: 0.435, green: 0.502, blue: 0.722, alpha: 1.0 }, // useful
    Color { red: 1.000, green: 0.875, blue: 0.000, alpha: 1.0 }, // useful & progressive
    Color { red: 0.827, green: 0.443, blue: 0.400, alpha: 1.0 }, // trap
    Color { red: 1.000, green: 0.675, blue: 0.110, alpha: 1.0 }, // trap & progressive
    Color { red: 0.608, green: 0.349, blue: 0.714, alpha: 1.0 }, // trap & useful
    Color { red: 0.502, green: 1.000, blue: 0.502, alpha: 1.0 }, // trap & useful & progressive
);

static LOG_FADEOUT_MS = 750;

static mut AP_LOG_COMPONENT = Component {
    id: ARCHIPELAGO_LOG_COMPONENT_ID,
    conflicts_with: List::of(ARCHIPELAGO_LOG_COMPONENT_ID, MULTIPLAYER_COMPONENT_ID, NEW_GAME_100_PERCENT_COMPONENT_ID, NEW_GAME_ALL_BUTTONS_COMPONENT_ID, NEW_GAME_NGG_COMPONENT_ID, PRACTICE_COMPONENT_ID, RANDOMIZER_COMPONENT_ID, TAS_COMPONENT_ID, WINDSCREEN_WIPERS_COMPONENT_ID),
    tick_mode: TickMode::DontCare,
    requested_delta_time: Option::None,
    on_tick: fn() {},
    on_yield: fn() {},
    draw_hud_text: fn(text: string) -> string { text },
    draw_hud_always: draw_ap_log_hud,
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
    on_component_exit: fn() { Tas::archipelago_disconnect(); },
    on_resolution_change: fn() {},
    on_menu_open: fn() {},
};

fn ap_log_internal(texts: List<ColorfulText>, force: bool) {
    // Filter any newlines
    let filtered_texts = List::new();
    for text in texts {
        filtered_texts.push(ColorfulText { text: text.text.replace("\n", ""), color: text.color });
    }
    AP_LOG.messages.push(ArchipelagoLogMessage { texts: filtered_texts, timestamp: current_time_millis(), force: force });
}

fn ap_log(texts: List<ColorfulText>) {
    ap_log_internal(texts, false);
}

fn ap_log_force(texts: List<ColorfulText>) {
    ap_log_internal(texts, true);
}

// Convenience functions
fn ap_log_info(text: string)    { ap_log(List::of(ColorfulText { text: text, color: COLOR_WHITE })); }
fn ap_log_warning(text: string) { ap_log(List::of(ColorfulText { text: text, color: AP_COLOR_YELLOW })); }
fn ap_log_error(text: string)   { ap_log_force(List::of(ColorfulText { text: text, color: AP_COLOR_RED })); }

fn archipelago_received_death(source: string, cause: string) {
    let mut message = List::new();
    match cause {
        "" => {
            message.push(ColorfulText { text: source, color: COLOR_RED });
            message.push(ColorfulText { text: " died", color: AP_COLOR_YELLOW });
        },
        _ => {
            for part in cause.split(source) {
                if part != "" {
                    message.push(ColorfulText { text: part, color: AP_COLOR_YELLOW });
                }
                message.push(ColorfulText { text: source, color: COLOR_RED });
            }
            message.pop();
        }
    }

    ap_log(message);
}

fn archipelago_print_json_message(json_message: ReboPrintJSONMessage) {
    let mut message = List::new();

    let mut contains_player = false;
    let mut contains_this_player = false;
    let mut contains_item = false;
    let mut contains_progressive_item = false;

    for part in json_message.data {
        if part._type == "player_id" {
            contains_player = true;
            contains_this_player |= match part.text {
                Option::Some(slot_id_str) => match slot_id_str.parse_int() {
                    Result::Ok(slot) => (slot == ARCHIPELAGO_ROOM_INFO.this_player_slot),
                    Result::Err(e) => false
                },
                Option::None => false,
            };
        } else if part._type == "item_id" {
            contains_item = true;
            contains_progressive_item |= match json_message.item {
                // Can traps be progressive???
                Option::Some(item) => ((1 <= item.flags && item.flags <= 3) || (5 <= item.flags && item.flags <= 7)),
                Option::None => false
            }
        }

        message.push(archipelago_interpret_json_message_part(part, json_message.receiving, json_message.item));
    }

    let player_log_level = SETTINGS.archipelago_log_level_player;
    let others_log_level = SETTINGS.archipelago_log_level_others;
    let should_show_message =
        (!contains_player && !contains_item) ||
        (player_log_level == ArchipelagoLogLevel::AllMessages     && contains_this_player) ||
        (player_log_level == ArchipelagoLogLevel::OnlyProgressive && contains_this_player && contains_progressive_item) ||
        (others_log_level == ArchipelagoLogLevel::AllMessages     && contains_player && !contains_this_player) ||
        (others_log_level == ArchipelagoLogLevel::OnlyProgressive && contains_player && !contains_this_player && contains_progressive_item);

    if should_show_message { ap_log(message); }
}

fn archipelago_interpret_json_message_part(
    part: ReboJSONMessagePart,
    receiving: Option<int>,
    network_item: Option<ReboNetworkItem>
) -> ColorfulText {
    match part._type {
        "player_id" => {
            let team = ARCHIPELAGO_ROOM_INFO.this_player_team;
            let player = match part.text {
                Option::Some(slot_id_str) => match slot_id_str.parse_int() {
                    Result::Ok(slot) => get_team_player(slot),
                    Result::Err(e) => UNKNOWN_PLAYER
                },
                Option::None => UNKNOWN_PLAYER,
            };

            if player.id.slot == 0 {
                // It's the server
                ColorfulText { text: player.name, color: COLOR_RED }
            } else if player.id.slot == ARCHIPELAGO_ROOM_INFO.this_player_slot {
                // It's us!
                ColorfulText { text: player.name, color: AP_COLOR_MAGENTA }
            } else {
                // It's someone else
                ColorfulText { text: player.name, color: AP_COLOR_YELLOW }
            }
        },
        "item_id" => ColorfulText {
            text: match part.text {
                Option::Some(item_id) => match part.player {
                    Option::Some(slot) => get_item_name_for_slot(slot, item_id),
                    Option::None => UNKNOWN_ITEM
                },
                Option::None => UNKNOWN_ITEM,
            },
            color: match network_item {
                Option::Some(item) => match AP_ITEM_COLORS.get(item.flags) {
                    Option::Some(color) => color,
                    Option::None => COLOR_WHITE
                },
                Option::None => COLOR_WHITE
            }
        },
        "location_id" => ColorfulText {
            text: match part.text {
                Option::Some(location_id) =>  match part.player {
                    Option::Some(slot) => get_location_name_for_slot(slot, location_id),
                    Option::None => UNKNOWN_LOCATION
                },
                Option::None => UNKNOWN_LOCATION,
            },
            color: Color {red: 0.0, green: 1.0, blue: 0.5, alpha: 1.0}
        },
        _ => ColorfulText {
            text: match part.text {
                Option::Some(text) => text,
                Option::None => "",
            },
            color: COLOR_WHITE
        }
    }
}

fn draw_ap_log_hud() {
    //if SETTINGS.archipelago_log_display == ArchipelagoLogDisplay::Off { return; }

    let viewport = Tas::get_viewport_size();
    let w = viewport.width.to_float();
    let log_display_width = w * SETTINGS.archipelago_log_display_width;
    let h = viewport.height.to_float();

    // ensure logs don't run over the width
    let mut i = int::max(0, AP_LOG.messages.len() - SETTINGS.archipelago_log_max_count);
    let mut formatted_texts = List::new();
    let now = current_time_millis();
    while i < AP_LOG.messages.len() {
        let log = AP_LOG.messages.get(i).unwrap();
        if SETTINGS.archipelago_log_display == ArchipelagoLogDisplay::Off && !log.force {
            i += 1;
            continue;
        }

        if (SETTINGS.archipelago_log_display == ArchipelagoLogDisplay::Temporary || SETTINGS.archipelago_log_display == ArchipelagoLogDisplay::Off)
        && (now - log.timestamp) > (SETTINGS.archipelago_log_display_time_sec * 1000 + LOG_FADEOUT_MS) {
            i += 1;
            continue;
        }

        for text in log.texts {
            let words = text.text.split(" ");
            let mut line_string = "";
            let mut j = 0;
            while j < words.len() {
                let mut word = words.get(j).unwrap();
                line_string = f"{line_string}{word}";
                let dim = Tas::get_text_size(line_string, SETTINGS.ui_scale);
                if dim.width > log_display_width {
                    line_string = "  {word}";
                    formatted_texts.push(ColorfulText { text: "\n  ", color: COLOR_WHITE });
                }

                if j != words.len() - 1 {
                    // Don't forget the space!
                    line_string = f"{line_string} ";
                    word = f"{word} ";
                }

                formatted_texts.push(ColorfulText {
                    text: word,
                    color: get_faded_log_color(text.color, now, log.timestamp)
                });

                j += 1;
            }
        }
        if i != AP_LOG.messages.len() - 1 {
            formatted_texts.push(ColorfulText { text: "\n", color: COLOR_WHITE });
        }
        i += 1;
    }

    ap_draw_colorful_text(formatted_texts, AP_COLOR_CLEAR, 0., h, Anchor::BottomLeft, 5.0);
}

fn get_faded_log_color(color: Color, now: int, log_timestamp: int) -> Color {
    let delta = now - log_timestamp;
    let display_ms = SETTINGS.archipelago_log_display_time_sec * 1000;

    if SETTINGS.archipelago_log_display == ArchipelagoLogDisplay::Temporary
    && display_ms <= delta && delta <= display_ms + LOG_FADEOUT_MS
    {
        let mut faded_color = color.clone();
        let t = delta - display_ms;
        faded_color.alpha = 1.0 - (t.to_float() / LOG_FADEOUT_MS.to_float());
        faded_color
    } else {
        color
    }
}
