

fn create_block_brawl_options_menu() {
    let elements = List::new();

    elements.push(UiElement::Chooser(Chooser {
        label: Text { text: "Brawl or Blub" },
        options: List::of(Text { text: "Block Brawl" }, Text { text: "Block Blub" }),
        selected: if BLOCK_BRAWL_SETTINGS.brawl_or_blub == "Block Brawl" { 0 } else { 1 },
        onchange: fn(index: int) { BLOCK_BRAWL_SETTINGS.brawl_or_blub = if index == 0 { "Block Brawl" } else { "Block Blub" }; },
    }));

    elements.push(UiElement::Chooser(Chooser {
        label: Text { text: "Alt" },
        options: List::of(Text { text: "Normal" }, Text { text: "Crazy" }),
        selected: if BLOCK_BRAWL_SETTINGS.alt { 1 } else { 0 },
        onchange: fn(index: int) { BLOCK_BRAWL_SETTINGS.alt = index == 1; },
    }));

    elements.push(UiElement::Chooser(Chooser {
        label: Text { text: "Movement" },
        options: List::of(Text { text: "Wall jump" }, Text { text: "Dash" }, Text { text: "Toggle" }),
        selected: match BLOCK_BRAWL_SETTINGS.wall_jump_or_dash { "Dash" => 1, "Toggle" => 2, _ => 0 },
        onchange: fn(index: int) {
            BLOCK_BRAWL_SETTINGS.wall_jump_or_dash = match index { 1 => "Dash", 2 => "Toggle", _ => "Wall jump" };
        },
    }));

    elements.push(UiElement::FloatInput(FloatInput {
        label: Text { text: "Countdown (0 - 3600) seconds" }, input: f"{BLOCK_BRAWL_SETTINGS.count_down_time}",
        onclick: fn(input: string) {}, onchange: fn(input: string) {
            match input.parse_int() { Result::Ok(value) => if 0 <= value && value <= 3600 { BLOCK_BRAWL_SETTINGS.count_down_time = value; }, Result::Err(e) => {}, }
        },
    }));

    elements.push(UiElement::FloatInput(FloatInput {
        label: Text { text: "Goal score (0 - 1000000)" }, input: f"{BLOCK_BRAWL_SETTINGS.goal_score}",
        onclick: fn(input: string) {}, onchange: fn(input: string) {
            match input.parse_int() { Result::Ok(value) => if 0 <= value && value <= 1000000 { BLOCK_BRAWL_SETTINGS.goal_score = value; }, Result::Err(e) => {}, }
        },
    }));

    elements.push(UiElement::FloatInput(FloatInput {
        label: Text { text: "Colors (1 - 4)" }, input: f"{BLOCK_BRAWL_SETTINGS.number_of_colors}",
        onclick: fn(input: string) {}, onchange: fn(input: string) {
            match input.parse_int() { Result::Ok(value) => if 1 <= value && value <= 4 { BLOCK_BRAWL_SETTINGS.number_of_colors = value; }, Result::Err(e) => {}, }
        },
    }));

    elements.push(UiElement::FloatInput(FloatInput {
        label: Text { text: "Cubes per color (1 - 10)" }, input: f"{BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color}",
        onclick: fn(input: string) {}, onchange: fn(input: string) {
            match input.parse_int() { Result::Ok(value) => if 1 <= value && value <= 10 { BLOCK_BRAWL_SETTINGS.number_of_cubes_per_color = value; }, Result::Err(e) => {}, }
        },
    }));

    elements.push(UiElement::FloatInput(FloatInput {
        label: Text { text: "Platforms (100 - 400)" }, input: f"{BLOCK_BRAWL_SETTINGS.number_of_platforms}",
        onclick: fn(input: string) {}, onchange: fn(input: string) {
            match input.parse_int() { Result::Ok(value) => if 100 <= value && value <= 400 { BLOCK_BRAWL_SETTINGS.number_of_platforms = value; }, Result::Err(e) => {}, }
        },
    }));

    elements.push(UiElement::Chooser(Chooser {
        label: Text { text: "Scoring" },
        options: List::of(Text { text: "Sum + lowest" }, Text { text: "Sum" }, Text { text: "Lowest" }),
        selected: match BLOCK_BRAWL_SETTINGS.scoring_method { "Sum" => 1, "Lowest" => 2, _ => 0 },
        onchange: fn(index: int) {
            BLOCK_BRAWL_SETTINGS.scoring_method = match index { 1 => "Sum", 2 => "Lowest", _ => "Sum + lowest" };
        },
    }));
    
    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui();leave_ui();leave_ui(); enter_ui(create_block_brawl_menu()); },
    }));

    enter_ui(Ui::new("Block Brawl & Blub Options", elements));
}

