use std::cell::Cell;
use std::collections::HashMap;
use std::fs::File;
use std::io::{ErrorKind, Write};
use std::ops::Deref;
use std::path::PathBuf;
use std::time::Duration;
use std::sync::Arc;
use archipelago_rs::protocol::{ClientMessage, ServerMessage, BounceData, DeathLink, GetDataPackage, RichPrint, RichMessageColor, RichMessagePart, NetworkItem, ItemsHandlingFlags};
use crossbeam_channel::{Sender, TryRecvError};
use image::Rgba;
use rebo::{DisplayValue, ExecError, IncludeConfig, Map, Output, ReboConfig, Span, Stdlib, Value, VmContext};
use itertools::Itertools;
use once_cell::sync::Lazy;
use websocket::{ClientBuilder, Message, OwnedMessage, WebSocketError};
use crate::native::{character::USceneComponent, uworld::JUMP6_INDEX, CubeWrapper, PlatformWrapper};
use crate::native::{try_find_element_index, ue::FVector, AActor, ALiftBaseUE, AMyCharacter, AMyHud, ActorWrapper, EBlendMode, FApp, FViewport, KismetSystemLibrary, Level, LevelState, LevelWrapper, ObjectIndex, ObjectWrapper, UGameplayStatics, UMyGameInstance, UObject, UTexture2D, UWorld, UeObjectWrapperType, UeScope, LEVELS};
use protocol::{Request, Response};
use crate::threads::{ArchipelagoToRebo, ReboToArchipelago, ReboToStream, StreamToRebo};
use super::{STATE, livesplit::{Game, NewGameGlitch, SplitsSaveError, SplitsLoadError}};
use serde::{Serialize, Deserialize};
use crate::threads::ue::{Suspend, UeEvent, rebo::YIELDER};
use crate::native::{ElementIndex, ElementType, ue::{FRotator, FLinearColor}, UEngine, TimeOfDay, UWidgetBlueprintLibrary};
use opener;
use chrono::{DateTime, Local};
use crate::threads::ue::rebo::livesplit;
use crate::threads::ue::iced_ui::Clipboard;
use crate::threads::ue::iced_ui::rebo_elements::{IcedButton, IcedColumn, IcedElement, IcedRow, IcedText, IcedWindow};

use crate::native::{BoolValueWrapper};
use crate::native::reflection::DerefToObjectWrapper;
use crate::native::font::replace_unrenderable_chars;

pub fn create_config(rebo_stream_tx: Sender<ReboToStream>) -> ReboConfig {
    let mut cfg = ReboConfig::new()
        .stdlib(Stdlib::all() - Stdlib::PRINT)
        .interrupt_interval(100)
        .interrupt_function(interrupt_function)
        .diagnostic_output(Output::buffered(move |s| {
            log!("{}", s);
            eprintln!("{}", s);
            rebo_stream_tx.send(ReboToStream::Print(s)).unwrap()
        }))
        .add_function(new_version_string)
        .add_function(print)
        .add_function(step)
        .add_function(step_yield)
        .add_function(load_settings)
        .add_function(store_settings)
        .add_function(list_recordings)
        .add_function(save_recording)
        .add_function(load_recording)
        .add_function(remove_recording)
        .add_function(key_down)
        .add_function(key_up)
        .add_function(move_mouse)
        .add_function(get_last_frame_delta)
        .add_function(get_delta)
        .add_function(set_delta)
        .add_function(get_location)
        .add_function(get_location_and_log)
        .add_function(set_location)
        .add_function(get_rotation)
        .add_function(set_rotation)
        .add_function(get_player_name)
        .add_function(get_steamid)
        .add_function(get_velocity)
        .add_function(set_velocity)
        .add_function(get_acceleration)
        .add_function(set_acceleration)
        .add_function(get_movement_mode)
        .add_function(set_movement_mode)
        .add_function(get_max_fly_speed)
        .add_function(set_max_fly_speed)
        .add_function(get_max_walk_speed)
        .add_function(get_base_speed)
        .add_function(get_max_bonus_speed)
        .add_function(get_level_state)
        .add_function(restart_game)
        .add_function(wait_for_new_game)
        .add_function(draw_line)
        .add_function(draw_text)
        .add_function(draw_rect)
        .add_function(draw_minimap)
        .add_function(set_minimap_alpha)
        .add_function(draw_player_minimap)
        .add_function(player_minimap_size)
        .add_function(minimap_size)
        .add_function(project)
        .add_function(get_viewport_size)
        .add_function(get_text_size)

        .add_function(spawn_platform_rebo)
        .add_function(spawn_platform_rando_location)
        .add_function(spawn_cube_rando_location)
        .add_function(destroy_platforms)
        .add_function(destroy_platform_rebo)

        .add_function(spawn_cube_rebo)
        .add_function(reset_cubes)
        .add_function(destroy_cubes)
        .add_function(collect_cube)
        .add_function(get_vanilla_cube)
        .add_function(set_cube_collision)
        .add_function(set_cube_color_rebo)
        .add_function(set_cube_color_random)
        .add_function(set_cube_location)
        .add_function(set_cube_scale)
        .add_function(get_vanilla_cubes)
        .add_function(get_non_vanilla_cubes)
        .add_function(get_non_vanilla_platforms_rebo)
        .add_function(get_all_cubes)

        .add_function(spawn_pawn)
        .add_function(destroy_pawn)
        .add_function(move_pawn)
        .add_function(set_pawn_velocity)
        .add_function(pawn_location)
        .add_function(connect_to_server)
        .add_function(disconnect_from_server)
        .add_function(join_multiplayer_room)
        .add_function(move_on_server)
        .add_function(press_platform_on_server)
        .add_function(press_button_on_server)

        .add_function(archipelago_connect)
        .add_function(archipelago_disconnect)
        .add_function(archipelago_send_check)
        .add_function(archipelago_goal)
        .add_function(new_game_pressed)
        .add_function(get_level)
        .add_function(set_level_rebo)
        .add_function(trigger_element)
        .add_function(trigger_element_by_type_rebo)
        .add_function(archipelago_activate_buttons_ap)
        .add_function(archipelago_deactivate_buttons_ap)
        .add_function(archipelago_gather_all_buttons)
        .add_function(archipelago_trigger_goal_animation)
        .add_function(archipelago_raise_cluster_rebo)

        .add_function(abilities_set_wall_jump)
        .add_function(abilities_set_ledge_grab)
        .add_function(abilities_set_jump_pads)
        .add_function(abilities_set_pipes)
        .add_function(abilities_set_lifts)
        .add_function(abilities_set_swim)
        .add_function(set_start_seconds)
        .add_function(set_start_partial_seconds)
        .add_function(set_end_seconds)
        .add_function(set_end_partial_seconds)
        .add_function(get_accurate_real_time)
        .add_function(is_windows)
        .add_function(is_linux)
        .add_function(get_clipboard)
        .add_function(set_clipboard)
        .add_function(show_hud)
        .add_function(set_all_cluster_speeds)
        .add_function(list_maps)
        .add_function(load_map)
        .add_function(save_map)
        .add_function(remove_map)
        .add_function(current_map)
        .add_function(original_map)
        .add_function(apply_map)
        .add_function(apply_map_cluster_speeds)
        .add_function(get_looked_at_element_index)
        .add_function(get_element_bounds)
        .add_function(enable_player_collision)
        .add_function(disable_player_collision)
        .add_function(exit_water)
        .add_function(respawn)        
        .add_function(is_death_link_on)
        .add_function(set_death_link)
        .add_function(open_maps_folder)
        .add_function(open_recordings_folder)
        .add_function(set_lighting_casts_shadows)
        .add_function(set_sky_light_enabled)
        .add_function(set_time_dilation)
        .add_function(set_gravity)
        .add_function(get_time_of_day)
        .add_function(set_time_of_day)
        .add_function(set_sky_time_speed)
        .add_function(set_sky_light_brightness)
        .add_function(set_sky_light_intensity)
        .add_function(set_stars_brightness)
        .add_function(set_fog_enabled)
        .add_function(set_sun_redness)
        .add_function(set_cloud_redness)
        .add_function(set_reflection_render_scale)
        .add_function(set_cloud_speed)
        .add_function(set_outro_time_dilation)
        .add_function(set_outro_dilated_duration)
        .add_function(set_kill_z)
        .add_function(set_gamma)
        .add_function(set_screen_percentage)
        .add_function(set_reticle_width)
        .add_function(set_reticle_height)
        .add_function(get_camera_mode)
        .add_function(set_camera_mode)
        .add_function(livesplit::livesplit_start)
        .add_function(livesplit::livesplit_split)
        .add_function(livesplit::livesplit_reset)
        .add_function(livesplit::livesplit_get_game_time)
        .add_function(livesplit::livesplit_set_game_time)
        .add_function(livesplit::livesplit_pause_game_time)
        .add_function(livesplit::livesplit_create_segment)
        .add_function(livesplit::livesplit_get_game_name)
        .add_function(livesplit::livesplit_set_game_info)
        .add_function(livesplit::livesplit_get_category_name)
        .add_function(livesplit::livesplit_get_segments)
        .add_function(livesplit::livesplit_get_attempt_count)
        .add_function(livesplit::livesplit_save_splits)
        .add_function(livesplit::livesplit_load_splits)
        .add_function(livesplit::livesplit_get_color)
        .add_function(livesplit::livesplit_set_color)
        .add_function(livesplit::livesplit_get_total_playtime)
        .add_function(livesplit::livesplit_get_total_playtime_digits_format)
        .add_function(livesplit::livesplit_set_total_playtime_digits_format)
        .add_function(livesplit::livesplit_get_total_playtime_accuracy)
        .add_function(livesplit::livesplit_set_total_playtime_accuracy)
        .add_function(livesplit::livesplit_get_sum_of_best_segments)
        .add_function(livesplit::livesplit_get_sum_of_best_digits_format)
        .add_function(livesplit::livesplit_set_sum_of_best_digits_format)
        .add_function(livesplit::livesplit_get_sum_of_best_accuracy)
        .add_function(livesplit::livesplit_set_sum_of_best_accuracy)
        .add_function(livesplit::livesplit_get_current_pace)
        .add_function(livesplit::livesplit_get_current_pace_digits_format)
        .add_function(livesplit::livesplit_set_current_pace_digits_format)
        .add_function(livesplit::livesplit_get_current_pace_accuracy)
        .add_function(livesplit::livesplit_set_current_pace_accuracy)
        .add_function(livesplit::get_pb_chance)
        .add_function(set_game_rendering_enabled)
        .add_function(set_input_mode_game_only)
        .add_function(set_input_mode_ui_only)
        .add_function(flush_pressed_keys)
        .add_external_type(Location)
        .add_external_type(Rotation)
        .add_external_type(Velocity)
        .add_external_type(Acceleration)
        .add_external_type(Vector)
        .add_external_type(TextSize)
        .add_external_type(Line)
        .add_external_type(Color)
        .add_external_type(DrawText)
        .add_external_type(Size)
        .add_external_type(LevelState)
        .add_external_type(Server)
        .add_external_type(Step)
        .add_external_type(Disconnected)
        .add_external_type(RecordFrame)
        .add_external_type(InputEvent)
        .add_external_type(RefunctMap)
        .add_external_type(Cluster)
        .add_external_type(Element)
        .add_external_type(ElementType)
        .add_external_type(ElementIndex)
        .add_external_type(Bounds)
        .add_external_type(TimeOfDay)
        .add_external_type(Game)
        .add_external_type(Segment)
        .add_external_type(NewGameGlitch)
        .add_external_type(SplitsSaveError)
        .add_external_type(SplitsLoadError)
        .add_external_type(livesplit::DigitsFormat)
        .add_external_type(livesplit::Accuracy)
        .add_external_type(livesplit::Comparison)
        .add_external_type(livesplit::LiveSplitLayoutColor)
        .add_external_type(IcedWindow)
        .add_external_type(IcedElement)
        .add_external_type(IcedButton)
        .add_external_type(IcedText)
        .add_external_type(IcedRow)
        .add_external_type(IcedColumn)
        .add_external_type(ReboPrintJSONMessage)
        .add_external_type(ReboJSONMessagePart)
        .add_external_type(ReboNetworkItem)
        .add_required_rebo_function(element_pressed)
        .add_required_rebo_function(element_released)
        .add_required_rebo_function(on_key_down)
        .add_required_rebo_function(on_key_up)
        .add_required_rebo_function(on_key_char)
        .add_required_rebo_function(on_mouse_move)
        .add_required_rebo_function(draw_hud)
        .add_required_rebo_function(player_joined_multiplayer_room)
        .add_required_rebo_function(player_left_multiplayer_room)
        .add_required_rebo_function(player_moved)
        .add_required_rebo_function(press_platform)
        .add_required_rebo_function(press_button)
        .add_required_rebo_function(player_pressed_new_game)
        .add_required_rebo_function(start_new_game_at)
        .add_required_rebo_function(disconnected)
        .add_required_rebo_function(archipelago_disconnected)
        .add_required_rebo_function(archipelago_received_item)
        .add_required_rebo_function(archipelago_got_grass)
        .add_required_rebo_function(archipelago_checked_location)
        .add_required_rebo_function(archipelago_received_slot_data)
        .add_required_rebo_function(archipelago_register_slot)
        .add_required_rebo_function(archipelago_register_player)
        .add_required_rebo_function(archipelago_register_game_item)
        .add_required_rebo_function(archipelago_register_game_location)
        .add_required_rebo_function(archipelago_print_json_message)
        .add_required_rebo_function(archipelago_received_death)
        .add_required_rebo_function(archipelago_trigger_one_cluster_now)
        .add_required_rebo_function(archipelago_init)
        .add_required_rebo_function(archipelago_set_own_id)
        .add_required_rebo_function(ap_log_error)
        .add_required_rebo_function(on_level_state_change)
        .add_required_rebo_function(on_resolution_change)
        .add_required_rebo_function(on_menu_open)
    ;
    if let Some(working_dir) = &STATE.lock().unwrap().as_ref().unwrap().working_dir {
        cfg = cfg.include_config(IncludeConfig::InDirectory(PathBuf::from(working_dir)));
    }
    cfg
}

