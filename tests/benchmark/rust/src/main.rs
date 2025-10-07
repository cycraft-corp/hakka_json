use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader};
use std::process;

// Get RSS (Resident Set Size) in kilobytes from /proc/self/status
fn get_rss_kb() -> i64 {
    #[cfg(target_os = "linux")]
    {
        if let Ok(file) = File::open("/proc/self/status") {
            let reader = BufReader::new(file);
            for line in reader.lines() {
                if let Ok(line) = line {
                    if line.starts_with("VmRSS:") {
                        // Extract the number from "VmRSS:    12345 kB"
                        let parts: Vec<&str> = line.split_whitespace().collect();
                        if parts.len() >= 2 {
                            if let Ok(rss) = parts[1].parse::<i64>() {
                                return rss;
                            }
                        }
                    }
                }
            }
        }
    }
    -1 // Not available on non-Linux systems
}

fn main() {
    let args: Vec<String> = env::args().collect();
    if args.len() != 2 {
        eprintln!("Usage: {} <input_file>", args[0]);
        process::exit(1);
    }

    // Measure RSS before loading
    let rss_before = get_rss_kb();

    let file = File::open(&args[1]).unwrap_or_else(|err| {
        eprintln!("Failed to open the file: {}", err);
        process::exit(1);
    });

    let reader = BufReader::new(file);
    let mut json_array: Vec<serde_json::Value> = Vec::new();

    for line in reader.lines() {
        if let Ok(line) = line {
            match serde_json::from_str::<serde_json::Value>(&line) {
                Ok(obj) => json_array.push(obj),
                Err(e) => {
                    eprintln!("Failed to parse JSON: {}", e);
                    continue;
                }
            }
        }
    }

    // Measure RSS after loading
    let rss_after = get_rss_kb();

    if rss_before >= 0 && rss_after >= 0 {
        let rss_diff = rss_after - rss_before;
        println!("rust RSS: {} KB", rss_diff);
    } else {
        eprintln!("RSS measurement not available on this platform");
    }
}
