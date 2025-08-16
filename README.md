# Paging and Memory Management Simulator

A comprehensive C++ implementation of memory management algorithms commonly used in operating systems, focusing on paging techniques with FIFO and LRU page replacement policies.

## 🚀 Overview

This project simulates how an operating system handles process memory using paging techniques. It provides implementations of two popular page replacement algorithms:
- **FIFO (First In First Out)**: Pages are replaced in the order they were loaded
- **LRU (Least Recently Used)**: The least recently accessed page is replaced first

The simulator allows loading, running, and managing processes while demonstrating memory allocation, page replacement, and process termination.

## ✨ Features

- **Process Management**: Load processes from files with unique process IDs
- **Memory Simulation**: Simulates main memory and swap space with configurable sizes
- **Page Replacement Algorithms**: 
  - FIFO implementation (`fifo.cpp`)
  - LRU implementation (`lru.cpp`)
- **Process Operations**: Run, terminate, and manage multiple processes
- **Memory Monitoring**: Print page tables and memory contents
- **Configurable Parameters**: Adjustable memory sizes and page sizes

## 📋 Supported Commands

The simulator supports the following commands through input files:

| Command | Description | Syntax |
|---------|-------------|--------|
| `load` | Load process(es) from file(s) | `load <filename(s)>` |
| `run` | Execute a process by PID | `run <pid>` |
| `kill` | Terminate a process | `kill <pid>` |
| `listpr` | List all active processes | `listpr` |
| `pte` | Print page table for specific process | `pte <pid> <file>` |
| `pteall` | Print page tables for all processes | `pteall <file>` |
| `print` | Display memory contents | `print <memloc> <length>` |
| `exit` | Terminate all processes and exit | `exit` |

## 🛠️ Compilation and Usage

### Prerequisites
- C++ compiler (g++)
- Make utility

### Compilation
```bash
# Compile both algorithms
make

# Or compile individually
make lru    # For LRU algorithm
make fifo   # For FIFO algorithm
```

### Running the Simulator
```bash
# Using LRU algorithm
./lru -M <main_memory_size> -V <swap_space_size> -P <page_size> -i <input_file> -o <output_file>

# Using FIFO algorithm  
./fifo -M <main_memory_size> -V <swap_space_size> -P <page_size> -i <input_file> -o <output_file>
```

#### Parameters:
- `-M`: Main memory size (in KB)
- `-V`: Swap space size (in KB)  
- `-P`: Page size (in bytes)
- `-i`: Input file containing commands
- `-o`: Output file for results

### Example
```bash
./lru -M 1024 -V 2048 -P 1024 -i commands.txt -o results.txt
```

## 📁 Project Structure

```
├── fifo.cpp        # FIFO page replacement implementation
├── lru.cpp         # LRU page replacement implementation  
├── Makefile        # Build configuration
└── README.md       # Project documentation
```

## 🧠 Algorithm Details

### FIFO (First In First Out)
- Maintains a queue of pages in memory
- When memory is full, removes the oldest page
- Simple but may not reflect actual usage patterns

### LRU (Least Recently Used)
- Tracks when each page was last accessed
- Replaces the page that hasn't been used for the longest time
- More complex but generally more effective than FIFO

## 📚 Implementation Details

### Process Class
Each process contains:
- **PID**: Unique process identifier
- **Page Distribution**: Tracks which pages are in main memory vs swap space
- **Page Table**: Maps virtual pages to physical frames
- **File Name**: Associated input file

### Memory Management
- **Main Memory**: Fast access, limited size
- **Swap Space**: Larger capacity, slower access
- **Page Replacement**: Automatic swapping based on chosen algorithm

## 🔧 Sample Input File Format

```
load program1.txt program2.txt
run 1
print 0 100
pte 1 page_table.txt
kill 1
exit
```

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is part of an Operating Systems course (CS3500) and is intended for educational purposes.

## 🎯 Learning Objectives

This project helps understand:
- Virtual memory management concepts
- Page replacement algorithms
- Process memory allocation
- Operating system design principles
- C++ system programming

## 🔍 Future Enhancements

- [ ] Add more page replacement algorithms (Clock, Second Chance)
- [ ] Implement memory compaction
- [ ] Add performance metrics and statistics
- [ ] Support for variable-sized pages
- [ ] Graphical visualization of memory state

---

**Author**: Prem Sagar K  
**Course**: CS3500 - Operating Systems  
**Institution**: IIT Madras