#[derive(rebo::ExternalType)]
pub enum Step {
    Tick,
    NewGame,
    Yield,
}

#[derive(rebo::ExternalType)]
pub enum Disconnected {
    Closed,
    ManualDisconnect,
    SendFailed,
    ReceiveFailed,
    ConnectionRefused,
    LocalTimeOffsetTooManyTries,
    RoomNameTooLong,
}

#[derive(rebo::ExternalType)]
pub struct Segment {
    pub name: String,
    pub time: f64,
    pub pb_time: f64,
    pub best_time: f64,
}

#[derive(rebo::ExternalType, Debug, Serialize, Deserialize)]
pub struct ReboPrintJSONMessage {
    pub _type: String, // the JSONMessage type field is optional, but we'll enforce it
    pub data: Vec<ReboJSONMessagePart>,

    // For certain message types, we'll also include this information, so that we can filter correctly
    pub receiving: Option<isize>,
    pub item: Option<ReboNetworkItem>,
}

#[derive(rebo::ExternalType, Debug, Serialize, Deserialize)]
pub struct ReboJSONMessagePart {
    pub _type: String, // the JSONMessagePart type field is optional, but we'll enforce it
    pub text: Option<String>,
    pub color: Option<String>,
    pub flags: Option<usize>,
    pub player: Option<isize>,
}

#[derive(rebo::ExternalType, Debug, Serialize, Deserialize)]
pub struct ReboNetworkItem {
    pub item: isize,
    pub location: isize,
    pub player: isize,
    pub flags: isize,
}

impl ReboPrintJSONMessage {
    pub fn from(print_json: &RichPrint) -> ReboPrintJSONMessage {
        let _type: String;
        let _data: &Vec<RichMessagePart>;

        let (_type, _data) = match print_json {
            RichPrint::ItemSend { data, .. } => (String::from("ItemSend"), data),
            RichPrint::ItemCheat { data, .. } => (String::from("ItemCheat"), data),
            RichPrint::Hint { data, .. } => (String::from("Hint"), data),
            RichPrint::Join { data, .. } => (String::from("Join"), data),
            RichPrint::Part { data, .. } => (String::from("Part"), data),
            RichPrint::Chat { data, .. } => (String::from("Chat"), data),
            RichPrint::ServerChat { data, .. } => (String::from("ServerChat"), data),
            RichPrint::Tutorial { data, .. } => (String::from("Tutorial"), data),
            RichPrint::TagsChanged { data, .. } => (String::from("TagsChanged"), data),
            RichPrint::CommandResult { data, .. } => (String::from("CommandResult"), data),
            RichPrint::AdminCommandResult { data, .. } => (String::from("AdminCommandResult"), data),
            RichPrint::Goal { data, .. } => (String::from("Goal"), data),
            RichPrint::Release { data, .. } => (String::from("Release"), data),
            RichPrint::Collect { data, .. } => (String::from("Collect"), data),
            RichPrint::Countdown { data, .. } => (String::from("Countdown"), data),
            RichPrint::Unknown { data, .. } => (String::from("Text"), data),
        };

        let (_receiving, _item) = match print_json {
            RichPrint::ItemSend { receiving, item, .. }
            | RichPrint::ItemCheat { receiving, item, .. }
            | RichPrint::Hint { receiving, item, .. } => {
                    (Some(*receiving as isize), Some(ReboNetworkItem::from(item)))
            },
            _ => (None, None),
        };

        ReboPrintJSONMessage {
            _type: _type,
            data: _data.iter().map(ReboJSONMessagePart::from).collect(),
            receiving: _receiving,
            item: _item,
        }
    }
}

impl ReboJSONMessagePart {
    pub fn from(json_message_part: &RichMessagePart) -> ReboJSONMessagePart {
        match json_message_part {
            RichMessagePart::PlayerId { id, .. } => ReboJSONMessagePart {
                _type: String::from("player_id"),
                text: Some(id.to_string()),
                color: None, flags: None, player: None,
            },
            RichMessagePart::PlayerName { text, .. } => ReboJSONMessagePart {
                _type: String::from("player_name"),
                text: Some(String::from(text)),
                color: None, flags: None, player: None,
            },
            RichMessagePart::ItemId { id, flags, player, .. } => ReboJSONMessagePart {
                _type: String::from("item_id"),
                text: Some(id.to_string()),
                flags: Some(Into::<u8>::into(flags.clone()) as usize),
                player: Some(*player as isize),
                color: None,
            },
            RichMessagePart::ItemName { text, flags, player, .. } => ReboJSONMessagePart {
                _type: String::from("item_name"),
                text: Some(String::from(text)),
                flags: Some(Into::<u8>::into(flags.clone()) as usize),
                player: Some(*player as isize),
                color: None,
            },
            RichMessagePart::LocationId { id, player, .. } => ReboJSONMessagePart {
                _type: String::from("location_id"),
                text: Some(id.to_string()),
                player: Some(*player as isize),
                color: None, flags: None,
            },
            RichMessagePart::LocationName { text, player, .. } => ReboJSONMessagePart {
                _type: String::from("location_name"),
                text: Some(String::from(text)),
                player: Some(*player as isize),
                color: None, flags: None,
            },
            RichMessagePart::EntranceName { text, .. } => ReboJSONMessagePart {
                _type: String::from("entrance_name"),
                text: Some(String::from(text)),
                color: None, flags: None, player: None,
            },
            RichMessagePart::Color { text, color, .. } => ReboJSONMessagePart {
                _type: String::from("color"),
                text: Some(String::from(text)),
                color: Some(json_color_to_string(color)),
                flags: None, player: None,
            },
            RichMessagePart::Text { text, .. } => ReboJSONMessagePart {
                _type: String::from("text"),
                text: Some(String::from(text)),
                color: None, flags: None, player: None,
            },
        }
    }
}

impl ReboNetworkItem {
    pub fn from(item: &NetworkItem) -> ReboNetworkItem {
        ReboNetworkItem {
            item: item.item as isize,
            location: item.location as isize,
            player: item.player as isize,
            flags: Into::<u8>::into(item.flags.clone()) as isize,
        }
    }
}

fn json_color_to_string(json_color: &RichMessageColor) -> String {
    match json_color {
        RichMessageColor::Bold => "bold",
        RichMessageColor::Underline => "underline",
        RichMessageColor::Black => "black",
        RichMessageColor::Red => "red",
        RichMessageColor::Green => "green",
        RichMessageColor::Yellow => "yellow",
        RichMessageColor::Blue => "blue",
        RichMessageColor::Magenta => "magenta",
        RichMessageColor::Cyan => "cyan",
        RichMessageColor::White => "white",
        RichMessageColor::BlackBg => "black_bg",
        RichMessageColor::RedBg => "red_bg",
        RichMessageColor::GreenBg => "green_bg",
        RichMessageColor::YellowBg => "yellow_bg",
        RichMessageColor::BlueBg => "blue_bg",
        RichMessageColor::MagentaBg => "magenta_bg",
        RichMessageColor::CyanBg => "cyan_bg",
        RichMessageColor::WhiteBg => "white_bg",
    }.to_string()
}

/// Check internal state and channels to see if we should stop.
fn interrupt_function<'i>(_vm: &mut VmContext<'i, '_, '_>) -> Result<(), ExecError<'i>> {
    loop {
        let result = STATE.lock().unwrap().as_ref().unwrap().stream_rebo_rx.try_recv();
        match result {
            Ok(res) => match res {
                StreamToRebo::WorkingDir(_) => {
                    log!("Got WorkingDir, but can't set it during execution");
                    panic!()
                }
                StreamToRebo::Start(_, _) => {
                    log!("Got StreamToRebo::Start but rebo is already running");
                    panic!()
                }
                StreamToRebo::Stop => {
                    log!("Should Exit");
                    return Err(ExecError::Panic);
                }
            }
            Err(TryRecvError::Empty) => return Ok(()),
            Err(e) => {
                log!("Error stream_rebo_rx.try_recv: {:?}", e);
                panic!();
            }
        }
    }
}

#[rebo::function(raw("Tas::new_version_string"))]
fn new_version_string() -> Option<String> {
    STATE.lock().unwrap().as_ref().unwrap().new_version_string.clone()
}

#[rebo::function(raw("print"))]
fn print(..: _) {
    let joined = args.as_slice().iter().map(DisplayValue).join(", ");
    log!("{}", joined);
    STATE.lock().unwrap().as_ref().unwrap().rebo_stream_tx.send(ReboToStream::Print(joined)).unwrap();
}

