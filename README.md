# Multithreaded Memory Allocator

## Overview
This project implements a thread-safe malloc and free in C, supporting sequential, coarse-grain, and fine-grain concurrent versions. It manages a pool of memory divided into small (≤64 bytes) and large (65-1024 bytes) blocks, with support for up to 8 concurrent threads.

## Features
- Custom memory allocation (myMalloc) and deallocation (myFree) functions
- Three operational modes: sequential, coarse-grain concurrent, and fine-grain concurrent
- Support for small (64 bytes) and large (1K bytes) memory blocks
- Per-core memory lists in fine-grain mode
- Overflow list for handling memory exhaustion in fine-grain mode
- Thread-safe operations using pthreads

## Requirements
- C compiler (gcc recommended)
- POSIX threads library (pthreads)
- Unix-like environment (for system calls)

## File Structure
- `myMalloc.c`: Main implementation of the memory allocator
- `myMalloc-helper.c`: Helper functions for memory management
- `myMalloc.h`: Header file with function prototypes
- `myMalloc-helper.h`: Header file for helper functions
- `driver.c`: Sample driver program for testing

## Compilation
Use the provided Makefile to compile the project:

make

## Usage
Include "myMalloc.h" in your C program and use the following functions:

int myInit(int numThreads, int flag);
void *myMalloc(int size);
void myFree(void *ptr);

Initialize the allocator with `myInit` before creating threads or using `myMalloc`/`myFree`.

## Implementation Details
- Total memory pool: 276,672 bytes
- Small blocks: ≤64 bytes
- Large blocks: 65-1024 bytes
- Coarse-grain version: Uses a single lock for entire operations
- Fine-grain version: Per-core lists with a shared overflow list
- Constant time operations for myMalloc and myFree

## Testing
A sample driver program (`driver.c`) is provided for basic testing. Implement more comprehensive tests to ensure thread safety and correct operation in all modes.

## Limitations
- Maximum request size: 1K bytes
- Fixed total memory pool size
- Assumes no errors in memory requests
- Supports up to 8 concurrent threads

## Contributing
This project is an academic assignment and is not open for external contributions.

## License
This project is for educational purposes only and is not licensed for commercial use.