fn create_block_brawl_menu() -> Ui {
    let elements = List::new();

    elements.push(UiElement::Chooser(Chooser {
        label: Text { text: "Preset" },
        options: List::of(Text { text: "Default Brawl" }, Text { text: "Default Blub" }, Text { text: "Custom" }),
        selected: match BLOCK_BRAWL_SETTINGS.preset {
            "Default Brawl" => 0,
            "Default Blub" => 1,
            _ => 2,
        },
        onchange: fn(index: int) {
            if index == 0 { BLOCK_BRAWL_SETTINGS = default_brawl_settings(); }
            if index == 1 { BLOCK_BRAWL_SETTINGS = default_blub_settings(); }
            BLOCK_BRAWL_SETTINGS.preset = match index {
                0 => "Default Brawl",
                1 => "Default Blub",
                _ => "Custom",
            };
        },
    }));

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Choose custom options" },
        onclick: fn(label: Text) {
            create_block_brawl_options_menu();
            BLOCK_BRAWL_SETTINGS.preset = "Custom";
        },
    }));

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Start!" },
        onclick: fn(label: Text) {
            block_brawl_start_select();
            leave_ui();leave_ui();leave_ui();leave_ui();
        },
    }));

    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui(); },
    }));

    Ui::new("Offline Block Brawl", elements)
}

fn block_brawl_start_select() {
    add_component(BLOCK_BRAWL_COMPONENT);
}

fn block_brawl_hud_text(text: string) -> string {
    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::Classic { return text; }

    let mut hud_text = "";
    for line in block_brawl_get_status_text_lines() {
        let text = line.text;
        hud_text = f"{hud_text}{text}";
    }
    hud_text
}

fn block_brawl_hub_color_coded() {
    let viewport = Tas::get_viewport_size();
    let w = viewport.width.to_float();
    let h = viewport.height.to_float();

    if SETTINGS.archipelago_display_style != ArchipelagoDisplayStyle::ColorCoded { return; }

    let lines = block_brawl_get_status_text_lines();
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

    if BLOCK_BRAWL_STATE.started == 0 {
        let anchor2 = Anchor::BottomishCenter;
        let lines2 = List::new();
        lines2.push(ColorfulText { text: "Press New Game from the Refunct menu to start.\n", color: COLOR_WHITE });
        lines2.push(ColorfulText { text: "You can change gamemode in the 'm' menu.", color: COLOR_WHITE });
        ap_draw_colorful_text(lines2, AP_COLOR_GRAY_BG, w/2.0, 4.0*h/5.0, anchor2, 5.0);
    }
    
}