#[rebo::function(raw("Tas::step"))]
fn step() -> Step {
    step_internal(vm, expr_span, Suspend::Return)?
}
#[rebo::function(raw("Tas::yield_"))]
fn step_yield() -> Step {
    step_internal(vm, expr_span, Suspend::Yield)?
}
fn step_internal<'i>(vm: &mut VmContext<'i, '_, '_>, expr_span: Span, suspend: Suspend) -> Result<Step, ExecError<'i>> {
    // get level state before and after we advance the UE frame to see changes created by Refunct itself
    let old_level_state = LevelState::get();

    if let Some(delta) = STATE.lock().unwrap().as_ref().unwrap().delta {
        FApp::set_delta(delta);
    }

    loop {
        let mut to_be_returned = None;
        // yield back to UE stack, but not necessarily the UE loop
        let evt = YIELDER.with(|yielder| unsafe { (*yielder.get()).suspend(suspend) });
        match evt {
            UeEvent::Tick => to_be_returned = Some(Step::Tick),
            UeEvent::ElementPressed(index) => {
                if index.element_type == ElementType::Cube && index.cluster_index == 9999 {
                    maybe_remove_extra_cube(index.element_index as i32);
                }
                element_pressed(vm, index)?
            },
            UeEvent::ElementReleased(index) => element_released(vm, index)?,
            UeEvent::NothingHappened => to_be_returned = Some(Step::Yield),
            UeEvent::NewGame => to_be_returned = Some(Step::NewGame),
            UeEvent::KeyDown(key, repeat) => {
                let key = STATE.lock().unwrap().as_mut().unwrap().ui.key_pressed(key);
                on_key_down(vm, key.raw_key_code, key.raw_character_code, repeat)?
            },
            UeEvent::KeyUp(key, repeat) => {
                let key = STATE.lock().unwrap().as_mut().unwrap().ui.key_released(key);
                on_key_up(vm, key.raw_key_code, key.raw_character_code, repeat)?
            },
            UeEvent::KeyChar(character, repeat) => on_key_char(vm, character.to_string(), repeat)?,
            UeEvent::MouseMove(x, y) => {
                let (absx, absy) = AMyCharacter::get_mouse_position();
                STATE.lock().unwrap().as_mut().unwrap().ui.mouse_moved(absx as u32, absy as u32);
                on_mouse_move(vm, x, y)?
            },
            UeEvent::MouseButtonDown(button) => {
                STATE.lock().unwrap().as_mut().unwrap().ui.mouse_button_pressed(button.to_iced_button());
            },
            UeEvent::MouseButtonUp(button) => {
                STATE.lock().unwrap().as_mut().unwrap().ui.mouse_button_released(button.to_iced_button());
            },
            UeEvent::MouseWheel(delta) => {
                STATE.lock().unwrap().as_mut().unwrap().ui.mouse_wheel(delta);
            },
            UeEvent::DrawHud => {
                // handle events
                loop {
                    let Some(function) = STATE.lock().unwrap().as_mut().unwrap().ui.next_ui_event() else { break };
                    vm.call_bound_function(function, expr_span)?;
                }

                // draw old UI
                draw_hud(vm)?
            },
            UeEvent::ApplyResolutionSettings => {
                let (width, height) = AMyCharacter::get_player().get_viewport_size();
                STATE.lock().unwrap().as_mut().unwrap().ui.resize(width.try_into().unwrap(), height.try_into().unwrap());
                on_resolution_change(vm)?
            },
            UeEvent::AddToScreen => on_menu_open(vm)?,
        }

        // check websocket
        loop {
            let response = match receive_from_server(vm, true) {
                Ok(response) => response,
                Err(ReceiveError::ExecError(err)) => return Err(err),
                Err(ReceiveError::Error) => break,
            };
            match response {
                Response::ServerTime(_) => unreachable!("got Response::ServerTime in step-function"),
                Response::PlayerJoinedRoom(id, name, red, green, blue, x, y, z, pitch, yaw, roll) => player_joined_multiplayer_room(vm, id.id(), name, Color { red, green, blue, alpha: 1. }, Location { x, y, z}, Rotation { pitch, yaw, roll })?,
                Response::PlayerLeftRoom(id) => player_left_multiplayer_room(vm, id.id())?,
                Response::MoveOther(id, x, y, z, pitch, yaw, roll) => player_moved(vm, id.id(), Location { x, y, z }, Rotation { pitch, yaw, roll })?,
                Response::PressPlatform(id) => press_platform(vm, id)?,
                Response::PressButton(id) => press_button(vm, id)?,
                Response::NewGamePressed(id) => player_pressed_new_game(vm, id.id())?,
                Response::StartNewGameAt(timestamp) => {
                    let local_time_offset = STATE.lock().unwrap().as_ref().unwrap().local_time_offset as i64;
                    start_new_game_at(vm, (timestamp as i64 + local_time_offset) as u64)?
                },
                Response::RoomNameTooLong => {
                    disconnected(vm, Disconnected::RoomNameTooLong)?;
                }
            }
        }

        // check archipelago
        loop {
            let res = STATE.lock().unwrap().as_mut().unwrap().archipelago_rebo_rx.try_recv();
            match res {
                Err(TryRecvError::Empty) => break,
                Err(TryRecvError::Disconnected) => panic!("archipelago_rebo_rx became disconnected"),
                Ok(ArchipelagoToRebo::ConnectionFailed(cause)) => {
                    ap_log_error(vm, "Failed connecting to archipelago server:".to_string())?;
                    for line in cause.split("\n") {
                        ap_log_error(vm, format!(" - {}", line.trim().to_string()))?;
                    }
                    archipelago_disconnected(vm)?
                },
                Ok(ArchipelagoToRebo::ConnectionAborted) => {
                    log!("ArchipelagoToRebo::ConnectionAborted");
                    archipelago_disconnected(vm)?
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::RoomInfo(info))) => {
                    log!("RoomInfo message");
                    let msg = format!("Archipelago ServerMessage::RoomInfo: {:?}", info);
                    log!("{}", msg);
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::ConnectionRefused(info))) => {
                    log!("ConnectionRefused message");
                    let msg = format!("Archipelago ServerMessage::ConnectionRefused: {:?}", info);
                    log!("{}", msg);
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::Connected(info))) => {
                    log!("Connected message");
                    let msg = format!("Archipelago ServerMessage::Connected: {:?}", info);
                    log!("{}", msg);
                    // example of message:
                    // Connected { 
                    //    team: 0, 
                    //    slot: 1, 
                    //    players: [NetworkPlayer { team: 0, slot: 1, alias: "Player1", name: "Player1" }], 
                    //    missing_locations: [10010103, 10010104], 
                    //    checked_locations: [10010102, 10010105], 
                    //    slot_data: Object {"ap_world_version": String("0.1.0"), "goal": Number(3)}, 
                    //    slot_info: {1: NetworkSlot { name: "Player1", game: "Refunct", type: Player, group_members: [] }}, 
                    //    hint_points: 2 
                    // }
                    archipelago_init(vm, 0 as usize)?;

                    archipelago_set_own_id(vm, info.team as isize, info.slot as isize)?;

                    for loc in &info.checked_locations {
                        let value: i64 = *loc;
                        archipelago_checked_location(vm,
                            value as usize,
                        )?;
                    }

                    for (key, value) in info.slot_data.as_object().unwrap() {
                        archipelago_received_slot_data(vm,
                            key.clone(),
                            value.clone().to_string()
                        )?;
                    }

                    for (key, value) in info.slot_info {
                        archipelago_register_slot(vm,
                            key as isize,
                            value.name.clone(),
                            value.game.clone(),
                            value.r#type as usize,
                            value.group_members.into_iter().map(|x| x as isize).collect()
                        )?;
                    }

                    for player in info.players {
                        archipelago_register_player(vm,
                            player.team as isize,
                            player.slot as isize,
                            player.alias.clone(),
                            player.name.clone()
                        )?;
                    }
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::ReceivedItems(received))) => {
                    log!("ReceivedItems message");
                    let msg = format!("Archipelago ServerMessage::ReceivedItems: {:?}", received);
                    log!("{}", msg);

                    let mut index = received.index;
                    //loop through received items
                    for net in &received.items {
                        let id = net.item as i32;
                        log!("APAPAP Received Item: {}", id);

                        archipelago_received_item(vm, index as usize, id as usize, received.index as usize)?;

                        index += 1;
                    }
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::LocationInfo(info))) => {
                    log!("LocationInfo message");
                    let msg = format!("Archipelago ServerMessage::LocationInfo: {:?}", info);
                    log!("{}", msg);
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::RoomUpdate(info))) => {
                    log!("RoomUpdate message");
                    let msg = format!("Archipelago ServerMessage::RoomUpdate: {:?}", info);
                    log!("{}", msg);
                    // example output: Archipelago ServerMessage::RoomUpdate: RoomUpdate { version: None, tags: None, password_required: false, permissions: None, hint_cost: None, location_check_points: None, games: None, datapackage_versions: None, datapackage_checksums: None, seed_name: None, time: None, hint_points: Some(3), players: None, checked_locations: Some([10010104]), missing_locations: None }
                    for loc in info.checked_locations.unwrap_or_default() {
                        let value: i64 = loc;
                        archipelago_checked_location(vm, value as usize)?;
                    }
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::Print(text))) => {
                    log!("Print message");
                    let msg = format!("Archipelago ServerMessage::Print: {:?}", text);
                    log!("{}", msg);
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::RichPrint(json))) => {
                    log!("PrintJSON message");
                    let msg = format!("Archipelago ServerMessage::PrintJSON: {:?}", json);
                    log!("{}", msg);

                    archipelago_print_json_message(vm, ReboPrintJSONMessage::from(&json))?;
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::DataPackage(pkg))) => {
                    // log!("DataPackage message");
                    // let msg = format!("Archipelago ServerMessage::DataPackage: {:?}", pkg);
                    // log!("{}", msg);

                    for (game_name, game_data) in pkg.data.games {
                        for (item_name, item_id) in game_data.item_name_to_id {
                            archipelago_register_game_item(vm, game_name.clone(), Arc::unwrap_or_clone(item_name), item_id.to_string())?;
                        }
                        for (location_name, location_id) in game_data.location_name_to_id {
                            archipelago_register_game_location(vm, game_name.clone(), Arc::unwrap_or_clone(location_name), location_id.to_string())?;
                        }
                    }
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::Bounced(info))) => {
                    log!("Bounced message");
                    let msg = format!("Archipelago ServerMessage::Bounced: {:?}", info);
                    log!("{}", msg);
                    match info.data {
                        BounceData::DeathLink(DeathLink { source, cause, .. }) => {
                            log!("[{}] {:?}", source, cause);
                            AMyCharacter::respawn();
                            archipelago_received_death(vm, source, cause.unwrap_or(String::from("")))?;
                        }
                        _ => (),
                    }
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::InvalidPacket(pkt))) => {
                    log!("InvalidPacket message");
                    let msg = format!("Archipelago ServerMessage::InvalidPacket: {:?}", pkt);
                    log!("{}", msg);
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::Retrieved(info))) => {
                    log!("Retrieved message");
                    let msg = format!("Archipelago ServerMessage::Retrieved: {:?}", info);
                    log!("{}", msg);
                },
                Ok(ArchipelagoToRebo::ServerMessage(ServerMessage::SetReply(reply))) => {
                    log!("SetReply message");
                    let msg = format!("Archipelago ServerMessage::SetReply: {:?}", reply);
                    log!("{}", msg);
                },
            }
        }

        // get current timestamp in milliseconds:
        let before = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_millis() as u64;
        let _ = archipelago_trigger_one_cluster_now(vm, before)?;

        match to_be_returned {
            Some(ret) => {
                // call level-state event function
                let new_level_state = LevelState::get();
                if old_level_state != new_level_state {
                    on_level_state_change(vm, old_level_state.clone(), new_level_state)?;
                }
                return Ok(ret)
            },
            None => (),
        }
    }
}

#[rebo::required_rebo_functions]
extern "rebo" {
    fn element_pressed(index: ElementIndex);
    fn element_released(index: ElementIndex);
    fn on_key_down(key_code: i32, character_code: u32, is_repeat: bool);
    fn on_key_up(key_code: i32, character_code: u32, is_repeat: bool);
    fn on_key_char(character: String, is_repeat: bool);
    fn on_mouse_move(x: i32, y: i32);
    fn draw_hud();
    fn player_joined_multiplayer_room(id: u32, name: String, col: Color, loc: Location, rot: Rotation);
    fn player_left_multiplayer_room(id: u32);
    fn player_moved(id: u32, loc: Location, rot: Rotation);
    fn press_platform(id: u8);
    fn press_button(id: u8);
    fn player_pressed_new_game(id: u32);
    fn start_new_game_at(timestamp: u64);
    fn disconnected(reason: Disconnected);
    fn archipelago_disconnected();
    fn on_level_state_change(old: LevelState, new: LevelState);
    fn on_resolution_change();
    fn on_menu_open();
    fn archipelago_received_item(index: usize, item_id: usize, starting_index: usize);
    fn archipelago_got_grass();
    fn archipelago_checked_location(id: usize);
    fn archipelago_received_slot_data(name_of_options: String, value_of_options: String);
    fn archipelago_init(gamemode: usize);
    fn archipelago_set_own_id(team: isize, slot: isize);
    fn archipelago_register_slot(index: isize, name: String, game: String, slot_type: usize, group_members: Vec<isize>);
    fn archipelago_register_player(team: isize, slot: isize, alias: String, name: String);
    fn archipelago_register_game_item(game_name: String, item_name: String, item_id: String);
    fn archipelago_register_game_location(game_name: String, location_name: String, location_id: String);
    fn archipelago_print_json_message(json_message: ReboPrintJSONMessage);
    fn archipelago_received_death(source: String, cause: String);
    fn archipelago_trigger_one_cluster_now(time: u64);
    fn ap_log_error(message: String);
}

fn config_path() -> PathBuf {
    let cfg_dir = dirs::config_dir().unwrap()
        .join("refunct-tas");
    if !cfg_dir.is_dir() {
        std::fs::create_dir(&cfg_dir).unwrap();
    }
    cfg_dir
}
pub(super) fn data_path() -> PathBuf {
    let cfg_dir = dirs::data_dir().unwrap()
        .join("refunct-tas");
    if !cfg_dir.is_dir() {
        std::fs::create_dir(&cfg_dir).unwrap();
    }
    cfg_dir
}

#[rebo::function("Tas::load_settings")]
fn load_settings() -> Option<Map<String, String>> {
    let path = config_path().join("settings.json");
    let file = File::open(path).ok()?;
    let map: HashMap<String, String> = serde_json::from_reader(file).unwrap();
    Some(Map::new(map))
}
#[rebo::function("Tas::store_settings")]
fn store_settings(settings: Map<String, String>) {
    let path = config_path().join("settings.json");
    let mut file = File::create(path).unwrap();
    let map: HashMap<_, _> = settings.clone_map();
    serde_json::to_writer_pretty(&mut file, &map).unwrap();
    writeln!(file).unwrap();
}

#[derive(Serialize, Deserialize)]
struct Recording {
    version: i32,
    author: String,
    steam_id: u64,
    filename: String,
    frame_count: i64,
    recording_start_timestamp: DateTime<Local>,
    recording_end_timestamp: DateTime<Local>,
    recording_save_timestamp: DateTime<Local>,
    base_speed: f32,
    max_walk_speed: f32,
    max_bonus_speed: f32,
    frames: Vec<RecordFrame>,
}
#[derive(rebo::ExternalType, Serialize, Deserialize, Clone)]
struct RecordFrame {
    delta: f64,
    events: Vec<InputEvent>,
    location: Location,
    rotation: Rotation,
    velocity: Velocity,
    acceleration: Acceleration,
}
#[derive(rebo::ExternalType, Serialize, Deserialize, Clone)]
enum InputEvent {
    KeyPressed(i32),
    KeyReleased(i32),
    MouseMoved(i32, i32),
}
fn recording_path() -> PathBuf {
    let appdata_path = data_path();
    let recording_path = appdata_path.join("recordings/");
    if !recording_path.is_dir() {
        std::fs::create_dir(&recording_path).unwrap();
    }
    recording_path
}
#[rebo::function("Tas::list_recordings")]
fn list_recordings() -> Vec<String> {
    let path = recording_path();
    std::fs::read_dir(path).unwrap().flatten()
        .map(|entry| {
            assert!(entry.file_type().unwrap().is_file());
            entry.file_name().into_string().unwrap()
        }).collect()
}
#[rebo::function("Tas::save_recording")]
fn save_recording(filename: String, frames: Vec<RecordFrame>, recording_start_timestamp: u64, recording_end_timestamp: u64) {
    let recording = Recording {
        version: 1,
        author: AMyCharacter::get_player().get_player_name(),
        steam_id: AMyCharacter::get_player().get_steamid(),
        filename: filename.clone(),
        frame_count: frames.len() as i64,
        recording_start_timestamp: DateTime::from_timestamp_millis(recording_start_timestamp as i64).unwrap().into(),
        recording_end_timestamp: DateTime::from_timestamp_millis(recording_end_timestamp as i64).unwrap().into(),
        recording_save_timestamp: Local::now(),
        base_speed: AMyCharacter::get_base_speed(),
        max_walk_speed: AMyCharacter::get_max_walk_speed(),
        max_bonus_speed: AMyCharacter::get_max_bonus_speed(),
        frames,
    };
    let filename = sanitize_filename::sanitize(filename);
    let path = recording_path().join(filename);
    let file = File::create(path).unwrap();
    serde_json::to_writer_pretty(file, &recording).unwrap();
}
#[rebo::function("Tas::load_recording")]
fn load_recording(filename: String) -> Vec<RecordFrame> {
    let filename = sanitize_filename::sanitize(filename);
    let path = recording_path().join(filename);
    let content = std::fs::read_to_string(path).unwrap();
    let res: Recording = serde_json::from_str(&content).unwrap();
    res.frames
}
#[rebo::function("Tas::remove_recording")]
fn remove_recording(filename: String) -> bool {
    let filename = sanitize_filename::sanitize(filename);
    let path = recording_path().join(filename);
    std::fs::remove_file(path).is_ok()
}

