use std::time::SystemTime;
use std::error::Error;
use std::thread;
use std::sync::atomic::{AtomicPtr, Ordering};
use archipelago_rs::client::{ArchipelagoClient, ArchipelagoClientReceiver, ArchipelagoClientSender, ArchipelagoError};
use archipelago_rs::protocol::{ClientStatus, ServerMessage, Bounce, BounceData, ClientMessage, DeathLink, ConnectUpdate}; // adjust path if needed
use crossbeam_channel::Sender;
use tokio::sync::mpsc::UnboundedReceiver;
use tokio::task::AbortHandle;
use rand::seq::IndexedRandom;
use crate::threads::{ArchipelagoToRebo, ReboToArchipelago};
use crate::native::{BoolValueWrapper};

use crate::native::UeScope;

static mut CURRENT_SLOT: Option<String> = None;

pub fn run(archipelago_rebo_tx: Sender<ArchipelagoToRebo>, mut rebo_archipelago_rx: UnboundedReceiver<ReboToArchipelago>) {
    thread::spawn(move || {
        loop {
            let future = async {
                let mut sender: Option<ArchipelagoClientSender> = None;
                let mut receiver_abort_handle: Option<AbortHandle> = None;
                loop {
                    match rebo_archipelago_rx.recv().await.unwrap() {
                        ReboToArchipelago::Connect { server_and_port, game, slot, password, items_handling, tags } => {
                            unsafe { CURRENT_SLOT = Some(slot.clone()); }
                            if let Some(receiver) = receiver_abort_handle {
                                receiver.abort();
                            }
                            let mut client = ArchipelagoClient::new(&server_and_port).await?;
                            log!("Connected to archipelago server `{server_and_port}`");
                            let connected_info = client.connect(&game, &slot, password.as_deref(), items_handling, tags)
                                .await?;
                            log!("Connected info: {:?}", connected_info);
                            archipelago_rebo_tx.send(ArchipelagoToRebo::ServerMessage(ServerMessage::Connected(connected_info))).unwrap();
                            let (s, mut receiver) = client.split();
                            sender = Some(s);
                            let join_handle = tokio::spawn(handle_receiver(receiver, archipelago_rebo_tx.clone()));
                            receiver_abort_handle = Some(join_handle.abort_handle());

                        },
                        ReboToArchipelago::ConnectUpdate { items_handling, tags } => {
                            if let Some(sender) = sender.as_mut() {
                                sender.send(ClientMessage::ConnectUpdate(ConnectUpdate {
                                    items_handling: items_handling.bits(),
                                    tags
                                })).await?;
                            }
                        }
                        ReboToArchipelago::ClientMessage(msg) => {
                            if let Some(sender) = sender.as_mut() {
                                sender.send(msg).await?;
                            } else {
                                // Handle the case where sender is None, e.g., log an error or return an error.
                                log!("Sender is None, cannot send message");
                                // Or return an error if appropriate:
                                // return Err(MyError::SenderNotAvailable);
                            }  
                        },
                        ReboToArchipelago::SendDeath => {
                            let slot: String;
                            unsafe {
                                match &CURRENT_SLOT {
                                    None => slot = String::from("null"),
                                    Some(s) => slot = s.clone()
                                }
                            }

                            let msgs = vec![
                                format!("{} can't swim", slot),
                                format!("{} drowned", slot),
                                format!("{} got their feet wet", slot),
                                format!("{} sank like a rock", slot),
                                format!("{} needs a towel", slot),
                                format!("{} used splash", slot),
                                format!("{} can't breathe under water", slot),
                            ];

                            if let Some(sender) = sender.as_mut() {
                                sender.send(ClientMessage::Bounce(Bounce {
                                    games: None,
                                    slots: None,
                                    tags: vec![String::from("DeathLink")],
                                    data: BounceData::DeathLink(DeathLink {
                                        time: SystemTime::now(),
                                        source: slot.clone(),
                                        cause: msgs.choose(&mut rand::rng()).map(|s|{s.clone()}),
                                    })
                                })).await?;
                            } else {
                                log!("Sender is None, cannot send message");
                            }
                        },
                        ReboToArchipelago::Disconnect => {
                            drop(sender.take());
                            if let Some(abort_handle) = receiver_abort_handle.take() {
                                abort_handle.abort();
                            }
                        },
                        ReboToArchipelago::LocationChecks { locations    } => {
                            log!("Sending location checks: {:?}", locations);
                            sender.as_mut().unwrap()
                                .location_checks(locations).await?;
                        },
                        ReboToArchipelago::Goal => {
                            log!("Sending status update: ClientGoal");
                            sender.as_mut().unwrap()
                                .status_update(ClientStatus::ClientGoal).await?;
                        }
                    }
                }
                Ok::<(), Box<dyn Error>>(())
            };

            let res = tokio::runtime::Builder::new_current_thread()
                .enable_all()
                .build()
                .unwrap()
                .block_on(future);
            archipelago_rebo_tx.send(ArchipelagoToRebo::ConnectionAborted).unwrap();
            log!("Archipelago thread finished with {res:?}, restarting...");
        }
    });
}

async fn handle_receiver(mut receiver: ArchipelagoClientReceiver, archipelago_rebo_tx: Sender<ArchipelagoToRebo>) {
    loop {
        match receiver.recv().await {
            Ok(None) => {
                continue;
            }
            Err(e) => {
                archipelago_rebo_tx.send(ArchipelagoToRebo::ConnectionAborted).unwrap();
                break;
            }
            Ok(Some(msg)) => {
                archipelago_rebo_tx.send(ArchipelagoToRebo::ServerMessage(msg)).unwrap();
            }
        }
    }
}
