enum ArchipelagoDisplayStyle {
    Off,
    Classic,
    ColorCoded
}

struct ColorfulText {
    text: string,
    color: Color
}

enum Anchor {
    TopLeft,    TopCenter,    TopRight,
    CenterLeft, Center,       CenterRight,
    BottomLeft, BottomCenter, BottomRight
}

static AP_COLOR_RED     = Color { red: 0.600, green: 0.160, blue: 0.227, alpha: 1. };
static AP_COLOR_CYAN    = Color { red: 0.000, green: 0.627, blue: 0.698, alpha: 1. };
static AP_COLOR_GREEN   = Color { red: 0.231, green: 0.600, blue: 0.165, alpha: 1. };
static AP_COLOR_YELLOW  = Color { red: 0.600, green: 0.533, blue: 0.165, alpha: 1. };
static AP_COLOR_MAGENTA = Color { red: 1.000, green: 0.000, blue: 1.000, alpha: 1. };
static AP_COLOR_GRAY_BG = Color { red: 0., green: 0., blue: 0., alpha: 0.6 };
static AP_COLOR_CLEAR   = Color { red: 0., green: 0., blue: 0., alpha: 0.0 };

fn create_archipelago_menu() -> Ui {
    let elements = List::new();

    if !ARCHIPELAGO_STATE.ap_connected {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Connect" },
            onclick: fn(label: Text) {
                enter_ui(create_archipelago_connection_details_menu());
            },
        }));
    }

    if ARCHIPELAGO_STATE.ap_connected {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Change gamemode" },
            onclick: fn(label: Text) {
                enter_ui(create_archipelago_gamemodes_menu());
            },
        }));
    }

    if ARCHIPELAGO_STATE.ap_connected {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Connection Details" },
            onclick: fn(label: Text) {
                enter_ui(create_archipelago_connection_details_menu());
            },
        }));
    }

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Settings" },
        onclick: fn(label: Text) {
            enter_ui(create_archipelago_settings_menu());
        },
    }));

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui(); },
    }));

    // elements.push(
    //     UiElement::Button(UiButton {
    //     label: Text { text: "Map Editor" },
    //     onclick: fn(label: Text) { enter_ui(create_map_editor_menu()); },
    // }));

    Ui::new("Archipelago:", elements)
}

struct ArchipelagoConnectionDetails {
    server: string,
    port: string,
    slot: string,
    password: string,
}

static mut ARCHIPELAGO_CONNECTION_DETAILS = ArchipelagoConnectionDetails {
    server: SETTINGS.archipelago_last_server,
    port: SETTINGS.archipelago_last_port,
    slot: SETTINGS.archipelago_last_slot,
    password: "",
};

fn create_archipelago_connection_details_menu() -> Ui {
    let elements = List::new();

    elements.push(UiElement::Input(Input {
        label: Text { text: "Server" },
        input: f"{ARCHIPELAGO_CONNECTION_DETAILS.server}",
        onclick: fn(input: string) {},
        onchange: fn(input: string) { ARCHIPELAGO_CONNECTION_DETAILS.server = input.trim(); },
    }));
    elements.push(UiElement::Input(Input {
        label: Text { text: "Port" },
        input: f"{ARCHIPELAGO_CONNECTION_DETAILS.port}",
        onclick: fn(input: string) {},
        onchange: fn(input: string) { ARCHIPELAGO_CONNECTION_DETAILS.port = input.trim(); },
    }));
    elements.push(UiElement::Input(Input {
        label: Text { text: "Slot" },
        input: f"{ARCHIPELAGO_CONNECTION_DETAILS.slot}",
        onclick: fn(input: string) {},
        onchange: fn(input: string) { ARCHIPELAGO_CONNECTION_DETAILS.slot = input.trim(); },
    }));
    elements.push(UiElement::Input(Input {
        label: Text { text: "Password" },
        input: f"{ARCHIPELAGO_CONNECTION_DETAILS.password}",
        onclick: fn(input: string) {},
        onchange: fn(input: string) { ARCHIPELAGO_CONNECTION_DETAILS.password = input; },
    }));

    if ARCHIPELAGO_STATE.ap_connected {
        elements.push(UiElement::Chooser(Chooser {
            label: Text { text: "Death Link" },
            options: List::of(Text { text: "On" }, Text { text: "Off" }),
            selected: if Tas::is_death_link_on() { 0 } else { 1 },
            onchange: fn(index: int) {
                if index == 0 {
                    Tas::set_death_link(true);
                } else {
                    Tas::set_death_link(false);
                }
            },
        }));
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Disconnect" },
            onclick: fn(label: Text) {
                Tas::archipelago_disconnect();
                remove_component(ARCHIPELAGO_COMPONENT);
                add_component(ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT);
                ARCHIPELAGO_STATE.ap_connected = false;
                leave_ui(); leave_ui();
            },
        }));
    } else {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Connect" },
            onclick: fn(label: Text) {
                ARCHIPELAGO_STATE = fresh_archipelago_state();
                let password = ARCHIPELAGO_CONNECTION_DETAILS.password;
                ARCHIPELAGO_CONNECTION_DETAILS.password = "";
                Tas::archipelago_connect(
                    f"{ARCHIPELAGO_CONNECTION_DETAILS.server}:{ARCHIPELAGO_CONNECTION_DETAILS.port}",
                    "Refunct",
                    ARCHIPELAGO_CONNECTION_DETAILS.slot,
                    if password == "" { Option::None } else { Option::Some(password) }
                );
                SETTINGS.archipelago_last_server = ARCHIPELAGO_CONNECTION_DETAILS.server;
                SETTINGS.archipelago_last_port = ARCHIPELAGO_CONNECTION_DETAILS.port;
                SETTINGS.archipelago_last_slot = ARCHIPELAGO_CONNECTION_DETAILS.slot;
                SETTINGS.store();
                remove_component(ARCHIPELAGO_DISCONNECTED_INFO_COMPONENT);
                add_component(ARCHIPELAGO_COMPONENT);
                leave_ui(); leave_ui(); // Now we're two levels deep
            },
        }));
    }

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui(); },
    }));

    Ui::new("Connection Details", elements)
}