#[rebo::function("Tas::key_down")]
fn key_down(key_code: i32, character_code: u32, is_repeat: bool) {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    state.pressed_keys.insert(key_code);
    state.hooks.fslateapplication.press_key(key_code, character_code, is_repeat);
}
#[rebo::function("Tas::key_up")]
fn key_up(key_code: i32, character_code: u32, is_repeat: bool) {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    state.pressed_keys.remove(&key_code);
    state.hooks.fslateapplication.release_key(key_code, character_code, is_repeat)
}
#[rebo::function("Tas::move_mouse")]
fn move_mouse(x: i32, y: i32) {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    state.hooks.fslateapplication.move_mouse(x, y);
}
#[rebo::function("Tas::get_last_frame_delta")]
fn get_last_frame_delta() -> f64 {
    let delta = STATE.lock().unwrap().as_mut().unwrap().delta;
    match delta {
        Some(delta) => delta,
        None => FApp::delta(),
    }
}
#[rebo::function("Tas::get_delta")]
fn get_delta() -> Option<f64> {
    STATE.lock().unwrap().as_mut().unwrap().delta
}
#[rebo::function("Tas::set_delta")]
fn set_delta(delta: Option<f64>) {
    STATE.lock().unwrap().as_mut().unwrap().delta = delta;
}
#[derive(Debug, Clone, Copy, rebo::ExternalType, Serialize, Deserialize)]
struct Location {
    x: f32,
    y: f32,
    z: f32,
}
#[rebo::function("Tas::get_location")]
fn get_location() -> Location {
    let (x, y, z) = AMyCharacter::get_player().location();
    Location { x, y, z }
}
#[rebo::function("Tas::get_location_and_log")]
fn get_location_and_log() {
    let (x, y, z) = AMyCharacter::get_player().location();
    log!("LOG get_location: x={}, y={}, z={}", x, y, z);
}
#[rebo::function("Tas::set_location")]
fn set_location(loc: Location) {
    log!("TELEPORTED to x={}, y={}, z={}", loc.x, loc.y, loc.z);
    AMyCharacter::get_player().set_location(loc.x, loc.y, loc.z);
}
#[derive(Debug, Clone, Copy, rebo::ExternalType, Serialize, Deserialize)]
struct Rotation {
    pitch: f32,
    yaw: f32,
    roll: f32,
}
#[rebo::function("Tas::get_rotation")]
fn get_rotation() -> Rotation {
    let (pitch, yaw, roll) = AMyCharacter::get_player().rotation();
    Rotation { pitch, yaw, roll }
}
#[rebo::function("Tas::set_rotation")]
fn set_rotation(rot: Rotation) {
    AMyCharacter::get_player().set_rotation(rot.pitch, rot.yaw, rot.roll);
}
#[rebo::function("Tas::get_player_name")]
fn get_player_name() -> String {
    AMyCharacter::get_player().get_player_name()
}
#[rebo::function("Tas::get_steamid")]
fn get_steamid() -> u64 {
    AMyCharacter::get_player().get_steamid()
}
#[derive(Debug, Clone, Copy, rebo::ExternalType, Serialize, Deserialize)]
struct Velocity {
    x: f32,
    y: f32,
    z: f32,
}
#[rebo::function("Tas::get_velocity")]
fn get_velocity() -> Velocity {
    let (x, y, z) = AMyCharacter::get_player().velocity();
    Velocity { x, y, z }
}
#[rebo::function("Tas::set_velocity")]
fn set_velocity(vel: Velocity) {
    AMyCharacter::get_player().set_velocity(vel.x, vel.y, vel.z);
}
#[derive(Debug, Clone, Copy, rebo::ExternalType, Serialize, Deserialize)]
struct Acceleration {
    x: f32,
    y: f32,
    z: f32,
}
#[rebo::function("Tas::get_acceleration")]
fn get_acceleration() -> Acceleration {
    let (x, y, z) = AMyCharacter::get_player().acceleration();
    Acceleration { x, y, z }
}
#[rebo::function("Tas::set_acceleration")]
fn set_acceleration(acc: Acceleration) {
    AMyCharacter::get_player().set_acceleration(acc.x, acc.y, acc.z);
}
#[rebo::function("Tas::get_movement_mode")]
fn get_movement_mode() -> u8 {
    AMyCharacter::get_player().movement_mode()
}
#[rebo::function("Tas::set_movement_mode")]
fn set_movement_mode(mode: u8) {
    AMyCharacter::get_player().set_movement_mode(mode);
}
#[rebo::function("Tas::get_max_fly_speed")]
fn get_max_fly_speed() -> f32 {
    AMyCharacter::get_player().max_fly_speed()
}
#[rebo::function("Tas::set_max_fly_speed")]
fn set_max_fly_speed(speed: f32) {
    AMyCharacter::get_player().set_max_fly_speed(speed);
}
#[rebo::function("Tas::get_max_walk_speed")]
fn get_max_walk_speed() -> f32 {
    AMyCharacter::get_max_walk_speed()
}
#[rebo::function("Tas::get_base_speed")]
fn get_base_speed() -> f32 {
    AMyCharacter::get_base_speed()
}
#[rebo::function("Tas::get_max_bonus_speed")]
fn get_max_bonus_speed() -> f32 {
    AMyCharacter::get_max_bonus_speed()
}
#[rebo::function("Tas::get_level_state")]
fn get_level_state() -> LevelState {
    LevelState::get()
}
#[rebo::function("Tas::restart_game")]
fn restart_game() {
    UMyGameInstance::restart_game();
}
#[rebo::function(raw("Tas::wait_for_new_game"))]
fn wait_for_new_game() {
    loop {
        match step_internal(vm, expr_span, Suspend::Return)? {
            Step::Tick => continue,
            Step::NewGame => break,
            Step::Yield => unreachable!("step_internal(StepKind::Step) returned Yield"),
        }
    }
}
#[derive(Debug, Clone, Copy, rebo::ExternalType)]
struct Line {
    startx: f32,
    starty: f32,
    endx: f32,
    endy: f32,
    color: Color,
    thickness: f32,
}
#[derive(Debug, Clone, Copy, rebo::ExternalType)]
pub struct Color {
    pub red: f32,
    pub green: f32,
    pub blue: f32,
    pub alpha: f32,
}
#[rebo::function("Tas::draw_line")]
fn draw_line(line: Line) {
    let color = (line.color.red, line.color.green, line.color.blue, line.color.alpha);
    AMyHud::draw_line(line.startx, line.starty, line.endx, line.endy, color, line.thickness);
}
#[derive(Debug, Clone, rebo::ExternalType)]
struct DrawText {
    text: String,
    color: Color,
    x: f32,
    y: f32,
    scale: f32,
    scale_position: bool,
}
#[rebo::function("Tas::draw_text")]
fn draw_text(text: DrawText) {
    let color = (text.color.red, text.color.green, text.color.blue, text.color.alpha);
    let render_text = replace_unrenderable_chars(text.text);
    AMyHud::draw_text(render_text, color, text.x, text.y, text.scale, text.scale_position);
}
#[rebo::function("Tas::draw_rect")]
fn draw_rect(color: Color, x: f32, y: f32, width: f32, height: f32) {
    let color = FLinearColor { red: color.red, green: color.green, blue: color.blue, alpha: color.alpha };
    AMyHud::draw_rect(color, x, y, width, height);
}
#[rebo::function("Tas::draw_minimap")]
fn draw_minimap(x: f32, y: f32, scale: f32, scale_position: bool) {
    AMyHud::draw_texture_simple(STATE.lock().unwrap().as_ref().unwrap().minimap_texture.as_ref().unwrap(), x, y, scale, scale_position);
}
#[rebo::function("Tas::set_minimap_alpha")]
fn set_minimap_alpha(alpha: f32) {
    let mut lock = STATE.lock().unwrap();
    let state = lock.as_mut().unwrap();
    let mut image = state.minimap_image.clone();
    for pixel in image.pixels_mut() {
        pixel.0[3] = (255.0 * alpha).round() as u8;
    }
    state.minimap_texture.as_mut().unwrap().set_image(&image);
}
#[derive(Debug, Clone, Copy, rebo::ExternalType)]
struct Size {
    width: i32,
    height: i32,
}
#[rebo::function("Tas::minimap_size")]
fn minimap_size() -> Size {
    let lock = STATE.lock().unwrap();
    let minimap = lock.as_ref().unwrap().minimap_texture.as_ref().unwrap();
    Size {
        width: minimap.width(),
        height: minimap.height(),
    }
}
#[rebo::function("Tas::draw_player_minimap")]
fn draw_player_minimap(x: f32, y: f32, width: f32, height: f32, rotation_degrees: f32, color: Color) {
    let mut lock = STATE.lock().unwrap();
    let state = lock.as_mut().unwrap();
    let color = Rgba([
        (255.0 * color.red).round() as u8,
        (255.0 * color.green).round() as u8,
        (255.0 * color.blue).round() as u8,
        (255.0 * color.alpha).round() as u8,
    ]);
    let texture = &mut state.player_minimap_textures.entry(color)
        .or_insert_with(|| {
            let mut image = state.player_minimap_image.clone();
            for pixel in image.pixels_mut() {
                if *pixel == Rgba([0, 0, 0, 255]) {
                    *pixel = color;
                } else if pixel.0[3] != 0 {
                    pixel.0[3] = color.0[3];
                }
            }
            UTexture2D::create(&image)
        });
    AMyHud::draw_texture(
        texture, x, y, width, height, 0., 0., 1., 1.,
        (1., 1., 1.), EBlendMode::Translucent, 1., false,
        rotation_degrees, 0.5, 0.5
    );
}
#[rebo::function("Tas::player_minimap_size")]
fn player_minimap_size() -> Size {
    let lock = STATE.lock().unwrap();
    let image = &lock.as_ref().unwrap().player_minimap_image;
    Size {
        width: image.width().try_into().unwrap(),
        height: image.height().try_into().unwrap(),
    }
}
#[derive(Debug, Clone, Copy, rebo::ExternalType)]
struct Vector {
    x: f32,
    y: f32,
    z: f32,
}
#[rebo::function("Tas::project")]
fn project(vec: Vector) -> Vector {
    let (x, y, z) = AMyHud::project(vec.x, vec.y, vec.z);
    Vector { x, y, z }
}
#[derive(Debug, Clone, Copy, rebo::ExternalType)]
struct TextSize {
    width: f32,
    height: f32,
}
#[rebo::function("Tas::get_text_size")]
fn get_text_size(text: String, scale: f32) -> TextSize {
    let (width, height) = AMyHud::get_text_size(text, scale);
    TextSize { width, height }
}
#[rebo::function("Tas::get_viewport_size")]
fn get_viewport_size() -> Size {
    let (width, height) = AMyCharacter::get_player().get_viewport_size();
    Size { width, height }
}

#[rebo::function("Tas::spawn_platform_rando_location")]
fn spawn_platform_rando_location(max: f32) -> i32 {
    let rx = rand::random::<f32>();
    let ry = rand::random::<f32>();
    let rz = rand::random::<f32>();
    let loc = Location { x: (rx-0.5) * 2. * max as f32, y: (ry-0.5) * 2. * max as f32, z: rz * max as f32 };
    spawn_platform(loc)
}
#[rebo::function("Tas::spawn_cube_rando_location")]
fn spawn_cube_rando_location(max: f32, spawn_platform_below: bool) -> i32 {
    let rx = rand::random::<f32>();
    let ry = rand::random::<f32>();
    let rz = rand::random::<f32>();
    let loc = Location { x: (rx-0.5) * 2. * max as f32, y: (ry-0.5) * 2. * max as f32, z: rz * max as f32 };
    if spawn_platform_below {
        let mut platform_loc = loc;
        platform_loc.x -= 125.;
        platform_loc.y -= 125.;
        platform_loc.z -= 250.;
        spawn_platform(platform_loc); // this id won't be returned
    }
    spawn_cube(loc)
}

#[rebo::function("Tas::spawn_platform")]
fn spawn_platform_rebo(loc: Location) -> i32 {
    spawn_platform(loc)
}
fn spawn_platform(loc: Location) -> i32 {
    match crate::native::PlatformWrapper::spawn(loc.x, loc.y, loc.z) {
        Ok(platform) => {
            let index = platform.internal_index();
            STATE.lock().unwrap().as_mut().unwrap().extra_platforms.push(index);
            log!("Successfully spawned platform at {:p} with internal index {}", platform.as_ptr(), index);
            index
        }
        Err(e) => {
            panic!("Failed to spawn platform: {}", e);
        }
    }
}

#[rebo::function("Tas::destroy_platforms")]
fn destroy_platforms(vanilla: bool, extra: bool) {
    if vanilla {
        // don't :)
    }
    if extra {
        let non_vanilla_platforms = get_non_vanilla_platforms();
        for p in non_vanilla_platforms {
            destroy_platform(p);
        }
    }
}
#[rebo::function("Tas::spawn_cube")]
fn spawn_cube_rebo(loc: Location) -> i32 {
    spawn_cube(loc)
}
fn spawn_cube(loc: Location) -> i32 {
    match CubeWrapper::spawn(loc.x, loc.y, loc.z) {
        Ok(cube) => {
            let index = cube.internal_index();
            STATE.lock().unwrap().as_mut().unwrap().extra_cubes.push(index);
            log!("Successfully spawned cube at {:p} with internal index {}", cube.as_ptr(), cube.internal_index());
            index
        }
        Err(e) => {
            panic!("Failed to spawn cube: {}", e);
        }
    }
}



fn get_uncollected_vanilla_cubes() -> Vec<i32> {
    UeScope::with(|scope| {
        LEVELS.lock().unwrap().iter().flat_map(|level| {
            level.cubes.iter()
                .filter_map(|&cube| {
                    let cube = scope.get(cube);
                    match cube.is_picked_up() {
                        true => None,
                        false => Some(cube.internal_index())
                    }
                })
                .collect::<Vec<i32>>()
        }).collect::<Vec<i32>>()
    })
}

fn get_uncollected_extra_cubes() -> Vec<i32> {
    let state = STATE.lock().unwrap();

    state.as_ref().unwrap()
        .extra_cubes.iter()
        .filter_map(|&cube_index| {
            match find_cube_and(cube_index, |cube| cube.is_picked_up()) {
                Ok(false) => Some(cube_index),
                Ok(true) | Err(_) => None,
            }
        })
        .collect::<Vec<i32>>()
}

fn get_non_vanilla_platforms() -> Vec<i32> {
    let state = STATE.lock().unwrap();

    state.as_ref().unwrap()
        .extra_platforms.iter()
        .filter_map(|&platform_index| {
            match find_platform_and(platform_index, |_platform| ()) {
                Ok(_) => Some(platform_index),
                Err(_) => None,
            }
        })
        .collect::<Vec<i32>>()
}

#[rebo::function("Tas::reset_cubes")]
fn reset_cubes(vanilla: bool, extra: bool) {
    if vanilla {
        UeScope::with(|scope| {
            LEVELS.lock().unwrap().iter().for_each(|level| {
                level.cubes.iter().for_each(|cube| scope.get(cube).reset());
            });
        });
    }
    if extra {
        let extra_cubes = get_uncollected_extra_cubes();
        log!("Resetting extra cubes: {:?}", extra_cubes);
        for c in extra_cubes {
            maybe_remove_extra_cube(c);
            find_cube_and(c, |cube| cube.reset())
                .unwrap_or_else(|e| log!("Could not reset cube {:?}: {}", c, e));
        }
    }
}

