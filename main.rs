use std::collections::{BTreeSet, VecDeque};
use std::env;
use std::fs;
use std::path::PathBuf;

fn main() -> std::io::Result<()> {
    // Determine root path from command line arguments
    let args: Vec<String> = env::args().collect();
    let root = if args.len() == 1 {
        PathBuf::from(".")
    } else if args.len() == 2 {
        PathBuf::from(&args[1])
    } else {
        return Ok(());
    };

    // BTreeSet keeps entries sorted (like Java's TreeSet)
    let mut dir_paths = BTreeSet::new();
    let mut file_paths = BTreeSet::new();
    let mut queue = VecDeque::new();

    queue.push_back(root);

    while let Some(path) = queue.pop_front() {
        if let Ok(metadata) = fs::metadata(&path) {
            if metadata.is_dir() {
                if let Ok(canonical) = fs::canonicalize(&path) {
                    let mut dir = canonical.display().to_string();
                    if !dir.ends_with('/') {
                        dir.push('/');
                    }
                    dir_paths.insert(dir);
                }
                if let Ok(entries) = fs::read_dir(&path) {
                    for entry in entries.flatten() {
                        queue.push_back(entry.path());
                    }
                }
            } else if metadata.is_file() {
                if let Ok(canonical) = fs::canonicalize(&path) {
                    file_paths.insert(canonical.display().to_string());
                }
            }
        }
    }

    // Print sorted directories
    for dir in &dir_paths {
        println!("{dir}");
    }

    // Print sorted files
    for file in &file_paths {
        println!("{file}");
    }

    println!("Files: {}", file_paths.len());
    println!("Folders: {}", dir_paths.len());

    Ok(())
}
