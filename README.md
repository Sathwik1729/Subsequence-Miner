# Advanced Subsequence Miner

This project is a C++ program for mining frequent contiguous and non-contiguous subsequences (patterns) from a set of sequences. It uses a trie-based approach to efficiently find the most common patterns in user interaction or event sequences.

## Features
- Finds frequent contiguous and non-contiguous subsequences in a dataset
- Interactive mode for custom user input
- Demo mode with sample data
- Configurable pattern length, support, and top-k results
- Displays mining statistics and pattern distributions

## Requirements
- C++17 compatible compiler (e.g., g++ 7.0 or later)

## Build Instructions
1. Open a terminal and navigate to the project directory.
2. Compile the program using:
   ```bash
   g++ -Wall -Wextra -std=c++17 subsequence_miner.cpp -o subsequence_miner
   ```

## Usage
Run the program from the terminal:
```bash
./subsequence_miner
```

You will be prompted to choose a mode:

### 1. Interactive Mining (user input)
- Enter your own sequences, one per line (space-separated elements).
- Type `done` when finished, or `demo` to use sample data.
- Configure mining parameters (top-k, min support, max pattern length, pattern type).
- The program will display the most frequent patterns and statistics.

### 2. Demo with Sample Data
- Runs the miner on built-in sample user interaction sequences.
- Shows frequent patterns, top-k, and non-contiguous patterns.

## Example
```
Choose mode:
1. Interactive Mining (user input)
2. Demo with sample data
Enter choice (1 or 2): 1

==================================================
INTERACTIVE SUBSEQUENCE MINER
==================================================

Enter your sequences (space-separated elements per line)
Type 'done' when finished, 'demo' for sample data:
Sequence 1: login browse search view_item add_to_cart checkout
Sequence 2: login browse view_item add_to_cart checkout
Sequence 3: done

Enter k for top-k patterns (default 5): 3
Enter minimum support (default 2): 2
Enter maximum pattern length (default 4): 3
Pattern type - (c)ontiguous, (n)on-contiguous, or (b)oth? (default: both): b

MINING RESULTS
...
```

## Notes
- For large sequences, non-contiguous mining is limited for performance.
- If no patterns are found, try lowering min support or increasing max pattern length.
 