#[rebo::function("Tas::destroy_cubes")]
fn destroy_cubes(vanilla: bool, extra: bool) {
    if vanilla {
        UeScope::with(|scope| {
            LEVELS.lock().unwrap().iter().for_each(|level| {
                level.cubes.iter().for_each(|cube| {
                    let cube = scope.get(cube);
                    cube.pickup();
                    cube.set_color(0., 0., 0.);
                });
            });
        });
    }
    if extra {
        let extra_cubes = get_uncollected_extra_cubes();
        log!("Destroying extra cubes: {:?}", extra_cubes);
        for c in extra_cubes {
            maybe_remove_extra_cube(c);
            find_cube_and(c, |cube| cube._destroy())
                .unwrap_or_else(|e| log!("Could not reset cube {:?}: {}", c, e));
        }
    }
}

#[rebo::function("Tas::get_vanilla_cubes")]
fn get_vanilla_cubes() -> Vec<i32> {
    get_uncollected_vanilla_cubes()
}

#[rebo::function("Tas::get_non_vanilla_cubes")]
fn get_non_vanilla_cubes() -> Vec<i32> {
    get_uncollected_extra_cubes()
}
#[rebo::function("Tas::get_non_vanilla_platforms")]
fn get_non_vanilla_platforms_rebo() -> Vec<i32> {
    get_non_vanilla_platforms()
}
#[rebo::function("Tas::get_all_cubes")]
fn get_all_cubes() -> Vec<i32> {
    let mut cubes = get_uncollected_vanilla_cubes();
    cubes.extend(get_uncollected_extra_cubes());

    cubes
}

fn find_cube_and<R: 'static, F: FnOnce(&CubeWrapper) -> R>(internal_index: i32, f: F) -> Result<R, String> {
    UeScope::with(|scope| {
        let item = scope.object_array().try_get(internal_index)
            .ok_or_else(|| format!("Failed to find cube at index {}", internal_index))?;

        match item.object().try_upcast::<CubeWrapper>() {
            Some(cube) => Ok(f(&cube)),
            None => Err(format!("Failed to find cube {}", internal_index))
        }
    })
}

fn find_platform_and<R: 'static, F: FnOnce(&PlatformWrapper) -> R>(internal_index: i32, f: F) -> Result<R, String> {
    UeScope::with(|scope| {
        let item = scope.object_array().try_get(internal_index)
            .ok_or_else(|| format!("Failed to find platform at index {}", internal_index))?;

        match item.object().try_upcast::<PlatformWrapper>() {
            Some(platform) => Ok(f(&platform)),
            None => Err(format!("Failed to find platform {}", internal_index))
        }
    })
}

#[rebo::function("Tas::get_vanilla_cube")]
fn get_vanilla_cube(cluster: usize, element_index: usize) -> Option<i32> {
    UeScope::with(|scope| {
        for (c, level) in LEVELS.lock().unwrap().iter().enumerate() {
            if c != cluster { continue; }

            for (i, cube_index) in level.cubes.iter().enumerate() {
                if i == element_index { return Some(scope.get(cube_index).internal_index()); }
            }
        }

        return None;
    })
}

#[rebo::function("Tas::set_cube_collision")]
fn set_cube_collision(internal_index: i32, collision_enabled: bool) {
    find_cube_and(internal_index, |cube| cube.set_collision(collision_enabled))
        .unwrap_or_else(|e| log!("Could not set collision for cube {:?}: {}", internal_index, e));
}

#[rebo::function("Tas::set_cube_color")]
fn set_cube_color_rebo(internal_index: i32, c: Color) -> i32 {
    set_cube_color(internal_index, c)
}

fn set_cube_color(internal_index: i32, c: Color) -> i32 {
    // Sadly alpha seems to be ignored, so we're just going to silently drop it
    find_cube_and(internal_index, |cube| cube.set_color(c.red, c.green, c.blue))
        .unwrap_or_else(|e| log!("Could not set color for cube {:?}: {}", internal_index, e));
    internal_index
}

#[rebo::function("Tas::set_cube_color_random")]
fn set_cube_color_random(internal_index: i32) {
    find_cube_and(internal_index, |cube| {
        let mut r = rand::random::<f32>();
        let mut g = rand::random::<f32>();
        let mut b = rand::random::<f32>();
        match rand::random::<u8>() % 3 {
            0 => r = 1.0,
            1 => g = 1.0,
            _ => b = 1.0,
        }
        cube.set_color(r, g, b);
    }).unwrap_or_else(|e| log!("Could not set random color for cube {:?}: {}", internal_index, e));
}

#[rebo::function("Tas::set_cube_location")]
fn set_cube_location(internal_index: i32, loc: Location) {
    find_cube_and(internal_index, |cube| cube.set_location(loc.x, loc.y, loc.z))
        .unwrap_or_else(|e| log!("Could not set location for cube {:?}: {}", internal_index, e));
}

#[rebo::function("Tas::set_cube_scale")]
fn set_cube_scale(internal_index: i32, s: f32) -> i32 {
    find_cube_and(internal_index, |cube| cube.set_scale(s))
        .unwrap_or_else(|e| log!("Could not set scale for cube {:?}: {}", internal_index, e));
    internal_index
}

#[rebo::function("Tas::collect_cube")]
fn collect_cube(internal_index: i32) -> i32 {
    maybe_remove_extra_cube(internal_index);
    find_cube_and(internal_index, |cube| cube.pickup())
        .unwrap_or_else(|e| log!("Could not pickup cube {:?}: {}", internal_index, e));
    internal_index 
}
#[rebo::function("Tas::destroy_platform")]
fn destroy_platform_rebo(internal_index: i32) {
    destroy_platform(internal_index);
}

fn destroy_platform(internal_index: i32) {
    maybe_remove_extra_platform(internal_index);
    let _ = find_platform_and(internal_index, |platform| platform.destroy());
}

pub fn maybe_remove_extra_cube(internal_index: i32) -> bool {
    if let Some(state) = STATE.lock().unwrap().as_mut() {
        if let Some(pos) = state.extra_cubes.iter().position(|i| *i == internal_index) {
            state.extra_cubes.remove(pos);
            return true;
        }
    }
    false
}

pub fn maybe_remove_extra_platform(internal_index: i32) -> bool {
    if let Some(state) = STATE.lock().unwrap().as_mut() {
        if let Some(pos) = state.extra_platforms.iter().position(|i| *i == internal_index) {
            state.extra_platforms.remove(pos);
            return true;
        }
    }
    false
}

#[rebo::function("Tas::abilities_set_pipes")]
fn abilities_set_pipes(enabled: bool) {
    log!("Archipelago: setting pipes to {}", enabled);
    UeScope::with(|scope| {
        for level in LEVELS.lock().unwrap().iter() {
            for pipe in level.pipes.iter() {
                scope.get(pipe).set_enabled(enabled);
            }
        }
    });
}

#[rebo::function("Tas::abilities_set_lifts")]
fn abilities_set_lifts(enabled: bool) {
    log!("Archipelago: setting lifts to {}", enabled);
    UeScope::with(|scope| {
        for level in LEVELS.lock().unwrap().iter() {
            for lift in level.lifts.iter() {
                scope.get(lift).set_enabled(enabled);
            }
        }

        // TODO: figure out why changing this volume occasionally crashes the game
        // let new_volume = if enabled { 1.0 } else { 0.0 };
        // log!("Setting lift sound volume to {}", new_volume);
        // for item in scope.iter_global_object_array() {
        //     let object = item.object();
        //     let obj_name = object.name();
        //
        //     if matches!(obj_name.as_str(), "LiftDownStart_Cue" | "LiftDownStop_Cue" | "LiftUpStart_Cue" | "LiftUpStop_Cue")
        //     {
        //         let volume = object.get_field("VolumeMultiplier").unwrap::<&Cell<f32>>();
        //         volume.set(new_volume);
        //         log!("  Set volume of {} to {new_volume}", obj_name);
        //     }
        // }
    });
}

#[rebo::function("Tas::abilities_set_swim")]
fn abilities_set_swim(enabled: bool) {
    log!("Archipelago: setting swim to {}", enabled);
    if enabled {
        UWorld::set_kill_z(-6000.);
    }else{
        UWorld::set_kill_z(-60.);
    }
}

#[rebo::function("Tas::spawn_pawn")]
fn spawn_pawn(loc: Location, rot: Rotation) -> u32 {
    let my_character = UWorld::spawn_amycharacter(loc.x, loc.y, loc.z, rot.pitch, rot.yaw, rot.roll);
    let id = STATE.lock().unwrap().as_mut().unwrap().pawn_id;
    STATE.lock().unwrap().as_mut().unwrap().pawn_id += 1;
    STATE.lock().unwrap().as_mut().unwrap().pawns.insert(id, my_character);
    id
}
#[rebo::function("Tas::destroy_pawn")]
fn destroy_pawn(pawn_id: u32) {
    let my_character = STATE.lock().unwrap().as_mut().unwrap().pawns.remove(&pawn_id).expect("pawn_id not valid anymore");
    UWorld::destroy_amycharaccter(my_character);
}
#[rebo::function("Tas::move_pawn")]
fn move_pawn(pawn_id: u32, loc: Location) {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    let my_character = state.pawns.get_mut(&pawn_id).expect("pawn_id not valid");
    my_character.set_location(loc.x, loc.y, loc.z);
}
#[rebo::function("Tas::set_pawn_velocity")]
fn set_pawn_velocity(pawn_id: u32, vel: Velocity) {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    let my_character = state.pawns.get_mut(&pawn_id).expect("pawn_id not valid");
    my_character.set_velocity(vel.x, vel.y, vel.z);
}
#[rebo::function("Tas::pawn_location")]
fn pawn_location(pawn_id: u32) -> Location {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    let my_character = state.pawns.get_mut(&pawn_id).expect("pawn_id not valid");
    let (x, y, z) = my_character.location();
    Location { x, y, z }
}


// SERVER / WEBSOCKET

#[derive(rebo::ExternalType)]
enum Server {
    Localhost,
    Remote,
    Testing,
}

#[rebo::function(raw("Tas::connect_to_server"))]
fn connect_to_server(server: Server) {
    let address = match server {
        Server::Localhost => "ws://localhost:8080/ws",
        Server::Remote => "wss://refunct-tas.oberien.de/ws",
        Server::Testing => "wss://refunct-tas-test.oberien.de/ws",
    };
    let client = ClientBuilder::new(address).unwrap().connect(None);
    let client = match client {
        Ok(client) => client,
        Err(e) => {
            log!("couldn't connect to server: {e:?}");
            disconnected(vm, Disconnected::ConnectionRefused)?;
            return Ok(Value::Unit)
        }
    };
    log!("connected to server, figuring out time delta");
    STATE.lock().unwrap().as_mut().unwrap().websocket = Some(client);

    // time delta calculation
    let mut deltas: Vec<i32> = vec![0];
    let delta = loop {
        deltas.sort();
        let median = deltas[deltas.len() / 2];
        let matches = deltas.iter().copied().filter(|&m| (m - median).abs() < 100).count();
        if deltas.len() > 5 && matches as f64 / deltas.len() as f64 > 0.8 {
            break median;
        }
        if deltas.len() > 20 {
            log!("connection too unstable to get local time offset: {deltas:?}");
            disconnected(vm, Disconnected::LocalTimeOffsetTooManyTries)?;
            return Ok(Value::Unit);
        }

        let before = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_millis();
        send_to_server(vm, "timesync", Request::GetServerTime)?;
        let remote_time = match receive_from_server(vm, false) {
            Err(ReceiveError::Error) => return Ok(Value::Unit),
            Err(ReceiveError::ExecError(err)) => return Err(err),
            Ok(Response::ServerTime(time)) => time,
            Ok(response) => unreachable!("got non-ServerTime during deltatime calculation: {response:?}"),
        };
        let after = std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).unwrap().as_millis();
        let local_time = ((before + after) / 2) as u64;
        let delta = local_time as i64 - remote_time as i64;
        deltas.push(delta as i32);
        std::thread::sleep(Duration::from_millis(500));
    };
    let msg = format!("local time offset between us and server: {delta} ms");
    log!("{}", msg);
    STATE.lock().unwrap().as_ref().unwrap().rebo_stream_tx.send(ReboToStream::Print(msg)).unwrap();

    STATE.lock().unwrap().as_mut().unwrap().local_time_offset = delta;
}
#[rebo::function(raw("Tas::disconnect_from_server"))]
fn disconnect_from_server() {
    STATE.lock().unwrap().as_mut().unwrap().websocket.take();
    disconnected(vm, Disconnected::ManualDisconnect)?;
}
fn send_to_server<'i>(vm: &mut VmContext<'i, '_, '_>, desc: &str, request: Request) -> Result<(), ExecError<'i>> {
    let mut state = STATE.lock().unwrap();
    let state = state.as_mut().unwrap();
    if state.websocket.is_none() {
        log!("called {desc} without active websocket session");
        // TODO: error propagation?
        return Ok(());
    }
    let msg = serde_json::to_string(&request).unwrap();
    let msg = Message::text(msg);
    if let Err(e) = state.websocket.as_mut().unwrap().send_message(&msg) {
        log!("error sending {desc} request: {e:?}");
        state.websocket.take();
        disconnected(vm, Disconnected::SendFailed)?;
    }
    Ok(())
}
enum ReceiveError<'i> {
    ExecError(ExecError<'i>),
    Error,
}
impl<'i> From<ExecError<'i>> for ReceiveError<'i> {
    fn from(e: ExecError<'i>) -> Self {
        ReceiveError::ExecError(e)
    }
}
fn receive_from_server<'i>(vm: &mut VmContext<'i, '_, '_>, nonblocking: bool) -> Result<Response, ReceiveError<'i>> {
    if STATE.lock().unwrap().as_ref().unwrap().websocket.is_none() {
        return Err(ReceiveError::Error);
    }
    loop {
        if nonblocking {
            STATE.lock().unwrap().as_mut().unwrap().websocket.as_mut().unwrap().set_nonblocking(true).unwrap();
        }
        let res = STATE.lock().unwrap().as_mut().unwrap().websocket.as_mut().unwrap().recv_message();
        if nonblocking {
            STATE.lock().unwrap().as_mut().unwrap().websocket.as_mut().unwrap().set_nonblocking(false).unwrap();
        }
        return match res {
            Ok(OwnedMessage::Text(text)) => Ok(serde_json::from_str(&text).unwrap()),
            Ok(OwnedMessage::Binary(_) | OwnedMessage::Ping(_) | OwnedMessage::Pong(_)) => continue,
            Err(WebSocketError::IoError(io)) if nonblocking && io.kind() == ErrorKind::WouldBlock => Err(ReceiveError::Error),
            Ok(OwnedMessage::Close(_)) => {
                drop(STATE.lock().unwrap().as_mut().unwrap().websocket.take());
                disconnected(vm, Disconnected::Closed)?;
                Err(ReceiveError::Error)
            },
            Err(_) => {
                drop(STATE.lock().unwrap().as_mut().unwrap().websocket.take());
                disconnected(vm, Disconnected::ReceiveFailed)?;
                Err(ReceiveError::Error)
            }
        };
    }
}
#[rebo::function(raw("Tas::join_multiplayer_room"))]
fn join_multiplayer_room(room: String, name: String, col: Color, loc: Location, rot: Rotation) {
    send_to_server(vm, "join room", Request::JoinRoom(room, name, col.red, col.green, col.blue, loc.x, loc.y, loc.z, rot.pitch, rot.yaw, rot.roll))?;
}
#[rebo::function(raw("Tas::move_on_server"))]
fn move_on_server(loc: Location, rot: Rotation) {
    send_to_server(vm, "move", Request::MoveSelf(loc.x, loc.y, loc.z, rot.pitch, rot.yaw, rot.roll))?;
}
#[rebo::function(raw("Tas::press_platform_on_server"))]
fn press_platform_on_server(platform_id: u8) {
    send_to_server(vm, "press platform", Request::PressPlatform(platform_id))?;
}
#[rebo::function(raw("Tas::press_button_on_server"))]
fn press_button_on_server(button_id: u8) {
    send_to_server(vm, "press button", Request::PressButton(button_id))?;
}
#[rebo::function(raw("Tas::new_game_pressed"))]
fn new_game_pressed() {
    send_to_server(vm, "new game pressed", Request::NewGamePressed)?;
}

