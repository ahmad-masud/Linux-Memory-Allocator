# Linux Memory Allocator

Linux Memory Allocator is a multithreaded C++ memory allocator designed to provide advanced allocation strategies and optimized memory management. This project offers flexibility and efficiency for managing memory in C++ applications.

## Features

- **Multithreaded Support**: Safely manage memory across multiple threads.
- **Flexible Allocation Strategies**: Choose between various allocation algorithms, including First Fit and Best Fit.
- **Memory Optimization**: Efficiently handle memory with compaction and merging of contiguous free blocks.
- **Detailed Statistics**: Get insights into memory usage and allocation statistics.

## Getting Started

### Prerequisites

- C++11 or later
- `g++` for compiling

### Building the Project

1. Clone the repository:

    ```bash
    git clone https://github.com/ahmad-masud/Linux-Memory-Allocator
    ```

2. Navigate to the project directory:

    ```bash
    cd Linux-Memory-Allocator
    ```

3. Build the project using `make`:

    ```bash
    make
    ```

### Usage

1. Run the executable:

    ```bash
    ./myalloc
    ```

2. The program will demonstrate allocation and deallocation of memory using the custom allocator.

### Contributing

Contributions are welcome! Please open an issue or submit a pull request with your proposed changes.

### License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Inspired by classic memory management techniques.
- Developed with modern C++ features for enhanced performance and safety.
