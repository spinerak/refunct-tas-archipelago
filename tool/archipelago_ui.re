enum ArchipelagoDisplayStyle {
    Classic,
    ColorCoded
}

struct ColorfulText {
    text: string,
    color: Color
}

static AP_COLOR_RED    = Color { red: 0.600, green: 0.160, blue: 0.227, alpha: 1. };
static AP_COLOR_CYAN   = Color { red: 0.000, green: 0.627, blue: 0.698, alpha: 1. };
static AP_COLOR_GREEN  = Color { red: 0.231, green: 0.600, blue: 0.165, alpha: 1. };
static AP_COLOR_YELLOW = Color { red: 0.600, green: 0.533, blue: 0.165, alpha: 1. };
static AP_COLOR_BG     = Color { red: 0., green: 0., blue: 0., alpha: 0.6 };

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
                ColorfulText { text: "Goal: Press the buttons!", color: AP_COLOR_CYAN },
            ),
            // 2 => List::of(
            //     ColorfulText { text: "Archipelago - Original Rando", color: COLOR_WHITE },
            //     ColorfulText { text: "Touch a platform to start!", color: AP_COLOR_CYAN },
            // ),
            3 => List::of(
                ColorfulText { text: "Archipelago - Seeker", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Find the empty platforms!", color: AP_COLOR_CYAN },
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

fn archipelago_hud_text(text: string) -> string {
    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::Classic { return text; }

    let mut hud_text = "";
    for line in get_status_text_lines() {
        let text = line.text;
        hud_text = f"{hud_text}{text}\n";
    }
    hud_text
}

fn archipelago_hud_color_coded() {
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
}