fn create_archipelago_settings_menu() -> Ui {
    Ui::new("Settings", List::of(
        UiElement::FloatInput(FloatInput {
            label: Text { text: "UI Scale (0.2 - 1.0)" },
            input: f"{SETTINGS.ui_scale}",
            onclick: fn(input: string) {},
            onchange: fn(input: string) {
                match input.parse_float() {
                    Result::Ok(size) => if 0.2 <= size && size <= 1.0 {
                        SETTINGS.ui_scale = size;
                        SETTINGS.store();
                    },
                    Result::Err(e) => {},
                }
            },
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "AP Display Style" },
            options: List::of(
                Text { text: "Off" },
                Text { text: "Classic (shown on main menu)" },
                Text { text: "Color Coded" }
            ),
            selected: match SETTINGS.archipelago_display_style {
                ArchipelagoDisplayStyle::Off => 0,
                ArchipelagoDisplayStyle::Classic => 1,
                ArchipelagoDisplayStyle::ColorCoded => 2
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_display_style = ArchipelagoDisplayStyle::Off },
                    1 => { SETTINGS.archipelago_display_style = ArchipelagoDisplayStyle::Classic },
                    2 => { SETTINGS.archipelago_display_style = ArchipelagoDisplayStyle::ColorCoded },
                    _ => panic(f"unknown display_style index {index}"),
                }
                SETTINGS.store();
            }
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "AP Display Position" },
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
                Anchor::TopLeft => 0,
                Anchor::TopCenter => 1,
                Anchor::TopRight => 2,
                Anchor::CenterRight => 3,
                Anchor::BottomRight => 4,
                Anchor::BottomCenter => 5,
                Anchor::BottomLeft => 6,
                Anchor::CenterLeft => 7,
                pos => panic(f"unknown archipelago display position: {pos}")
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_display_position = Anchor::TopLeft; },
                    1 => { SETTINGS.archipelago_display_position = Anchor::TopCenter; },
                    2 => { SETTINGS.archipelago_display_position = Anchor::TopRight; },
                    3 => { SETTINGS.archipelago_display_position = Anchor::CenterRight; },
                    4 => { SETTINGS.archipelago_display_position = Anchor::BottomRight; },
                    5 => { SETTINGS.archipelago_display_position = Anchor::BottomCenter; },
                    6 => { SETTINGS.archipelago_display_position = Anchor::BottomLeft; },
                    7 => { SETTINGS.archipelago_display_position = Anchor::CenterLeft; },
                    _ => panic(f"unknown archipelago display index: {index}"),
                }
                SETTINGS.store();
            },
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Platform Display" },
            options: List::of(Text { text: "On" }, Text { text: "Off" }),
            selected: if SETTINGS.platform_display_enabled { 0 } else { 1 },
            onchange: fn(index: int) {
                if index == 0 {
                    SETTINGS.platform_display_enabled = true;
                } else {
                    SETTINGS.platform_display_enabled = false;
                }
                SETTINGS.store();
            },
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Platform Display Position" },
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
            selected: match SETTINGS.platform_display_position {
                Anchor::TopLeft => 0,
                Anchor::TopCenter => 1,
                Anchor::TopRight => 2,
                Anchor::CenterRight => 3,
                Anchor::BottomRight => 4,
                Anchor::BottomCenter => 5,
                Anchor::BottomLeft => 6,
                Anchor::CenterLeft => 7,
                pos => panic(f"unknown archipelago display position: {pos}")
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.platform_display_position = Anchor::TopLeft; },
                    1 => { SETTINGS.platform_display_position = Anchor::TopCenter; },
                    2 => { SETTINGS.platform_display_position = Anchor::TopRight; },
                    3 => { SETTINGS.platform_display_position = Anchor::CenterRight; },
                    4 => { SETTINGS.platform_display_position = Anchor::BottomRight; },
                    5 => { SETTINGS.platform_display_position = Anchor::BottomCenter; },
                    6 => { SETTINGS.platform_display_position = Anchor::BottomLeft; },
                    7 => { SETTINGS.platform_display_position = Anchor::CenterLeft; },
                    _ => panic(f"unknown archipelago display index: {index}"),
                }
                SETTINGS.store();
            },
        }),
        UiElement::Button(UiButton { label: Text { text: "--" }, onclick: fn(label: Text) {} }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Minimap" },
            options: List::of(Text { text: "On" }, Text { text: "Off" }),
            selected: if SETTINGS.minimap_enabled { 0 } else { 1 },
            onchange: fn(index: int) {
                if index == 0 {
                    add_component(MINIMAP_COMPONENT);
                    SETTINGS.minimap_enabled = true;
                    SETTINGS.store();
                } else {
                    remove_component(MINIMAP_COMPONENT);
                    SETTINGS.minimap_enabled = false;
                    SETTINGS.store();
                }
            },
        }),
        UiElement::FloatInput(FloatInput {
            label: Text { text: "Minimap Size (0.0-1.0) " },
            input: f"{MINIMAP_STATE.size}",
            onclick: fn(input: string) {},
            onchange: fn(input: string) {
                match input.parse_float() {
                    Result::Ok(size) => if 0.0 <= size && size <= 1.0 {
                        MINIMAP_STATE.calculate_minimap_size(size);
                        SETTINGS.minimap_size = size;
                        SETTINGS.store();
                    },
                    Result::Err(e) => {},
                }
            },
        }),
        UiElement::FloatInput(FloatInput {
            label: Text { text: "Minimap Alpha (0.0 - 1.0)" },
            input: f"{MINIMAP_STATE.alpha}",
            onclick: fn(input: string) {},
            onchange: fn(input: string) {
                match input.parse_float() {
                    Result::Ok(alpha) => if 0.0 <= alpha && alpha <= 1.0 {
                        MINIMAP_STATE.alpha = alpha;
                        Tas::set_minimap_alpha(alpha);
                        SETTINGS.minimap_alpha = alpha;
                        SETTINGS.store();
                    },
                    Result::Err(e) => {},
                }
            },
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Minimap Position" },
            options: List::of(
                Text { text: "Top Left" },
                Text { text: "Top Center" },
                Text { text: "Top Right" },
                Text { text: "Center Right" },
                Text { text: "Bottom Right" },
                Text { text: "Bottom Center" },
                Text { text: "Bottom Left" },
                Text { text: "Center Left" },
                Text { text: "Center" },
            ),
            selected: match SETTINGS.minimap_position {
                MinimapPosition::TopLeft => 0,
                MinimapPosition::TopCenter => 1,
                MinimapPosition::TopRight => 2,
                MinimapPosition::CenterRight => 3,
                MinimapPosition::BottomRight => 4,
                MinimapPosition::BottomCenter => 5,
                MinimapPosition::BottomLeft => 6,
                MinimapPosition::CenterLeft => 7,
                MinimapPosition::Center => 8,
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.minimap_position = MinimapPosition::TopLeft; },
                    1 => { SETTINGS.minimap_position = MinimapPosition::TopCenter; },
                    2 => { SETTINGS.minimap_position = MinimapPosition::TopRight; },
                    3 => { SETTINGS.minimap_position = MinimapPosition::CenterRight; },
                    4 => { SETTINGS.minimap_position = MinimapPosition::BottomRight; },
                    5 => { SETTINGS.minimap_position = MinimapPosition::BottomCenter; },
                    6 => { SETTINGS.minimap_position = MinimapPosition::BottomLeft; },
                    7 => { SETTINGS.minimap_position = MinimapPosition::CenterLeft; },
                    8 => { SETTINGS.minimap_position = MinimapPosition::Center; },
                    _ => panic(f"unknown index {index}"),
                }
                SETTINGS.store();
                MINIMAP_STATE.calculate_minimap_size(MINIMAP_STATE.size);
            },
        }),
        UiElement::Button(UiButton { label: Text { text: "--" }, onclick: fn(label: Text) {} }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Show Logs" },
            options: List::of(
                Text { text: "Temporarily" },
                Text { text: "Forever" },
                Text { text: "Never" },
            ),
            selected: match SETTINGS.archipelago_log_display {
                ArchipelagoLogDisplay::Temporary => 0,
                ArchipelagoLogDisplay::On => 1,
                ArchipelagoLogDisplay::Off => 2,
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_log_display = ArchipelagoLogDisplay::Temporary; },
                    1 => { SETTINGS.archipelago_log_display = ArchipelagoLogDisplay::On; },
                    2 => { SETTINGS.archipelago_log_display = ArchipelagoLogDisplay::Off; },
                    _ => panic(f"unknown index {index}"),
                };
                SETTINGS.store();
            },
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Items you send/receive" },
            options: List::of(
                Text { text: "Show All" },
                Text { text: "Show Only Progressive" },
                Text { text: "Show None" },
            ),
            selected: match SETTINGS.archipelago_log_level_player {
                ArchipelagoLogLevel::AllMessages => 0,
                ArchipelagoLogLevel::OnlyProgressive => 1,
                ArchipelagoLogLevel::NoMessages => 2,
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_log_level_player = ArchipelagoLogLevel::AllMessages; },
                    1 => { SETTINGS.archipelago_log_level_player = ArchipelagoLogLevel::OnlyProgressive; },
                    2 => { SETTINGS.archipelago_log_level_player = ArchipelagoLogLevel::NoMessages; },
                    _ => panic(f"unknown index {index}"),
                };
                SETTINGS.store();
            },
        }),
        UiElement::Chooser(Chooser {
            label: Text { text: "Items others send/receive" },
            options: List::of(
                Text { text: "Show All" },
                Text { text: "Show Only Progressive" },
                Text { text: "Show None" },
            ),
            selected: match SETTINGS.archipelago_log_level_others {
                ArchipelagoLogLevel::AllMessages => 0,
                ArchipelagoLogLevel::OnlyProgressive => 1,
                ArchipelagoLogLevel::NoMessages => 2,
            },
            onchange: fn(index: int) {
                match index {
                    0 => { SETTINGS.archipelago_log_level_others = ArchipelagoLogLevel::AllMessages; },
                    1 => { SETTINGS.archipelago_log_level_others = ArchipelagoLogLevel::OnlyProgressive; },
                    2 => { SETTINGS.archipelago_log_level_others = ArchipelagoLogLevel::NoMessages; },
                    _ => panic(f"unknown index {index}"),
                };
                SETTINGS.store();
            },
        }),
        UiElement::FloatInput(FloatInput {
            label: Text { text: "Max Log Count" },
            input: f"{SETTINGS.archipelago_log_max_count}",
            onclick: fn(input: string) {},
            onchange: fn(input: string) {
                match input.parse_int() {
                    Result::Ok(count) => if 0 <= count && count <= 100 {
                        SETTINGS.archipelago_log_max_count = count;
                        SETTINGS.store();
                    },
                    Result::Err(e) => {},
                };
            },
        }),
        UiElement::FloatInput(FloatInput {
            label: Text { text: "Log Temporary Display Time (sec)" },
            input: f"{SETTINGS.archipelago_log_display_time_sec}",
            onclick: fn(input: string) {},
            onchange: fn(input: string) {
                match input.parse_int() {
                    Result::Ok(t) => if 0 <= t {
                        SETTINGS.archipelago_log_display_time_sec = t;
                        SETTINGS.store();
                    },
                    Result::Err(e) => {},
                };
            },
        }),
        UiElement::FloatInput(FloatInput {
            label: Text { text: "Log Display Width (0.0 - 1.0)" },
            input: f"{SETTINGS.archipelago_log_display_width}",
            onclick: fn(input: string) {},
            onchange: fn(input: string) {
                match input.parse_float() {
                    Result::Ok(size) => if 0.0 <= size && size <= 1.0 {
                        SETTINGS.archipelago_log_display_width = size;
                        SETTINGS.store();
                    },
                    Result::Err(e) => {},
                };
            },
        }),
        UiElement::Button(UiButton { label: Text { text: "--" }, onclick: fn(label: Text) {} }),
        UiElement::Button(UiButton {
            label: Text { text: "Back" },
            onclick: fn(label: Text) { leave_ui(); },
        })
    ))
}