// ARCHIPELAGO

#[rebo::function(raw("Tas::archipelago_connect"))]
fn archipelago_connect(server_and_port: String, game: String, slot: String, password: Option<String>) {
    let state = STATE.lock().unwrap();
    let tx = &state.as_ref().unwrap().rebo_archipelago_tx;

    tx.send(ReboToArchipelago::Connect {
        server_and_port, game, slot, password,
        items_handling: ItemsHandlingFlags::OWN_WORLD | ItemsHandlingFlags::STARTING_INVENTORY,
        tags: vec![],
    }).unwrap();

    // Request item/location names from the server
    tx.send(ReboToArchipelago::ClientMessage(
        ClientMessage::GetDataPackage(GetDataPackage { games: None, })
    )).unwrap();
}
#[rebo::function(raw("Tas::archipelago_disconnect"))]
fn archipelago_disconnect() {
    STATE.lock().unwrap().as_ref().unwrap().rebo_archipelago_tx.send(ReboToArchipelago::Disconnect).unwrap();
}
fn archipelago_send_death() {
    STATE.lock().unwrap().as_ref().unwrap().rebo_archipelago_tx.send(ReboToArchipelago::SendDeath).unwrap();
}
#[rebo::function(raw("Tas::archipelago_send_check"))]
fn archipelago_send_check(location_id: i64) {
    // if location_id is already in STATE.checked_locations, do nothing
    log!("Archipelago: sending location check for {}", location_id);
    STATE.lock().unwrap().as_ref().unwrap().rebo_archipelago_tx
        .send(ReboToArchipelago::LocationChecks { locations: vec![location_id] })
        .unwrap();
}
#[rebo::function(raw("Tas::archipelago_goal"))]
fn archipelago_goal() {
    let msg = format!("Archipelago: goal!");
    log!("{}", msg);
    STATE.lock().unwrap().as_ref().unwrap().rebo_stream_tx.send(ReboToStream::Print(msg)).unwrap();
    STATE.lock().unwrap().as_ref().unwrap().rebo_archipelago_tx
        .send(ReboToArchipelago::Goal)
        .unwrap();
}


#[rebo::function("Tas::get_level")]
fn get_level() -> i32 {
    LevelState::get_level()
}
#[rebo::function("Tas::set_level")]
fn set_level_rebo(level: i32) {
    set_level(level);
}
fn set_level(level: i32) {
    LevelState::set_level(level);
}

static BUTTON_CACHE: Lazy<std::sync::Mutex<Option<Vec<usize>>>> = Lazy::new(|| std::sync::Mutex::new(None));

#[rebo::function("Tas::archipelago_gather_all_buttons")]
fn archipelago_gather_all_buttons() {
    log!("Archipelago: gathering all buttons");

    // If already gathered, do nothing.
    {
        let lock = BUTTON_CACHE.lock().unwrap();
        if lock.is_some() {
            log!("Archipelago: button cache already filled ({} buttons)", lock.as_ref().unwrap().len());
            return;
        }
    }

    let mut vec: Vec<usize> = Vec::new();
    UeScope::with(|scope| {
        for item in scope.iter_global_object_array() {
            let object = item.object();
            if object.is_null() {
                continue;
            }
            let class_name = object.class().name();
            let name = object.name();
            if class_name == "BP_Button_C" && name != "Default__BP_Button_C" {
                vec.push(object.as_ptr() as usize);
            }
        }
    });

    let mut cache_lock = BUTTON_CACHE.lock().unwrap();
    *cache_lock = Some(vec);
    log!("Archipelago: gathered {} buttons", cache_lock.as_ref().unwrap().len());
}

#[rebo::function("Tas::archipelago_activate_buttons_ap")]
fn archipelago_activate_buttons_ap() {
    archipelago_activate_buttons(-1);
}
#[rebo::function("Tas::archipelago_deactivate_buttons_ap")]
fn archipelago_deactivate_buttons_ap() {
    archipelago_deactivate_buttons(-1);
}

fn archipelago_activate_buttons(index: i32) {
    // ensure cache is filled
    {
        let lock = BUTTON_CACHE.lock().unwrap();
        if lock.is_none() {
            panic!("BUTTON_CACHE not initialized; call Tas::archipelago_gather_all_buttons first");
        }
    }

    log!("Archipelago: activating buttons (filter index={})", index);

    let cached = {
        let lock = BUTTON_CACHE.lock().unwrap();
        lock.as_ref().map(|v| v.clone()).unwrap_or_default()
    };

    UeScope::with(|_scope| {
        for ptr in cached {
            let object = unsafe { ObjectWrapper::new(ptr as *mut UObject) };
            if object.is_null() {
                continue;
            }
            let name = object.name();
            if index < 0 || name == format!("BP_Button_C_{}", index) {
                object.get_field("IsPressed").unwrap::<BoolValueWrapper>().set(false);
            }
        }
    });
}
fn archipelago_deactivate_buttons(index: i32) {
    // ensure cache is filled
    {
        let lock = BUTTON_CACHE.lock().unwrap();
        if lock.is_none() {
            panic!("BUTTON_CACHE not initialized; call Tas::archipelago_gather_all_buttons first");
        }
    }

    log!("Archipelago: deactivating buttons (filter index={})", index);

    let cached = {
        let lock = BUTTON_CACHE.lock().unwrap();
        lock.as_ref().map(|v| v.clone()).unwrap_or_default()
    };

    UeScope::with(|_scope| {
        for ptr in cached {
            let object = unsafe { ObjectWrapper::new(ptr as *mut UObject) };
            if object.is_null() {
                continue;
            }
            let name = object.name();
            if index < 0 || name == format!("BP_Button_C_{}", index) {
                object.get_field("IsPressed").unwrap::<BoolValueWrapper>().set(true);
            }
        }
    });
}


#[rebo::function("Tas::archipelago_raise_cluster")]
fn archipelago_raise_cluster_rebo(cluster_index: i32, last_unlocked: usize) {
    archipelago_raise_cluster(cluster_index, last_unlocked);
}

fn archipelago_raise_cluster(cluster_index: i32, last_unlocked: usize) {
    set_level(cluster_index);
    if last_unlocked == 6 {
        trigger_element_by_type(last_unlocked, "Button".to_string(), 1);
    }
    if last_unlocked == 9 {
        trigger_element_by_type(last_unlocked, "Button".to_string(), 1);
    }
    if last_unlocked == 17 {
        trigger_element_by_type(last_unlocked, "Button".to_string(), 1);
    }
    if last_unlocked == 25 {
        trigger_element_by_type(last_unlocked, "Button".to_string(), 1);
        trigger_element_by_type(last_unlocked, "Button".to_string(), 2);
    }
    if last_unlocked == 27 {
        trigger_element_by_type(last_unlocked, "Button".to_string(), 1);
    }
    trigger_element_by_type(last_unlocked, "Button".to_string(), 0);
}


#[rebo::function("Tas::trigger_element")]
fn trigger_element(index: ElementIndex) {
    fn add_remove_based_character(actor: &ActorWrapper<'_>) {
        let state = STATE.lock().unwrap();
        let state = state.as_ref().unwrap();
        unsafe {
            let liftbase = actor.as_ptr() as *mut ALiftBaseUE;
            let character = AMyCharacter::get_player().as_ptr();
            state.hooks.aliftbase.add_based_character(liftbase, character);
            state.hooks.aliftbase.remove_based_character(liftbase, character);
        }
    }
    fn collect_cube(actor: &ActorWrapper<'_>) {
        let trigger_fn = actor
            .class()
            .find_function("BndEvt__Trigger_K2Node_ComponentBoundEvent_24_ComponentBeginOverlapSignature__DelegateSignature")
            .unwrap();
        let params = trigger_fn.create_argument_struct();
        let movement = unsafe { ObjectWrapper::new(AMyCharacter::get_player().movement() as *mut UObject) };
        let updated_primitive = movement.get_field("UpdatedPrimitive").unwrap::<ObjectWrapper>();
        let trigger = actor.get_field("Trigger").unwrap::<ObjectWrapper>();
        params.get_field("OverlappedComponent").set_object(&updated_primitive);
        params.get_field("OtherComp").set_object(&trigger);
        params.get_field("OtherActor").set_object(&actor);
        params.get_field("OtherBodyIndex").unwrap::<&Cell<i32>>().set(0);
        unsafe {
            trigger_fn.call(actor.as_ptr(), &params);
        }
    }
    UeScope::with(|scope| {
        let levels = LEVELS.lock().unwrap();
        match index.element_type {
            ElementType::Platform => add_remove_based_character(&*scope.get(levels[index.cluster_index].platforms[index.element_index])),
            ElementType::Cube => collect_cube(&*scope.get(levels[index.cluster_index].cubes[index.element_index])),
            ElementType::Button => add_remove_based_character(&*scope.get(levels[index.cluster_index].buttons[index.element_index])),
            ElementType::Lift => add_remove_based_character(&*scope.get(levels[index.cluster_index].lifts[index.element_index])),
            ElementType::Pipe => (),
            ElementType::Springpad => (),
        }
    });
}

fn trigger_element_by_type(cluster_index: usize, element_type: String, element_index: usize) {
    if element_type == "Button" {
        archipelago_activate_buttons(-1);
    }
    trigger_element_by_type_details(cluster_index, element_type.clone(), element_index);
    if element_type == "Button" {
        archipelago_deactivate_buttons(-1);
    }
}

#[rebo::function("Tas::trigger_element_by_type")]
fn trigger_element_by_type_rebo(cluster_index: usize, element_type: String, element_index: usize) {
    trigger_element_by_type(cluster_index, element_type, element_index);
} 

