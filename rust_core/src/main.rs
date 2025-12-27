use quicksfv_core::*;
use std::env;
use std::fs;
use std::io::{self, Read};

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() < 2 {
        println!("Usage: {} <file_path> [file_path...]", args[0]);
        println!("       or");
        println!("       {} --stdin", args[0]);
        println!();
        println!("Examples:");
        println!("  {} test.txt", args[0]);
        println!("  echo -n '123456789' | {} --stdin", args[0]);
        std::process::exit(1);
    }
    
    if args[1] == "--stdin" {
        // Read from stdin and compute CRC32
        let mut buffer = Vec::new();
        io::stdin().read_to_end(&mut buffer).expect("Failed to read from stdin");
        
        let crc = unsafe {
            quicksfv_crc32(buffer.as_ptr(), buffer.len())
        };
        
        println!("CRC32: {:08X}", crc);
    } else {
        // Process files
        for file_path in &args[1..] {
            match fs::read(file_path) {
                Ok(data) => {
                    let crc = unsafe {
                        quicksfv_crc32(data.as_ptr(), data.len())
                    };
                    println!("{}: {:08X}", file_path, crc);
                }
                Err(e) => {
                    eprintln!("Error reading {}: {}", file_path, e);
                }
            }
        }
    }
}
