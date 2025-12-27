use quicksfv_core::*;
use std::env;
use std::fs;
use std::time::Instant;

fn main() {
    let args: Vec<String> = env::args().collect();
    
    if args.len() != 3 {
        println!("Usage: {} <file_path> <iterations>", args[0]);
        println!();
        println!("Example:");
        println!("  {} test.bin 1000", args[0]);
        std::process::exit(1);
    }
    
    let file_path = &args[1];
    let iterations: usize = args[2].parse().expect("Invalid iteration count");
    
    println!("Benchmark: CRC32 computation");
    println!("File: {}", file_path);
    println!("Iterations: {}", iterations);
    println!();
    
    // Read file once
    let data = fs::read(file_path).expect("Failed to read file");
    let file_size = data.len();
    
    println!("File size: {} bytes ({:.2} MB)", file_size, file_size as f64 / 1024.0 / 1024.0);
    println!();
    
    // Warm-up run
    let crc = unsafe { quicksfv_crc32(data.as_ptr(), data.len()) };
    println!("CRC32: {:08X}", crc);
    println!();
    
    // Benchmark
    println!("Running {} iterations...", iterations);
    let start = Instant::now();
    
    for _ in 0..iterations {
        let _ = unsafe { quicksfv_crc32(data.as_ptr(), data.len()) };
    }
    
    let elapsed = start.elapsed();
    let total_bytes = file_size * iterations;
    let throughput = (total_bytes as f64) / elapsed.as_secs_f64();
    
    println!("Time: {:.3} seconds", elapsed.as_secs_f64());
    println!("Average per iteration: {:.3} ms", elapsed.as_secs_f64() * 1000.0 / iterations as f64);
    println!("Throughput: {:.2} MB/s", throughput / 1024.0 / 1024.0);
    println!("Throughput: {:.2} GB/s", throughput / 1024.0 / 1024.0 / 1024.0);
}