fn create_list_of_minigames_with_checks(txt: string) -> List<ColorfulText> {
    let lines = List::new();
    let mut added_minigame_header = false;
    if ARCHIPELAGO_STATE.unlock_vanilla_minigame && !ARCHIPELAGO_STATE.done_vanilla_minigame {
        if !added_minigame_header {
            lines.push(ColorfulText { text: txt, color: COLOR_WHITE });
        }
        lines.push(ColorfulText {
            text:  "\nVanilla",
            color: AP_COLOR_GREEN
        });
        added_minigame_header = true;
    }
    if ARCHIPELAGO_STATE.unlock_seeker_minigame && !ARCHIPELAGO_STATE.done_seeker_minigame {
        if !added_minigame_header {
            lines.push(ColorfulText { text: txt, color: COLOR_WHITE });
        }
        lines.push(ColorfulText {
            text:  "\nSeeker",
            color: AP_COLOR_GREEN
        });
        added_minigame_header = true;
    }
    if ARCHIPELAGO_STATE.unlock_button_galore_minigame && !ARCHIPELAGO_STATE.done_button_galore_minigame {
        if !added_minigame_header {
            lines.push(ColorfulText { text: txt, color: COLOR_WHITE });
        }
        lines.push(ColorfulText {
            text:  "\nButton Galore",
            color: AP_COLOR_GREEN
        });
        added_minigame_header = true;
    }
    if ARCHIPELAGO_STATE.unlock_OG_randomizer && !ARCHIPELAGO_STATE.done_OG_randomizer_minigame {
        if !added_minigame_header {
            lines.push(ColorfulText { text: txt, color: COLOR_WHITE });
        }
        lines.push(ColorfulText {
            text:  "\nOG Randomizer",
            color: AP_COLOR_GREEN
        });
        added_minigame_header = true;
    }

    if ARCHIPELAGO_STATE.block_brawl_check_in_logic > 0 {
        if !added_minigame_header {
            lines.push(ColorfulText { text: txt, color: COLOR_WHITE });
        }
        lines.push(ColorfulText {
            text:  f"\nBlock Brawl: {ARCHIPELAGO_STATE.block_brawl_check_in_logic} in logic",
            color: AP_COLOR_GREEN
        });
        added_minigame_header = true;
    }
    lines
}

