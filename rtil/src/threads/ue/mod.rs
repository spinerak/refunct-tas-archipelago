use std::sync::atomic::Ordering;
use crossbeam_channel::{Receiver, Sender};
use tokio::sync::mpsc::UnboundedSender;
use crate::native::{ALiftBaseUE, ElementIndex, ElementType, EMouseButtonsType, Hooks, try_find_element_index, UObject, AActor, ActorWrapper};
use crate::native::character::CURRENT_PLAYER;
use crate::threads::{ArchipelagoToRebo, ReboToArchipelago, ReboToStream, StreamToRebo};
use crate::threads::ue::iced_ui::Key;

mod rebo;
mod iced_ui;

#[derive(Debug, Clone)]
enum UeEvent {
    Tick,
    ElementPressed(ElementIndex),
    ElementReleased(ElementIndex),
    /// Response to `Yield` if no new event happened
    NothingHappened,
    NewGame,
    KeyDown(Key, bool),
    KeyUp(Key, bool),
    KeyChar(char, bool),
    MouseMove(i32, i32),
    MouseButtonDown(EMouseButtonsType),
    MouseButtonUp(EMouseButtonsType),
    MouseWheel(f32),
    DrawHud,
    ApplyResolutionSettings,
    AddToScreen,
}
#[derive(Debug, Clone, Copy)]
enum Suspend {
    /// yield only without returning to the UE loop for the event-queue to be handled
    Yield,
    /// return back to the UE loop
    Return,
}

pub fn run(
    stream_rebo_rx: Receiver<StreamToRebo>, rebo_stream_tx: Sender<ReboToStream>, 
    hooks: Hooks,
    archipelago_rebo_rx: Receiver<ArchipelagoToRebo>, rebo_archipelago_tx: UnboundedSender<ReboToArchipelago>,
) {
    rebo::init(stream_rebo_rx, rebo_stream_tx, hooks, archipelago_rebo_rx, rebo_archipelago_tx);
    log!("\"starting\" ue thread");
}

pub fn new_game() {
    log!("New Game");
    handle(UeEvent::NewGame)
}

pub fn tick() {
    handle(UeEvent::Tick);
}

pub fn add_based_character(ptr: *mut ALiftBaseUE) {
    // TODO: remove once we added pipes to the map editor
    let element_index = match try_find_element_index(ptr as *mut UObject) {
        Some(i) => i,
        None => return,
    };
    handle(UeEvent::ElementPressed(element_index));
}
pub fn remove_based_character(ptr: *mut ALiftBaseUE) {
    // TODO: remove once we added pipes to the map editor
    let element_index = match try_find_element_index(ptr as *mut UObject) {
        Some(i) => i,
        None => return,
    };
    handle(UeEvent::ElementReleased(element_index));
}

fn codes_to_key(key_code: i32, character_code: u32) -> Key {
    #[cfg(unix)] {
        Key::try_from_linux(key_code, character_code)
    }
    #[cfg(windows)] {
        Key::try_from_windows(key_code, character_code)
    }
}

pub fn key_down(key_code: i32, character_code: u32, is_repeat: bool) {
    handle(UeEvent::KeyDown(codes_to_key(key_code, character_code), is_repeat));
}
pub fn key_up(key_code: i32, character_code: u32, is_repeat: bool) {
    handle(UeEvent::KeyUp(codes_to_key(key_code, character_code), is_repeat));
}

pub fn key_char(character: char, is_repeat: bool) {
    handle(UeEvent::KeyChar(character, is_repeat));
}

pub fn mouse_move(x: i32, y: i32) {
    handle(UeEvent::MouseMove(x, y));
}
pub fn mouse_button_down(button: EMouseButtonsType) {
    handle(UeEvent::MouseButtonDown(button));
}
pub fn mouse_button_up(button: EMouseButtonsType) {
    handle(UeEvent::MouseButtonUp(button));
}
pub fn mouse_wheel(delta: f32) {
    handle(UeEvent::MouseWheel(delta));
}

pub fn aactor_receive_begin_overlap(this: *mut AActor, other: *mut AActor) {
    if this.addr() == CURRENT_PLAYER.load(Ordering::SeqCst).addr() {
        unsafe {
            let other_actor = ActorWrapper::new(other);
            if other_actor.name().starts_with("BP_PowerCore") {
                // Cluster index 9999 is a hacky way to identify that this is a cube we spawned
                let element_index = match try_find_element_index(other as *mut UObject) {
                    Some(i) => i,
                    None => ElementIndex { cluster_index: 9999, element_type: ElementType::Cube, element_index: other_actor.internal_index() as usize },
                };
                handle(UeEvent::ElementPressed(element_index));
            }
        }
    }
}

pub fn draw_hud() {
    handle(UeEvent::DrawHud);
}

pub fn apply_resolution_settings() {
    handle(UeEvent::ApplyResolutionSettings);
}

pub fn add_to_screen() {
    handle(UeEvent::AddToScreen);
}

fn handle(event: UeEvent) {
    rebo::poll(event);
}
