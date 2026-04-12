#[cfg(all(target_os = "linux", not(target_pointer_width = "64")))]
compile_error!("must be compiled as 64bit on Linux (e.g. with `--target x86_64-unknown-linux-gnu`");
#[cfg(all(target_os = "windows", not(target_pointer_width = "32")))]
compile_error!("must be compiled as 32bit on Windows (e.g. with `--target i686-pc-windows-msvc`)");
#[cfg(not(any(target_os = "linux", target_os = "windows")))]
compile_error!("currently only 64bit linux and 32bit windows are supported");
//#[cfg(all(target_os = "macos", not(target_pointer_width = "64")))]
//compile_error!("must be compiled as 64bit on macOS (e.g. with `--target x86_64-apple-darwin`");

use std::sync::Once;
use std::thread;
use std::panic;

mod error;
#[macro_use] mod log;
mod native;
mod threads;
mod semaphore;
mod watch;

#[cfg(unix)] pub use crate::native::INITIALIZE_CTOR;
#[cfg(windows)] pub use native::DllMain;

static INIT: Once = Once::new();

pub extern "C" fn initialize() {
    INIT.call_once(|| {
        panic::set_hook(Box::new(|info| {
            let msg = match info.payload().downcast_ref::<&'static str>() {
                Some(s) => *s,
                None => match info.payload().downcast_ref::<String>() {
                    Some(s) => &s[..],
                    None => "Box<Any>",
                }
            };
            let thread = thread::current();
            let name = thread.name().unwrap_or("<unnamed>");
            log!("thread '{}' panicked with '{}' at {:?}\nBacktrace: {:?}", name, msg, info.location(), backtrace::Backtrace::new());
        }));
        env_logger::init();
        log!("initialize");
        let exe = ::std::env::current_exe().unwrap();
        log!("got exe: {:?}", exe);
        let file = exe.file_name().unwrap();
        log!("got exe file name: {:?}", file);
        if cfg!(unix) && file == "Refunct-Linux-Shipping"
            || cfg!(windows) && file == "Refunct-Win32-Shipping.exe"
        {
            log!("Platform: unix={} windows={}", cfg!(unix), cfg!(windows));

            thread::spawn(|| {
                log!("Thread spawned → Starting initialize");

                // on Linux we need to wait for the packer to finish
                if cfg!(unix) {
                    log!("Unix detected → sleeping before init...");
                    ::std::thread::sleep(::std::time::Duration::from_secs(7));
                    log!("Woke up from sleep");
                }

                // =====================
                // SUSPEND THREADS
                // =====================
                #[cfg(windows)]
                log!("About to suspend threads");

                #[cfg(windows)]
                let handles = {
                    let h = native::suspend_threads();
                    log!("Suspended threads successfully");
                    h
                };

                // =====================
                // INIT (YOUR CRASH ZONE)
                // =====================
                log!("Calling native::init()");
                let hooks = {
                    let h = native::init();
                    log!("native::init() returned");
                    h
                };

                // =====================
                // RESUME THREADS
                // =====================
                #[cfg(windows)]
                {
                    log!("Resuming threads...");
                    native::resume_threads(handles);
                    log!("Threads resumed");
                }

                // =====================
                // START THREADS
                // =====================
                log!("Calling threads::start()");
                threads::start(hooks);
                log!("threads::start() returned (this may never happen if it loops)");
            });
        }
    });
}