fn create_archipelago_gamemodes_menu() -> Ui {
    Ui::new("Select Game Mode:", List::of(
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
        UiElement::Button(UiButton {
            label: Text { text: {
                if ARCHIPELAGO_STATE.unlock_button_galore_minigame {
                    "Button Galore"
                } else {
                    "Button Galore (locked)"
                }
            } },
            onclick: fn(label: Text) {
                if !ARCHIPELAGO_STATE.unlock_button_galore_minigame {
                    // log("Button Galore gamemode is locked!");
                    return;
                }
                // log("Set gamemode to Button Galore");
                archipelago_init(2);
                leave_ui();
            },
        }),
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
            label: Text { text: {
                if ARCHIPELAGO_STATE.unlock_OG_randomizer {
                    "OG Randomizer"
                } else {
                    "OG Randomizer (locked)"
                }
            } },
            onclick: fn(label: Text) {
                if !ARCHIPELAGO_STATE.unlock_OG_randomizer {
                    // log("OG Randomizer gamemode is locked!");
                    return;
                }
                // log("Set gamemode to OG Randomizer");
                archipelago_init(4);
                leave_ui();
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: {
                if ARCHIPELAGO_STATE.unlock_block_brawl {
                    "Block Brawl"
                } else {
                    "Block Brawl (locked)"
                }
            } },
            onclick: fn(label: Text) { 
                if !ARCHIPELAGO_STATE.unlock_block_brawl {
                    // log("Block Brawl gamemode is locked!");
                    return;
                }
                // log("Set gamemode to Block Brawl");
                archipelago_init(5); 
                leave_ui(); 
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: "[test] The Climb: Line" },
            onclick: fn(label: Text) { 
                archipelago_init(6); 
                leave_ui(); 
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: "[test] The Climb: Spiral" },
            onclick: fn(label: Text) { 
                archipelago_init(7); 
                leave_ui(); 
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: "[test] The Climb: Random" },
            onclick: fn(label: Text) { 
                archipelago_init(8); 
                leave_ui(); 
            },
        }),
        UiElement::Button(UiButton {
            label: Text { text: "Back" },
            onclick: fn(label: Text) { leave_ui(); },
        }),
    ))
}