fn trigger_element_by_type_details(cluster_index: usize, element_type: String, element_index: usize) {
    let element_type = element_type.as_str();
    fn add_remove_based_character(actor: &ActorWrapper<'_>) {
        let state = STATE.lock().unwrap();
        let state = state.as_ref().unwrap();
        unsafe {
            let liftbase = actor.as_ptr() as *mut ALiftBaseUE;
            let character = AMyCharacter::get_player().as_ptr();
            state.hooks.aliftbase.add_based_character(liftbase, character);
            state.hooks.aliftbase.remove_based_character(liftbase, character);
        }
    }
    fn collect_cube(actor: &ActorWrapper<'_>) {
        let trigger_fn = actor
            .class()
            .find_function("BndEvt__Trigger_K2Node_ComponentBoundEvent_24_ComponentBeginOverlapSignature__DelegateSignature")
            .unwrap();
        let params = trigger_fn.create_argument_struct();
        let movement = unsafe { ObjectWrapper::new(AMyCharacter::get_player().movement() as *mut UObject) };
        let updated_primitive = movement.get_field("UpdatedPrimitive").unwrap::<ObjectWrapper>();
        let trigger = actor.get_field("Trigger").unwrap::<ObjectWrapper>();
        params.get_field("OverlappedComponent").set_object(&updated_primitive);
        params.get_field("OtherComp").set_object(&trigger);
        params.get_field("OtherActor").set_object(&actor);
        params.get_field("OtherBodyIndex").unwrap::<&Cell<i32>>().set(0);
        unsafe {
            trigger_fn.call(actor.as_ptr(), &params);
        }
    }
    UeScope::with(|scope| {
        let levels = LEVELS.lock().unwrap();
        match element_type {
            "Platform" => add_remove_based_character(&*scope.get(levels[cluster_index].platforms[element_index])),
            "Cube" => collect_cube(&*scope.get(levels[cluster_index].cubes[element_index])),
            "Button" => add_remove_based_character(&*scope.get(levels[cluster_index].buttons[element_index])),
            "Lift" => add_remove_based_character(&*scope.get(levels[cluster_index].lifts[element_index])),
            "Pipe" => (),
            "Springpad" => (),
            _ => panic!("unknown element type: {}", element_type),
        }
    });
}
#[rebo::function("Tas::abilities_set_jump_pads")]
fn abilities_set_jump_pads(enabled: bool) {
    log!("Archipelago: setting jump pads to {}", enabled);
    unsafe {
        let character = ObjectWrapper::new(AMyCharacter::get_player().as_ptr() as *mut UObject);
        if !enabled {  // no jump pads
            character.get_field("LaunchVelocityZ").unwrap::<&Cell<f32>>().set(700.);
        } else {
            character.get_field("LaunchVelocityZ").unwrap::<&Cell<f32>>().set(1979.899);  // default value
        }
    }
}
#[rebo::function("Tas::abilities_set_wall_jump")]
fn abilities_set_wall_jump(wall_jump: i32, change_physics: bool) {
    // 0 is off, 1 is one, 2 is infinite
    // change_physics changes the wall jump angles to make wall jumping not replace ledge grab
    log!("Archipelago: setting wall jump to {}", wall_jump);
    log!("Archipelago: change_physics to {}", change_physics);
    unsafe {
        let character = ObjectWrapper::new(AMyCharacter::get_player().as_ptr() as *mut UObject);

        if wall_jump == 0 {  // no wall jump
            character.get_field("LastWallJumpTime").unwrap::<&Cell<f32>>().set(385738752.);
            character.get_field("MinWallJumpInterval").unwrap::<&Cell<f32>>().set(385738752.);
        } else if wall_jump == 1 {  // just one wall jump
            character.get_field("LastWallJumpTime").unwrap::<&Cell<f32>>().set(-385738752.);
            character.get_field("MinWallJumpInterval").unwrap::<&Cell<f32>>().set(1.);
        } else{  // infinite wall jumps
            character.get_field("LastWallJumpTime").unwrap::<&Cell<f32>>().set(-385738752.);
            character.get_field("MinWallJumpInterval").unwrap::<&Cell<f32>>().set(0.15);  // default value
        }

        if change_physics {
            character.get_field("MinForcedWallJumpAngle").unwrap::<&Cell<f32>>().set(-0.8);
            character.get_field("MinWallJumpAngleDiff").unwrap::<&Cell<f32>>().set(0.8);
            character.get_field("MaxForcedWallJumpAngle").unwrap::<&Cell<f32>>().set(0.8);
        } else {
            character.get_field("MinForcedWallJumpAngle").unwrap::<&Cell<f32>>().set(-0.5);
            character.get_field("MinWallJumpAngleDiff").unwrap::<&Cell<f32>>().set(0.5);
            character.get_field("MaxForcedWallJumpAngle").unwrap::<&Cell<f32>>().set(0.5);
        }
    }
}
#[rebo::function("Tas::abilities_set_ledge_grab")]
fn abilities_set_ledge_grab(ledge_grab: bool) {
    log!("Archipelago: setting ledge grab to {}", ledge_grab);
    unsafe {
        let character = ObjectWrapper::new(AMyCharacter::get_player().as_ptr() as *mut UObject);
        if !ledge_grab {  // no ledge grab
            character.get_field("MaxClimbWallAngle").unwrap::<&Cell<f32>>().set(-385738752.);
        } else {  // yes ledge grab
            character.get_field("MaxClimbWallAngle").unwrap::<&Cell<f32>>().set(-0.5);  // default value
        }
    }
}

#[rebo::function("Tas::archipelago_trigger_goal_animation")]
fn archipelago_trigger_goal_animation() {
    log!("Archipelago: triggering goal animation");
    UeScope::with(|scope| {
        let levels = LEVELS.lock().unwrap();
        let jump6 = scope.get(JUMP6_INDEX.get().unwrap());
        let foo = &*scope.get(levels[0].buttons[0]);
        log!("Archipelago: triggering goal animation on {:?} {:?}", jump6.name(), foo.name());
        let fun = jump6.class().find_function("ButtonPressed_Event").unwrap();
        let params = fun.create_argument_struct();
        params.get_field("Reference").set_object(foo.get_object_wrapper());
        unsafe {
            fun.call(jump6.as_ptr(), &params);
        }
    })
}
#[rebo::function("Tas::set_start_seconds")]
fn set_start_seconds(start_seconds: i32) {
    LevelState::set_start_seconds(start_seconds);
}
#[rebo::function("Tas::set_start_partial_seconds")]
fn set_start_partial_seconds(start_partial_seconds: f32) {
    LevelState::set_start_partial_seconds(start_partial_seconds);
}
#[rebo::function("Tas::set_end_seconds")]
fn set_end_seconds(end_seconds: i32) {
    LevelState::set_end_seconds(end_seconds);
}
#[rebo::function("Tas::set_end_partial_seconds")]
fn set_end_partial_seconds(end_partial_seconds: f32) {
    LevelState::set_end_partial_seconds(end_partial_seconds);
}
#[rebo::function("Tas::get_accurate_real_time")]
fn get_accurate_real_time() -> f64 {
    return UGameplayStatics::get_accurate_real_time();
}
#[rebo::function("Tas::is_windows")]
fn is_windows() -> bool {
    cfg!(windows)
}
#[rebo::function("Tas::is_linux")]
fn is_linux() -> bool {
    !cfg!(windows)
}
#[rebo::function("Tas::get_clipboard")]
fn get_clipboard() -> String {
    Clipboard::get().unwrap_or_default()
}
#[rebo::function("Tas::set_clipboard")]
fn set_clipboard(content: String) {
    Clipboard::set(content)
}
#[rebo::function("Tas::show_hud")]
fn show_hud() {
    AMyHud::show_hud();
}

#[rebo::function("Tas::set_all_cluster_speeds")]
fn set_all_cluster_speeds(speed: f32) {
    // initialize before we change anything
    let _ = &*ORIGINAL_MAP;
    UeScope::with(|scope| {
        for level in &*LEVELS.lock().unwrap() {
            scope.get(level.level).set_speed(speed);
        }
    })
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct RefunctMapV0 {
    clusters: Vec<ClusterV0>,
}
#[derive(Debug, Clone, Serialize, Deserialize)]
struct ClusterV0 {
    platforms: Vec<ElementV0>,
    cubes: Vec<ElementV0>,
    buttons: Vec<ElementV0>,
}
#[derive(Debug, Clone, Serialize, Deserialize)]
struct ElementV0 {
    x: f32,
    y: f32,
    z: f32,
    pitch: f32,
    yaw: f32,
    roll: f32,
    xscale: f32,
    yscale: f32,
    zscale: f32,
}

// Changes since the last released map version:
// - Cluster 25 pipe -> Cluster 24 pipe
// - Cluster 25 springpad -> Cluster 24 springpad
// - Cluster 24 springpad -> Cluster 25 springpad
// - Cluster 26 springpad -> Cluster 25 springpad

fn migrate_v0_to_v1(map: RefunctMapV0) -> RefunctMap {
    fn migrate_element(orig: &RefunctMap, e: ElementV0, index: ElementIndex) -> Element {
        let orig = get_indexed_element(orig, index);
        Element {
            x: e.x,
            y: e.y,
            z: e.z,
            pitch: e.pitch,
            yaw: e.yaw,
            roll: e.roll,
            sizex: e.xscale * orig.sizex,
            sizey: e.yscale * orig.sizey,
            sizez: e.zscale * orig.sizez,
        }
    }
    let orig = &*ORIGINAL_MAP;
    RefunctMap {
        version: 1,
        clusters: map.clusters.into_iter().enumerate().map(|(cluster_index, cluster)| Cluster {
            z: orig.clusters[cluster_index].z,
            rise_speed: orig.clusters[cluster_index].rise_speed,
            platforms: cluster.platforms.into_iter().enumerate().map(|(element_index, e)| migrate_element(&orig, e, ElementIndex { cluster_index, element_type: ElementType::Platform, element_index })).collect(),
            cubes: cluster.cubes.into_iter().enumerate().map(| (element_index, e)| migrate_element(&orig, e, ElementIndex { cluster_index, element_type: ElementType::Cube, element_index })).collect(),
            buttons: cluster.buttons.into_iter().enumerate().map(| (element_index, e)| migrate_element(&orig, e, ElementIndex { cluster_index, element_type: ElementType::Button, element_index })).collect(),
            lifts: orig.clusters[cluster_index].lifts.clone(),
            pipes: orig.clusters[cluster_index].pipes.clone(),
            springpads: orig.clusters[cluster_index].springpads.clone(),
        }).collect(),
    }
}

#[derive(Debug, Clone, Serialize, Deserialize, rebo::ExternalType)]
pub struct RefunctMap {
    version: u32,
    clusters: Vec<Cluster>,
}
#[derive(Debug, Clone, Serialize, Deserialize, rebo::ExternalType)]
struct Cluster {
    z: f32,
    rise_speed: f32,
    platforms: Vec<Element>,
    cubes: Vec<Element>,
    buttons: Vec<Element>,
    lifts: Vec<Element>,
    pipes: Vec<Element>,
    springpads: Vec<Element>,
}
#[derive(Debug, Clone, Serialize, Deserialize, rebo::ExternalType)]
struct Element {
    x: f32,
    y: f32,
    z: f32,
    pitch: f32,
    yaw: f32,
    roll: f32,
    sizex: f32,
    sizey: f32,
    sizez: f32,
}

fn map_path() -> PathBuf {
    let appdata_path = data_path();
    let map_path = appdata_path.join("maps/");
    if !map_path.is_dir() {
        std::fs::create_dir(&map_path).unwrap();
    }
    map_path
}
#[rebo::function("Tas::list_maps")]
fn list_maps() -> Vec<String> {
    let path = map_path();
    std::fs::read_dir(path).unwrap().flatten()
        .map(|entry| {
            assert!(entry.file_type().unwrap().is_file());
            entry.file_name().into_string().unwrap()
        }).collect()
}
#[rebo::function("Tas::load_map")]
fn load_map(filename: String) -> RefunctMap {
    #[derive(Deserialize)]
    struct Version {
        #[serde(default)]
        version: u32,
    }
    let filename = sanitize_filename::sanitize(filename);
    let path = map_path().join(filename);
    let content = std::fs::read_to_string(path).unwrap();
    let version: Version = serde_json::from_str(&content).unwrap();
    let map = match version.version {
        0 => {
            let map = serde_json::from_str(&content).unwrap();
            migrate_v0_to_v1(map)
        }
        1 => serde_json::from_str(&content).unwrap(),
        version => panic!("the map lives in the future (unknown map version {version})"),
    };
    map
}
#[rebo::function("Tas::save_map")]
fn save_map(filename: String, map: RefunctMap) {
    let filename = sanitize_filename::sanitize(filename);
    let path = map_path().join(filename);
    let file = File::create(path).unwrap();
    serde_json::to_writer_pretty(file, &map).unwrap();
}
#[rebo::function("Tas::remove_map")]
fn remove_map(filename: String) -> bool {
    let filename = sanitize_filename::sanitize(filename);
    let path = map_path().join(filename);
    std::fs::remove_file(path).is_ok()
}

fn get_current_map(original: bool) -> RefunctMap {
    UeScope::with(|scope| {
        fn actor_list_to_element_list<'a, T, F>(scope: &'a UeScope, level: &LevelWrapper<'a>, list: &[ObjectIndex<T>], element_type: ElementType, get_orig_size: F) -> Vec<Element>
        where
            T: UeObjectWrapperType,
            T::UeObjectWrapper<'a>: Deref<Target = ActorWrapper<'a>>,
            F: Fn(&ActorWrapper<'a>, ElementIndex) -> (f32, f32, f32),
        {
            list.iter().enumerate().map(|(element_index, actor)| {
                let actor = scope.get(actor);
                let actor = actor.deref();
                let index = ElementIndex { cluster_index: level.level_index(), element_type, element_index };
                let (sizex, sizey, sizez) = get_orig_size(actor, index);
                let (_, _, lz) = level.relative_location();
                let (ax, ay, az) = actor.absolute_location();
                let (pitch, yaw, roll) = actor.relative_rotation();
                let (xscale, yscale, zscale) = actor.relative_scale();
                Element { x: ax, y: ay, z: az - lz, pitch, yaw, roll, sizex: sizex / xscale, sizey: sizey / yscale, sizez: sizez / zscale }
            }).collect()
        }
        let get_orig_size: Box<for<'a> fn(&'a ActorWrapper, _) -> _> = if original {
            Box::new(|actor: &ActorWrapper, _index: ElementIndex| {
                let (_, _, _, hx, hy, hz) = actor.get_actor_bounds();
                // ugly hack for rotated platforms to switch sizex and sizey
                // if they are rotated by 90° or -90°
                let (pitch, yaw, roll) = actor.relative_rotation();
                let (hx, hy) = if (89. < yaw && yaw <= 91.) || (-91. < yaw && yaw < -89.) {
                    (hy, hx)
                } else {
                    (hx, hy)
                };
                // it's never rotated exactly 90° / 180° / -180° / -90° but slightly off
                // like -89.99963 or -179.99976 or 179.99976 or even 0.0033555343
                // we correct that by flooring in those cases
                let (hx, hy) = if pitch.fract() != 0. || yaw.fract() != 0. || roll.fract() != 0. {
                    ((hx * 4.).floor() / 4., (hy * 4.).floor() / 4.)
                } else {
                    (hx, hy)
                };
                (hx*2., hy*2., hz*2.)
            })
        } else {
            Box::new(move |_actor: &ActorWrapper, index: ElementIndex| {
                let e = get_indexed_element(&*ORIGINAL_MAP, index);
                (e.sizex, e.sizey, e.sizez)
            })
        };
        let levels = LEVELS.lock().unwrap();
        let clusters: Vec<Cluster> = levels.iter()
            .map(|level| {
                let level_wrapper = scope.get(level.level);
                Cluster {
                    z: level_wrapper.source_location().2,
                    rise_speed: level_wrapper.speed(),
                    platforms: actor_list_to_element_list(scope, &level_wrapper, &level.platforms, ElementType::Platform, &get_orig_size),
                    cubes: actor_list_to_element_list(scope, &level_wrapper, &level.cubes, ElementType::Cube, &get_orig_size),
                    buttons: actor_list_to_element_list(scope, &level_wrapper, &level.buttons, ElementType::Button, &get_orig_size),
                    lifts: actor_list_to_element_list(scope, &level_wrapper, &level.lifts, ElementType::Lift, &get_orig_size),
                    pipes: actor_list_to_element_list(scope, &level_wrapper, &level.pipes, ElementType::Pipe, &get_orig_size),
                    springpads: actor_list_to_element_list(scope, &level_wrapper, &level.springpads, ElementType::Springpad, &get_orig_size),
                }
            }).collect();
        RefunctMap { version: 1, clusters }
    })
}

#[rebo::function("Tas::current_map")]
fn current_map() -> RefunctMap {
    // initialize original map
    let _ = &*ORIGINAL_MAP;
    get_current_map(false)
}
pub static ORIGINAL_MAP: Lazy<RefunctMap> = Lazy::new(|| get_current_map(true));
#[rebo::function("Tas::original_map")]
fn original_map() -> RefunctMap {
    ORIGINAL_MAP.clone()
}
pub fn apply_map_internal(map: &RefunctMap) {
    // initialize before we change anything
    let _ = &*ORIGINAL_MAP;

    fn set_element(scope: &UeScope, levels: &[Level], target_map: &RefunctMap, index: ElementIndex) {
        let level = scope.get(levels[index.cluster_index].level);
        let actor = get_indexed_actor(scope, levels, index);
        let target = get_indexed_element(target_map, index);
        let orig = get_indexed_element(&*ORIGINAL_MAP, index);
        let (_, _, rz) = level.relative_location();
        let (rpitch, ryaw, rroll) = level.relative_rotation();
        let target_location = FVector { x: target.x, y: target.y, z: target.z + rz };
        let target_rotation = FRotator { pitch: target.pitch + rpitch, yaw: target.yaw + ryaw, roll: target.roll + rroll };
        let target_scale = FVector { x: target.sizex / orig.sizex, y: target.sizey / orig.sizey, z: target.sizez / orig.sizez };
        USceneComponent::set_world_location_and_rotation(target_location, target_rotation, &actor);
        USceneComponent::set_world_scale(target_scale, &actor);
    }

    UeScope::with(|scope| {
        let levels = LEVELS.lock().unwrap();
        assert_eq!(map.clusters.len(), levels.len());
        for (cluster_index, (level, cluster)) in levels.iter().zip(&map.clusters).enumerate() {
            let level_wrapper = scope.get(level.level);
            let (rx, ry, _) = level_wrapper.source_location();
            level_wrapper.set_source_location(rx, ry, cluster.z);
            level_wrapper.set_speed(cluster.rise_speed);
            level.platforms.iter().zip(&cluster.platforms).enumerate().map(|i| (i.0, ElementType::Platform))
                .chain(level.cubes.iter().zip(&cluster.cubes).enumerate().map(|i| (i.0, ElementType::Cube)))
                .chain(level.buttons.iter().zip(&cluster.buttons).enumerate().map(|i| (i.0, ElementType::Button)))
                .chain(level.lifts.iter().zip(&cluster.lifts).enumerate().map(|i| (i.0, ElementType::Lift)))
                .chain(level.pipes.iter().zip(&cluster.pipes).enumerate().map(|i| (i.0, ElementType::Pipe)))
                .chain(level.springpads.iter().zip(&cluster.springpads).enumerate().map(|i| (i.0, ElementType::Springpad)))
                .for_each(|(element_index, element_type)| {
                    let index = ElementIndex { cluster_index, element_type, element_index };
                    set_element(scope, &levels, &map, index);
                });
        }
    })
}
#[rebo::function("Tas::apply_map")]
fn apply_map(map: RefunctMap) {
    apply_map_internal(&map)
}
#[rebo::function("Tas::apply_map_cluster_speeds")]
fn apply_map_cluster_speeds(map: RefunctMap) {
    UeScope::with(|scope| {
        let levels = LEVELS.lock().unwrap();
        assert_eq!(map.clusters.len(), levels.len());
        for (_cluster_index, (level, cluster)) in levels.iter().zip(&map.clusters).enumerate() {
            let level_wrapper = scope.get(level.level);
            level_wrapper.set_speed(cluster.rise_speed);
        }
    })
}
#[rebo::function("Tas::get_looked_at_element_index")]
fn get_looked_at_element_index() -> Option<ElementIndex> {
    let intersected = KismetSystemLibrary::line_trace_single(AMyCharacter::get_player());
    try_find_element_index(intersected as *mut UObject)
}

fn get_indexed_element(map: &RefunctMap, index: ElementIndex) -> Element {
    let level = &map.clusters[index.cluster_index];
    match index.element_type {
        ElementType::Platform => level.platforms[index.element_index].clone(),
        ElementType::Cube => level.cubes[index.element_index].clone(),
        ElementType::Button => level.buttons[index.element_index].clone(),
        ElementType::Lift => level.lifts[index.element_index].clone(),
        ElementType::Pipe => level.pipes[index.element_index].clone(),
        ElementType::Springpad => level.springpads[index.element_index].clone(),
    }
}
fn get_indexed_actor<'a>(scope: &'a UeScope, levels: &[Level], index: ElementIndex) -> ActorWrapper<'a> {
    let level = &levels[index.cluster_index];
    match index.element_type {
        ElementType::Platform => (*scope.get(level.platforms[index.element_index])).clone(),
        ElementType::Cube => (*scope.get(level.cubes[index.element_index])).clone(),
        ElementType::Button => (*scope.get(level.buttons[index.element_index])).clone(),
        ElementType::Lift => (*scope.get(level.lifts[index.element_index])).clone(),
        ElementType::Pipe => (*scope.get(level.pipes[index.element_index])).clone(),
        ElementType::Springpad => (*scope.get(level.springpads[index.element_index])).clone(),
    }
}

