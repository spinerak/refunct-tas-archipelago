struct GamemodeState {
    started: bool,
}

fn fresh_gamemode_state() -> GamemodeState {
    GamemodeState {
        started: false,
    }
}

static mut GAMEMODE_STATE = fresh_gamemode_state();

fn create_minigames_menu() -> Ui {
    let elements = List::new();
    if BLOCK_BRAWL_STATE.block_brawl_time > 0 {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "THIS WILL RESET CURRENT MINIGAME" },
            onclick: fn(label: Text) {},
        }));
    }
    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Block Brawl & Blub" },
        onclick: fn(label: Text) {
            BLOCK_BRAWL_STATE = fresh_block_brawl_state();
            GAMEMODE_STATE.started = true;
            enter_ui(create_block_brawl_menu());
        },
    }));
    if GAMEMODE_STATE.started {
        elements.push(UiElement::Button(UiButton {
            label: Text { text: "Exit Offline Minigames" },
            onclick: fn(label: Text) {
                BLOCK_BRAWL_STATE = fresh_block_brawl_state();
                GAMEMODE_STATE.started = false;
                remove_component(BLOCK_BRAWL_COMPONENT);
                leave_ui();
            },
        }));
    }
    elements.push(UiElement::Button(UiButton {
        label: Text { text: "Back" },
        onclick: fn(label: Text) { leave_ui(); },
    }));

    Ui::new("Offline Minigames", elements)
}