fn get_status_text_lines() -> List<ColorfulText> {
    let mut height = 0.;
    if ARCHIPELAGO_STATE.gamemode == 6 || ARCHIPELAGO_STATE.gamemode == 7 || ARCHIPELAGO_STATE.gamemode == 8 {
        let loc = Tas::get_location();
        height = loc.z;
    }
  
    let lines = List::new();

    if INPUT_MODE_IS_UI_ONLY {
        List::extend(lines, List::of(
            ColorfulText { text: "Movement input disabled\n", color: AP_COLOR_RED },
            ColorfulText { text: "Press F1 to enable movement input", color: AP_COLOR_RED },
            ColorfulText { text: "\n\n", color: AP_COLOR_RED },
        ));
    }

    List::extend(lines, match ARCHIPELAGO_STATE.started {
        0 => List::of(
            ColorfulText { text: "Archipelago Randomizer\n", color: COLOR_WHITE },
            ColorfulText { text: "Press new game (in Refunct menu).", color: AP_COLOR_CYAN },
        ),
        1 => List::of(
            ColorfulText { text: "Archipelago Randomizer\n", color: COLOR_WHITE },
            ColorfulText { text: "Touch a platform to start!", color: AP_COLOR_CYAN },
        ),
        _ => match ARCHIPELAGO_STATE.gamemode {
            0 => get_move_rando_status_lines(),
            1 => List::of(
                ColorfulText { text: "Archipelago - Vanilla\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Press the buttons!", color: AP_COLOR_CYAN },
                ColorfulText { text: f"\nProgress: {ARCHIPELAGO_STATE.progress_vanilla_minigame}", color: COLOR_WHITE },
            ),
            2 => List::of(
                ColorfulText { text: "Archipelago - Button Galore\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Press the buttons!", color: AP_COLOR_CYAN },
                ColorfulText { text: f"\nProgress: {ARCHIPELAGO_STATE.progress_button_galore_minigame}", color: COLOR_WHITE },
            ),
            3 => List::of(
                ColorfulText { text: "Archipelago - Seeker\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Find the empty platforms!\n", color: AP_COLOR_CYAN },
                ColorfulText { text: "Final platform is not a check\n", color: COLOR_WHITE },
                ColorfulText { text: "but pressing it gives a nice view!", color: COLOR_WHITE },
                ColorfulText { text: f"\nProgress: {ARCHIPELAGO_STATE.progress_seeker_minigame}", color: COLOR_WHITE },
            ),
            4 => List::of(
                ColorfulText { text: "Archipelago - OG Randomizer\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Press the buttons!", color: AP_COLOR_CYAN },
                ColorfulText { text: f"\nProgress: {ARCHIPELAGO_STATE.progress_OG_randomizer_minigame}", color: COLOR_WHITE },
            ),
            5 => List::of(
                ColorfulText { text: "Archipelago - Block Brawl\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: cubes => points => checks!", color: AP_COLOR_GREEN },
                ColorfulText { text: f"\n\nNew game => new layout + cubes\nScores will be saved", color: COLOR_WHITE},

                ColorfulText { text: f"\n\nCurrent scores: ", color: COLOR_WHITE },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.score_block_brawl_reds:3} ", color: AP_COLOR_RED },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.score_block_brawl_blues:3} ", color: AP_COLOR_CYAN },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.score_block_brawl_greens:3} ", color: AP_COLOR_GREEN },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.score_block_brawl_yellows:3} ", color: AP_COLOR_YELLOW },

                ColorfulText { text: f"\nScore in logic: ", color: COLOR_WHITE },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.in_logic_block_brawl_reds:3} ", color: AP_COLOR_RED },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.in_logic_block_brawl_blues:3} ", color: AP_COLOR_CYAN },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.in_logic_block_brawl_greens:3} ", color: AP_COLOR_GREEN },
                ColorfulText { text: f"{ARCHIPELAGO_STATE.in_logic_block_brawl_yellows:3} ", color: AP_COLOR_YELLOW },
                ColorfulText { text: f"\nCubes collected: {ARCHIPELAGO_STATE.block_brawl_cubes_collected} / {ARCHIPELAGO_STATE.block_brawl_cubes_total}", color: COLOR_WHITE },

                ColorfulText { text: f"\nCombo: next cube is worth {ARCHIPELAGO_STATE.score_for_next_block} pts", color: COLOR_WHITE },
            ),
            6 => List::of(
                ColorfulText { text: "Archipelago - The Climb: Curve\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Reach 100m!\n", color: AP_COLOR_CYAN },
                ColorfulText { text: f"Current height: {height/100.0:3.0}m", color: AP_COLOR_CYAN },
            ),
            7 => List::of(
                ColorfulText { text: "Archipelago - The Climb: Spiral\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Reach 100m!\n", color: AP_COLOR_CYAN },
                ColorfulText { text: f"Current height: {height/100.0:3.0}m", color: AP_COLOR_CYAN },
            ),
            8 => List::of(
                ColorfulText { text: "Archipelago - The Climb: Random\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Reach 100m!\n", color: AP_COLOR_CYAN },
                ColorfulText { text: f"Current height: {height/100.0:3.0}m", color: AP_COLOR_CYAN },
            ),

            _ => List::of(
                ColorfulText { text: "Archipelago\n", color: COLOR_WHITE },
                ColorfulText { text: "Unknown Gamemode", color: AP_COLOR_RED },
            ),
        }
    });

    let mut txt = "Checks in:";
    if ARCHIPELAGO_STATE.gamemode == 0 {
        txt = "\n\nMinigames with checks:";
    }
    if (ARCHIPELAGO_STATE.gamemode != 0 || ARCHIPELAGO_STATE.started == 0) && ARCHIPELAGO_STATE.grass >= 0 {
        lines.push(ColorfulText {
            text:  f"\n\n[Grass {ARCHIPELAGO_STATE.grass}/{ARCHIPELAGO_STATE.required_grass}] ",
            color: if ARCHIPELAGO_STATE.grass >= ARCHIPELAGO_STATE.required_grass { AP_COLOR_GREEN } else { AP_COLOR_CYAN }
        });
    }
    lines.extend(create_list_of_minigames_with_checks(txt));

    if ARCHIPELAGO_STATE.apworld_version != ARCHIPELAGO_STATE.mod_version {
        lines.push(ColorfulText {
            text:  "\n\nVERSION MISMATCH",
            color: AP_COLOR_RED
        });
        lines.push(ColorfulText {
            text:  f"\nMOD: {ARCHIPELAGO_STATE.mod_version}, APWORLD: {ARCHIPELAGO_STATE.apworld_version}",
            color: AP_COLOR_RED
        });
    }else{
        if ARCHIPELAGO_STATE.started == 0 {
            lines.push(ColorfulText {
                text:  f"\n\nVersion {ARCHIPELAGO_STATE.apworld_version}",
                color: COLOR_WHITE
            });
        }
    }
    lines
}