fn block_brawl_get_status_text_lines() -> List<ColorfulText> {  
    let lines = List::new();

    if INPUT_MODE_IS_UI_ONLY {
        List::extend(lines, List::of(
            ColorfulText { text: "Movement input disabled\n", color: AP_COLOR_RED },
            ColorfulText { text: "Press F1 to enable movement input", color: AP_COLOR_RED },
            ColorfulText { text: "\n\n", color: AP_COLOR_RED },
        ));
    }

    let min1 = List::min(List::of(
        BLOCK_BRAWL_STATE.score_block_brawl_reds,
        BLOCK_BRAWL_STATE.score_block_brawl_blues,
        BLOCK_BRAWL_STATE.score_block_brawl_greens,
        BLOCK_BRAWL_STATE.score_block_brawl_yellows
    ));
    let mut total_score = 0;

    if BLOCK_BRAWL_SETTINGS.scoring_method == "Sum" {
        total_score = BLOCK_BRAWL_STATE.score_block_brawl_reds
        + BLOCK_BRAWL_STATE.score_block_brawl_blues
        + BLOCK_BRAWL_STATE.score_block_brawl_greens
        + BLOCK_BRAWL_STATE.score_block_brawl_yellows;
    }
    if BLOCK_BRAWL_SETTINGS.scoring_method == "Lowest" {
        total_score = min1.unwrap_or(0);
    }
    if BLOCK_BRAWL_SETTINGS.scoring_method == "Sum + lowest" {
        total_score = BLOCK_BRAWL_STATE.score_block_brawl_reds
        + BLOCK_BRAWL_STATE.score_block_brawl_blues
        + BLOCK_BRAWL_STATE.score_block_brawl_greens
        + BLOCK_BRAWL_STATE.score_block_brawl_yellows
        + min1.unwrap_or(0);
    }

    if BLOCK_BRAWL_STATE.finish_result == "" {
        if BLOCK_BRAWL_SETTINGS.count_down_time > 0 && BLOCK_BRAWL_STATE.block_brawl_time >= BLOCK_BRAWL_SETTINGS.count_down_time * 1000 {
            BLOCK_BRAWL_STATE.finish_result = f"Final score after {BLOCK_BRAWL_SETTINGS.count_down_time} seconds: {total_score}";
        } 
        if BLOCK_BRAWL_SETTINGS.goal_score > 0 && total_score >= BLOCK_BRAWL_SETTINGS.goal_score {
            BLOCK_BRAWL_STATE.finish_result = f"Final time for getting {BLOCK_BRAWL_SETTINGS.goal_score} points: {int::to_float(BLOCK_BRAWL_STATE.block_brawl_time)/1000.0:.3f}s";
        }
    }

    List::extend(lines, match BLOCK_BRAWL_STATE.started {
        0 => List::of(
            ColorfulText { text: "Block Brawl!\n", color: COLOR_WHITE },
            ColorfulText { text: "Press new game (in Refunct menu).\n", color: AP_COLOR_CYAN },
        ),
        _ => List::of(
            ColorfulText { text: "Archipelago - Block Brawl\n", color: COLOR_WHITE },
            ColorfulText { text: f"Preset: {BLOCK_BRAWL_SETTINGS.preset}\n", color: COLOR_WHITE },
            ColorfulText { text: f"Time: {int::to_float(BLOCK_BRAWL_STATE.block_brawl_time)/1000.0:.1f}s{if BLOCK_BRAWL_SETTINGS.count_down_time > 0 { f" / {BLOCK_BRAWL_SETTINGS.count_down_time}" } else { "" }}\n", color: COLOR_WHITE },
            ColorfulText { text: f"{if BLOCK_BRAWL_SETTINGS.wall_jump_or_dash == "Toggle" { "[T to toggle] " } else { " " }}", color: COLOR_WHITE },
            ColorfulText { text: "Wall Jump  ", color: if BLOCK_BRAWL_STATE.dash { AP_COLOR_RED } else { COLOR_GREEN } },
            ColorfulText { text: "Dash\n", color: if BLOCK_BRAWL_STATE.dash { COLOR_GREEN } else { AP_COLOR_RED } },

            ColorfulText { text: "Goal: cubes => points => checks!", color: AP_COLOR_GREEN },

            ColorfulText { text: f"\n\nCurrent scores: ", color: COLOR_WHITE },
            ColorfulText { text: f"{BLOCK_BRAWL_STATE.score_block_brawl_reds:4} ", color: AP_COLOR_RED },
            ColorfulText { text: f"{BLOCK_BRAWL_STATE.score_block_brawl_blues:4} ", color: AP_COLOR_CYAN },
            ColorfulText { text: f"{BLOCK_BRAWL_STATE.score_block_brawl_greens:4} ", color: AP_COLOR_GREEN },
            ColorfulText { text: f"{BLOCK_BRAWL_STATE.score_block_brawl_yellows:4} ", color: AP_COLOR_YELLOW },

            ColorfulText { text: f"\nScore ({BLOCK_BRAWL_SETTINGS.scoring_method}): {total_score}{if BLOCK_BRAWL_SETTINGS.goal_score > 0 { f" / {BLOCK_BRAWL_SETTINGS.goal_score}" } else { "" }}", color: COLOR_WHITE },

            ColorfulText { text: f"\nCubes collected: {BLOCK_BRAWL_STATE.block_brawl_cubes_collected} / {BLOCK_BRAWL_STATE.block_brawl_cubes_total}", color: COLOR_WHITE },

            ColorfulText { text: f"\nCombo: next cube is worth {BLOCK_BRAWL_STATE.score_for_next_block_brawl} pts", color: COLOR_WHITE },

            ColorfulText { text: f"{if BLOCK_BRAWL_STATE.finish_result != "" { f"\n\n{BLOCK_BRAWL_STATE.finish_result}" } else { "" }}", color: COLOR_WHITE },
        ),
    });
    lines
}