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
static AP_COLOR_GRAY_BG = Color { red: 0., green: 0., blue: 0., alpha: 0.6 };
static AP_COLOR_CLEAR   = Color { red: 0., green: 0., blue: 0., alpha: 0.0 };

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
                Anchor::TopLeft => 0,
                Anchor::TopCenter => 1,
                Anchor::TopRight => 2,
                Anchor::CenterRight => 3,
                Anchor::BottomRight => 4,
                Anchor::BottomCenter => 5,
                Anchor::BottomLeft => 6,
                Anchor::CenterLeft => 7,
                pos => panic(f"unknown archipelago display position: {pos}"),
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
            ),
            // 2 => List::of(
            //     ColorfulText { text: "Archipelago - Original Rando\n", color: COLOR_WHITE },
            //     ColorfulText { text: "Touch a platform to start!", color: AP_COLOR_CYAN },
            // ),
            3 => List::of(
                ColorfulText { text: "Archipelago - Seeker\n", color: COLOR_WHITE },
                ColorfulText { text: "Goal: Find the empty platforms!", color: AP_COLOR_CYAN },
            ),
            _ => List::of(
                ColorfulText { text: "Archipelago\n", color: COLOR_WHITE },
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

    let vanilla_state = if ARCHIPELAGO_STATE.unlock_vanilla_minigame { "YES" } else { "NO" };
    let seeker_state = if ARCHIPELAGO_STATE.unlock_seeker_minigame { "YES" } else { "NO" };

    List::of(
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
            text:  f"Ledge Grab: { ledge_grab_state }\n",
            color: if ARCHIPELAGO_STATE.ledge_grab > 0 { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Wall Jump:  { wall_jump_state }\n",
            color: if ARCHIPELAGO_STATE.wall_jump >= 2 { AP_COLOR_GREEN } else if ARCHIPELAGO_STATE.wall_jump == 1 { AP_COLOR_YELLOW } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Jump Pads:  { jump_pad_state }\n",
            color: if ARCHIPELAGO_STATE.jumppads > 0 { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Swim:       { swim_state }\n\n",
            color: if ARCHIPELAGO_STATE.swim > 0 { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText { text: "Minigames Unlocked\n", color: COLOR_WHITE },
        ColorfulText {
            text:  f"Vanilla: {vanilla_state}\n",
            color: if ARCHIPELAGO_STATE.unlock_vanilla_minigame { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
        ColorfulText {
            text:  f"Seeker:  {seeker_state}",
            color: if ARCHIPELAGO_STATE.unlock_seeker_minigame { AP_COLOR_GREEN } else { AP_COLOR_RED }
        },
    )
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
    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::Off {

        // For now, always draw the platform display
        let player_loc = Tas::get_location();
        let player_vel = Tas::get_velocity();
        let platform_text = match ARCHIPELAGO_STATE.last_platform_c {
            Option::Some(cluster) => {
                let platform = ARCHIPELAGO_STATE.last_platform_p.unwrap();
                f"Last Platform: {cluster}-{platform}"
            },
            Option::None => "Last Platform: ??-??"
        };

        ap_draw_colorful_text(
            List::of(ColorfulText { text: platform_text, color: COLOR_WHITE }), AP_COLOR_GRAY_BG,
            viewport.width.to_float() / 2.0, viewport.height.to_float(), Anchor::BottomCenter, 5.0);
    }

    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::ColorCoded { return; }

    let lines = get_status_text_lines();
    let anchor = SETTINGS.archipelago_display_position;

    match SETTINGS.archipelago_display_position {
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