fn get_move_rando_status_lines() -> List<ColorfulText> {
    let final_platform = if ARCHIPELAGO_STATE.final_platform_known { f"{ARCHIPELAGO_STATE.final_platform_c}-{ARCHIPELAGO_STATE.final_platform_p}" } else { "??-??" };
    let ledge_grab_state = if ARCHIPELAGO_STATE.ledge_grab { "✔" } else { "✖" };
    let wall_jump_state = if ARCHIPELAGO_STATE.wall_jump >= 2 { "∞" } else if ARCHIPELAGO_STATE.wall_jump == 1 { "1" } else { "✖" };
    let jump_pads_state = if ARCHIPELAGO_STATE.jump_pads { "✔" } else { "✖" };
    let swim_state = if ARCHIPELAGO_STATE.swim { "✔" } else { "✖" };
    let lifts_state = if ARCHIPELAGO_STATE.lifts { "✔" } else { "✖" };
    let pipes_state = if ARCHIPELAGO_STATE.pipes { "✔" } else { "✖" };

    let vanilla_state = if ARCHIPELAGO_STATE.unlock_vanilla_minigame { "YES" } else { "NO" };
    let seeker_state = if ARCHIPELAGO_STATE.unlock_seeker_minigame { "YES" } else { "NO" };

    let lines = List::of(
        ColorfulText { text: "Archipelago - Move Rando\nGoals\n", color: COLOR_WHITE },
        ColorfulText {
            text:  f"Get Grass: {ARCHIPELAGO_STATE.grass}/{ARCHIPELAGO_STATE.required_grass}\n",
            color: if ARCHIPELAGO_STATE.grass >= ARCHIPELAGO_STATE.required_grass { AP_COLOR_GREEN } else { AP_COLOR_CYAN }
        },
        ColorfulText {
            text:  f"Go to Platform {final_platform}\n\n",
            color: if ARCHIPELAGO_STATE.has_goaled { AP_COLOR_GREEN } else { AP_COLOR_CYAN }
        },
        ColorfulText { text: "Abilities\n", color: COLOR_WHITE },
        ColorfulText {
            text:  f"{ledge_grab_state} Ledge Grab  ",
            color: if ARCHIPELAGO_STATE.ledge_grab  { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"{jump_pads_state} Jump Pads\n",
            color: if ARCHIPELAGO_STATE.jump_pads { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  if ARCHIPELAGO_STATE.wall_jump >= 2 { f"{wall_jump_state} Wall Jumps  " } else { f"{wall_jump_state} Wall Jump   " },
            color: if ARCHIPELAGO_STATE.wall_jump >= 2 { AP_COLOR_GREEN } else if ARCHIPELAGO_STATE.wall_jump == 1 { AP_COLOR_YELLOW } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"{swim_state} Swim\n",
            color: if ARCHIPELAGO_STATE.swim { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"{lifts_state} Lifts       ",
            color: if ARCHIPELAGO_STATE.lifts { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"{pipes_state} Pipes",
            color: if ARCHIPELAGO_STATE.pipes { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
    );
    lines
}

fn archipelago_hud_text(text: string) -> string {
    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::Classic { return text; }

    let mut hud_text = "";
    for line in get_status_text_lines() {
        let text = line.text;
        hud_text = f"{hud_text}{text}";
    }
    hud_text
}

fn archipelago_hud_color_coded() {
    let viewport = Tas::get_viewport_size();
    let w = viewport.width.to_float();
    let h = viewport.height.to_float();

    if SETTINGS.platform_display_enabled {

        let player_loc = Tas::get_location();
        let player_vel = Tas::get_velocity();
        let platform_text = match ARCHIPELAGO_STATE.last_platform_c {
            Option::Some(cluster) => {
                let platform = ARCHIPELAGO_STATE.last_platform_p.unwrap();
                f"Last Platform: {cluster}-{platform}"
            },
            Option::None => "Last Platform: ??-??"
        };

        let line = List::of(ColorfulText { text: platform_text, color: COLOR_WHITE });
        let anchor = SETTINGS.platform_display_position;
        match anchor {
            Anchor::TopLeft => {
                // Make sure we don't overlap with the menu
                let text_y = match UI_STACK.last() {
                    Option::Some(ui) => {
                        let size = Tas::get_text_size("x", SETTINGS.ui_scale);
                        let line_height = size.height;

                        let elements = ui.elements;
                        line_height * (elements.len().to_float() + 1.5)
                    },
                    Option::None => 0.0,
                };

                ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, 5.0, text_y, anchor, 5.0);
            },
            Anchor::TopCenter    => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, w/2.0, 5.0, anchor, 5.0); },
            Anchor::TopRight     => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, w, 5.0, anchor, 5.0); },
            Anchor::CenterRight  => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, w, h/2.0, anchor, 5.0); },
            Anchor::BottomRight  => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, w, h, anchor, 5.0); },
            Anchor::BottomCenter => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, w/2.0, h, anchor, 5.0); },
            Anchor::BottomLeft   => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, 5.0, h, anchor, 5.0); },
            Anchor::CenterLeft   => { ap_draw_colorful_text(line, AP_COLOR_GRAY_BG, 5.0, h/2.0, anchor, 5.0); },

            pos => panic(f"unknown/invalid archipelago display position: {pos}"),
        };
    }

    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::ColorCoded { return; }

    let lines = get_status_text_lines();
    let anchor = SETTINGS.archipelago_display_position;
    match anchor {
        Anchor::TopLeft => {
            // Make sure we don't overlap with the menu
            let text_y = match UI_STACK.last() {
                Option::Some(ui) => {
                    let size = Tas::get_text_size("x", SETTINGS.ui_scale);
                    let line_height = size.height;

                    let elements = ui.elements;
                    line_height * (elements.len().to_float() + 1.5)
                },
                Option::None => 0.0,
            };

            ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, text_y, anchor, 5.0);
        },
        Anchor::TopCenter    => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w/2.0, 5.0, anchor, 5.0); },
        Anchor::TopRight     => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, 5.0, anchor, 5.0); },
        Anchor::CenterRight  => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, h/2.0, anchor, 5.0); },
        Anchor::BottomRight  => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, h, anchor, 5.0); },
        Anchor::BottomCenter => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w/2.0, h, anchor, 5.0); },
        Anchor::BottomLeft   => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, h, anchor, 5.0); },
        Anchor::CenterLeft   => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, h/2.0, anchor, 5.0); },

        pos => panic(f"unknown/invalid archipelago display position: {pos}"),
    };
}