#[derive(Debug, Clone, rebo::ExternalType)]
struct Bounds {
    originx: f32,
    originy: f32,
    originz: f32,
    extentx: f32,
    extenty: f32,
    extentz: f32,
}

#[rebo::function("Tas::get_element_bounds")]
fn get_element_bounds(index: ElementIndex) -> Bounds {
    UeScope::with(|scope| {
        let levels = LEVELS.lock().unwrap();
        let actor = get_indexed_actor(scope, &levels, index);
        let (originx, originy, originz, extentx, extenty, extentz) = actor.get_actor_bounds();
        Bounds { originx, originy, originz, extentx, extenty, extentz }
    })
}

#[rebo::function("Tas::enable_player_collision")]
fn enable_player_collision() {
    AActor::set_actor_enable_collision(AMyCharacter::get_player().as_ptr() as *const AActor, true);
}

#[rebo::function("Tas::disable_player_collision")]
fn disable_player_collision() {
    AActor::set_actor_enable_collision(AMyCharacter::get_player().as_ptr() as *const AActor, false);
}

#[rebo::function("Tas::exit_water")]
fn exit_water() {
    AMyCharacter::exit_water();
}
#[rebo::function("Tas::respawn")]
fn respawn() {
    AMyCharacter::respawn();
}
#[rebo::function("Tas::is_death_link_on")]
fn is_death_link_on() -> bool {
    AMyCharacter::has_death_hook()
}
#[rebo::function("Tas::set_death_link")]
fn set_death_link(on: bool) {
    let tags: Vec<String>;
    if on {
        tags = vec![String::from("DeathLink")];
        AMyCharacter::set_death_hook(archipelago_send_death as fn());
    } else {
        tags = vec![];
        AMyCharacter::unset_death_hook();
    }

    let state = STATE.lock().unwrap();
    let tx = &state.as_ref().unwrap().rebo_archipelago_tx;

    tx.send(ReboToArchipelago::ConnectUpdate {
        items_handling: ItemsHandlingFlags::OWN_WORLD | ItemsHandlingFlags::STARTING_INVENTORY,
        tags,
    }).unwrap();
}
#[rebo::function("Tas::open_maps_folder")]
fn open_maps_folder() {
    if let Err(err) = opener::open(&map_path()) {
        log!("Error opening maps folder in file manager: {}", err);
    }
}
#[rebo::function("Tas::open_recordings_folder")]
fn open_recordings_folder() {
    if let Err(err) = opener::open(&recording_path()) {
        log!("Error opening recordings folder in file manager: {}", err);
    }
}
#[rebo::function("Tas::set_lighting_casts_shadows")]
fn set_lighting_casts_shadows(value: bool, def: bool) {
    UWorld::set_lighting_casts_shadows(value);
    if value == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_lighting_casts_shadows(def);
    });
}
#[rebo::function("Tas::set_sky_light_enabled")]
fn set_sky_light_enabled(enabled: bool, def: bool) {
    UWorld::set_sky_light_enabled(enabled);
    if enabled == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_sky_light_enabled(def);
    });
}
#[rebo::function("Tas::set_time_dilation")]
fn set_time_dilation(dilation: f32, def: f32) {
    UWorld::set_time_dilation(dilation);
    if dilation == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(30));
        UWorld::set_time_dilation(def);
    });
}
#[rebo::function("Tas::set_gravity")]
fn set_gravity(gravity: f32) {
    UWorld::set_gravity(gravity);
}
#[rebo::function("Tas::get_time_of_day")]
fn get_time_of_day() -> f32 {
    UWorld::get_time_of_day()
}
#[rebo::function("Tas::set_time_of_day")]
fn set_time_of_day(time: f32) {
    UWorld::set_time_of_day(time);
}
#[rebo::function("Tas::set_sky_time_speed")]
fn set_sky_time_speed(speed: f32, def: f32) {
    UWorld::set_sky_time_speed(speed);
    if speed == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_sky_time_speed(def);
    });
}
#[rebo::function("Tas::set_sky_light_brightness")]
fn set_sky_light_brightness(brightness: f32) {
    UWorld::set_sky_light_brightness(brightness);
}
#[rebo::function("Tas::set_sky_light_intensity")]
fn set_sky_light_intensity(intensity: f32) {
    UWorld::set_sky_light_intensity(intensity);
}
#[rebo::function("Tas::set_stars_brightness")]
fn set_stars_brightness(brightness: f32, def: f32) {
    UWorld::set_stars_brightness(brightness);
    if brightness == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_stars_brightness(def);
    });
}
#[rebo::function("Tas::set_fog_enabled")]
fn set_fog_enabled(enabled: bool, def: bool) {
    UWorld::set_fog_enabled(enabled);
    if enabled == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_fog_enabled(def);
    });
}
#[rebo::function("Tas::set_sun_redness")]
fn set_sun_redness(redness: f32, def: f32) {
    UWorld::set_sun_redness(redness);
    if redness == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_sun_redness(def);
    });
}
#[rebo::function("Tas::set_cloud_redness")]
fn set_cloud_redness(red: f32, def: f32) {
    UWorld::set_cloud_redness(red);
    if red == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_cloud_redness(def);
    });
}
#[rebo::function("Tas::set_reflection_render_scale")]
fn set_reflection_render_scale(render_scale: i32) {
    UWorld::set_reflection_render_scale(render_scale);
}
#[rebo::function("Tas::set_cloud_speed")]
fn set_cloud_speed(speed: f32, def: f32) {
    UWorld::set_cloud_speed(speed);
    if speed == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(60));
        UWorld::set_cloud_speed(def);
    });
}
#[rebo::function("Tas::set_outro_time_dilation")]
fn set_outro_time_dilation(dilation: f32) {
    UWorld::set_outro_time_dilation(dilation);
}
#[rebo::function("Tas::set_outro_dilated_duration")]
fn set_outro_dilated_duration(duration: f32) {
    UWorld::set_outro_dilated_duration(duration);
}
#[rebo::function("Tas::set_kill_z")]
fn set_kill_z(kill_z: f32) {
    UWorld::set_kill_z(kill_z);
}
#[rebo::function("Tas::set_gamma")]
fn set_gamma(value: f32) {
    UEngine::set_gamma(value);
}
#[rebo::function("Tas::set_screen_percentage")]
fn set_screen_percentage(percentage: f32, def: f32) {
    UWorld::set_screen_percentage(percentage);
    if percentage == def {
        return;
    }
    std::thread::spawn(move || {
        std::thread::sleep(Duration::from_secs(30));
        UWorld::set_screen_percentage(def);
    });
}
#[rebo::function("Tas::set_reticle_width")]
fn set_reticle_width(width: f32) {
    AMyHud::set_reticle_width(width);
}
#[rebo::function("Tas::set_reticle_height")]
fn set_reticle_height(height: f32) {
    AMyHud::set_reticle_height(height);
}
#[rebo::function("Tas::get_camera_mode")]
fn get_camera_mode() -> u8 {
    AMyCharacter::camera_mode()
}
#[rebo::function("Tas::set_camera_mode")]
fn set_camera_mode(mode: u8) {
    AMyCharacter::set_camera_mode(mode);
}
#[rebo::function("Tas::set_game_rendering_enabled")]
fn set_game_rendering_enabled(enable: bool) {
    FViewport::set_game_rendering_enabled(enable);
}
#[rebo::function("Tas::set_input_mode_game_only")]
fn set_input_mode_game_only() {
    UWidgetBlueprintLibrary::set_input_mode_game_only();
}
#[rebo::function("Tas::set_input_mode_ui_only")]
fn set_input_mode_ui_only() {
    UWidgetBlueprintLibrary::set_input_mode_ui_only();
}
#[rebo::function("Tas::flush_pressed_keys")]
fn flush_pressed_keys() {
    AMyCharacter::flush_pressed_keys();
}
