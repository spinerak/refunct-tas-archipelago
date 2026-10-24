use std::process::Command;
use std::path::Path;
use std::ffi::CString;
use std::ptr::null_mut;
use std::mem;

use winapi::um::processthreadsapi::{CreateRemoteThread, OpenProcess};
use winapi::um::libloaderapi::{GetProcAddress, GetModuleHandleA};
use winapi::um::memoryapi::{VirtualAllocEx, WriteProcessMemory};
use winapi::um::handleapi::CloseHandle;
use winapi::um::winnt::{PROCESS_ALL_ACCESS, MEM_RESERVE, MEM_COMMIT, PAGE_READWRITE};

// http://resources.infosecinstitute.com/using-createremotethread-for-dll-injection-on-windows/
pub fn inject() {
    unsafe {
        use std::os::windows::ffi::OsStrExt;

        let rtil = ::std::env::var("RTIL").unwrap_or("./rtil.dll".to_string());
        let mut path = Path::new(&rtil);
        if !path.is_file() {
            path = Path::new("../rtil/target/i686-pc-windows-msvc/release/rtil.dll");
        }

        let absolute = path.canonicalize().unwrap();
        println!("Trying to inject {}", absolute.display());

        // Convert the path to null-terminated UTF-16
        let lib: Vec<u16> = absolute.as_os_str()
            .encode_wide()
            .chain(std::iter::once(0))
            .collect();

        let lib_size = lib.len() * std::mem::size_of::<u16>();

        let pid = pidof();
        let handle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
        if handle.is_null() {
            panic!("Could not open process: {}", std::io::Error::last_os_error());
        }

        let kernel = CString::new("kernel32.dll").unwrap();
        let load = CString::new("LoadLibraryW").unwrap();

        let addr = GetProcAddress(
            GetModuleHandleA(kernel.as_ptr()),
            load.as_ptr()
        );

        if addr.is_null() {
            panic!("Cannot find LoadLibraryW in kernel32.dll");
        }

        println!("Found LoadLibraryW at {:?}", addr);

        let arg = VirtualAllocEx(
            handle,
            null_mut(),
            lib_size,
            MEM_RESERVE | MEM_COMMIT,
            PAGE_READWRITE
        );

        if arg.is_null() {
            panic!("Cannot allocate memory in Refunct: {}", std::io::Error::last_os_error());
        }

        println!("Alloc'ed at {:?}", arg);

        let n = WriteProcessMemory(
            handle,
            arg,
            lib.as_ptr() as *const _,
            lib_size,
            null_mut()
        );

        if n == 0 {
            panic!("Cannot write to Refunct: {}", std::io::Error::last_os_error());
        }

        let thread_id = CreateRemoteThread(
            handle,
            null_mut(),
            0,
            Some(mem::transmute(addr)),
            arg,
            0,
            null_mut()
        );

        if thread_id.is_null() {
            panic!("Could not start remote thread: {}", std::io::Error::last_os_error());
        }

        println!("thread_id: {:?}", thread_id);

        CloseHandle(thread_id);
        CloseHandle(handle);
    }
}

fn pidof() -> u32 {
    let output = Command::new("powershell")
        .args(&[
            "-NoProfile",
            "-Command",
            "Get-Process -Name 'Refunct-Win32-Shipping' -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Id"
        ])
        .output()
        .expect("Cannot get pid of Refunct");

    let s = String::from_utf8(output.stdout)
        .expect("Output of pidof is not utf8");

    // this constant needs to be rechecked and possibly updated if there is a new refunct build
    const KNOWN_SIZE: u64 = 34400;

    let candidates = s.trim()
        .split("\n")
        .map(|id| id.trim().parse::<u32>())
        .filter(|res| res.is_ok())
        .map(|res| res.unwrap())
        .filter(|id| {
            // use powershell magic to determine the memory size of the binary from which the process started
            // while not impossible this should be relatively unlikely to accidentally match anything else sharing the process name
            println!("checking if pid {} could be refunct", id);
            let res = Command::new("powershell")
                .args(&[
                    "-NoProfile",
                    "-Command",
                    format!(
                        "Get-Process -Id {} -Module | where-object {{ $_.ModuleName -eq 'Refunct-Win32-Shipping.exe' }} | Select-Object -ExpandProperty Size",
                        id
                    ).as_str()
                ])
                .output();
            match res {
                Err(_) => false,
                Ok(res_output) => {
                    match String::from_utf8(res_output.stdout) {
                        Err(_) => false,
                        Ok(size_str) => {
                            match size_str.trim().parse::<u64>() {
                                Err(_) => false,
                                Ok(size) => size == KNOWN_SIZE
                            }
                        }
                    }
                }
            }
        })
        .collect::<Vec<u32>>();
    if candidates.len() > 1 || candidates.is_empty() {
        println!("Found {} processes that match", candidates.len());
        panic!("Could not find or parse pid of Refunct");
    }
    candidates.first().unwrap().to_owned()
}