fn ap_draw_colorful_text(text_list: List<ColorfulText>, background_color: Color, x_pos: float, y_pos: float, anchor: Anchor, padding: float) {
    // Pre-process the lines
    let mut all_text = "";
    for text in text_list {
        all_text = f"{all_text}{text.text}";
    }
    let lines = all_text.split("\n");

    let mut text_width = 0.0;
    for line in lines {
        let text_size = Tas::get_text_size(line, SETTINGS.ui_scale);
        text_width = float::max(text_size.width, text_width);
    }

    let _size = Tas::get_text_size("TEST", SETTINGS.ui_scale);
    let line_height = _size.height;
    let text_height = line_height * lines.len().to_float();

    let mut x_start = 0.0;
    let mut y_start = 0.0;
    match anchor {
        Anchor::TopLeft      => { x_start = x_pos;                  y_start = y_pos; },
        Anchor::TopCenter    => { x_start = x_pos - text_width/2.0; y_start = y_pos; },
        Anchor::TopRight     => { x_start = x_pos - text_width;     y_start = y_pos; },
        Anchor::CenterLeft   => { x_start = x_pos;                  y_start = y_pos - text_height/2.0; },
        Anchor::Center       => { x_start = x_pos - text_width/2.0; y_start = y_pos - text_height/2.0; },
        Anchor::CenterRight  => { x_start = x_pos - text_width;     y_start = y_pos - text_height/2.0; },
        Anchor::BottomLeft   => { x_start = x_pos;                  y_start = y_pos - text_height; },
        Anchor::BottomCenter => { x_start = x_pos - text_width/2.0; y_start = y_pos - text_height; },
        Anchor::BottomRight  => { x_start = x_pos - text_width;     y_start = y_pos - text_height; },
    };

    Tas::draw_rect(background_color,
        x_start - padding, y_start - padding,
        text_width + padding*2.0, text_height + padding*2.0);

    let mut x = x_start;
    let mut y = y_start;
    for text in text_list {
        let lines = text.text.split("\n");
        let mut i = 0;
        while i < lines.len() {
            let line = lines.get(i).unwrap();
            if line != "" {
                Tas::draw_text(DrawText {
                    text: line, color: text.color,
                    x: x, y: y, scale: SETTINGS.ui_scale, scale_position: false
                });
                let text_size = Tas::get_text_size(line, SETTINGS.ui_scale);
                x += text_size.width;
            }

            i += 1;
            if i != lines.len() {
                // The last line didn't end in a newline character
                x = x_start;
                y += line_height;
            }
        }
    }
}

fn archipelago_disconnected_info_hud() {
    let viewport = Tas::get_viewport_size();
    let w = viewport.width.to_float();
    let h = viewport.height.to_float();
    let lines = List::new();
    if INPUT_MODE_IS_UI_ONLY {
        lines.push(ColorfulText { text: "Movement input disabled\n", color: AP_COLOR_RED });
        lines.push(ColorfulText { text: "Press F1 to enable movement input\n\n", color: AP_COLOR_RED });
    } 

    lines.push(ColorfulText { text: "Archipelago Randomizer\n\n", color: COLOR_WHITE });

    lines.push(ColorfulText { text: "Not connected to an Archipelago server yet,\nuse the menu to ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "log in\n\n", color: AP_COLOR_YELLOW });

    lines.push(ColorfulText { text: "Once logged in, use ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "Change gamemode ", color: AP_COLOR_YELLOW });
    lines.push(ColorfulText { text: "to switch\nbetween the ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "main rando ", color: AP_COLOR_YELLOW });
    lines.push(ColorfulText { text: "and the ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "minigames\n\n", color: AP_COLOR_YELLOW });

    lines.push(ColorfulText { text: "Visit settings to change ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "UI layout,\n", color: AP_COLOR_YELLOW });
    lines.push(ColorfulText { text: "show the ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "minimap ", color: AP_COLOR_YELLOW });
    lines.push(ColorfulText { text: "or adjust ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "logs\n\n", color: AP_COLOR_YELLOW });

    lines.push(ColorfulText { text: "If you have issues with background input,\n", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "hit F1 to ", color: COLOR_WHITE });
    lines.push(ColorfulText { text: "enable/disable movement input", color: AP_COLOR_YELLOW });
    
    let anchor = SETTINGS.archipelago_display_position;
    match anchor {
        Anchor::TopLeft => {
            // Make sure we don't overlap with the menu
            let text_y = match UI_STACK.last() {
                Option::Some(ui) => {
                    let size = Tas::get_text_size("x", SETTINGS.ui_scale);
                    let line_height = size.height;

                    let elements = ui.elements;
                    line_height * (elements.len().to_float() + 1.5)
                },
                Option::None => 0.0,
            };

            ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, text_y, anchor, 5.0);
        },
        Anchor::TopCenter    => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w/2.0, 5.0, anchor, 5.0); },
        Anchor::TopRight     => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, 5.0, anchor, 5.0); },
        Anchor::CenterRight  => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, h/2.0, anchor, 5.0); },
        Anchor::BottomRight  => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w, h, anchor, 5.0); },
        Anchor::BottomCenter => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, w/2.0, h, anchor, 5.0); },
        Anchor::BottomLeft   => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, h, anchor, 5.0); },
        Anchor::CenterLeft   => { ap_draw_colorful_text(lines, AP_COLOR_GRAY_BG, 5.0, h/2.0, anchor, 5.0); },

        pos => panic(f"unknown/invalid archipelago display position: {pos}"),
    };
}
