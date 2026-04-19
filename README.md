
# Linux Shell Clone (C)

## Overview

This project is a **multi-phase Linux shell clone implemented in C**, developed as part of an Operating Systems course. It evolves from a simple local shell into a **remote, multi-client, multithreaded shell with a custom scheduling system**.

The final implementation supports:

* Local command execution
* Remote command execution via sockets
* Multi-client handling using threads
* Execution of user programs (e.g., `gcc file.c`)
* A custom **preemptive scheduler** for controlled task execution

---

## Project Phases

### Phase 1: Local CLI Shell

* Basic shell implementation
* Supports 15 built-in commands (`ls`, `pwd`, `cp`, etc.)
* Handles compound commands with pipes and redirection
* Improved error handling

### Phase 2: Remote CLI Shell

* Introduces **client-server architecture**
* Commands sent from client → executed on server
* Output returned to client via sockets
* Supports compiling and executing programs remotely

### Phase 3: Remote Multitasking CLI Shell

* Adds **multithreading (pthreads)** on server
* Multiple clients can connect simultaneously
* Server logs include client IP address and port
* Improved concurrency and responsiveness

### Phase 4: Remote Multitasking CLI Shell with Scheduling

* Introduces a **custom scheduler system**
* Supports execution of:

  ```
  ./demo [burst_time]
  ```
* Features:

  * Shortest Job First (SJF-inspired ordering)
  * Round Robin scheduling:

    * First run → 3 seconds
    * Subsequent runs → 7 seconds
  * **Preemptive scheduling**
  * Prevents same task from running consecutively
* Uses:

  * Threads
  * Semaphores
  * Mutex locks
  * Shared task queue

---

## Features

* Command execution via `execvp()`
* Pipe (`|`) and redirection (`<`, `>`, `2>`) support
* Program compilation and execution (`gcc`)
* Multi-client socket communication
* Thread-safe task scheduling
* Real-time output streaming for scheduled tasks

---

## File Structure

* `server.c` → Handles client connections and command execution
* `client.c` → Client interface for sending commands
* `myshell.c` → Core shell logic
* `shell_Commands.c` → Command execution handler
* `combinations_F.c` → Compound command handling
* `helper_Functions.c` → Utility functions
* `scheduler.c / scheduler.h` → Scheduling system
* `demo.c` → Test program for scheduler

---

## Compilation

To compile everything at once:

```bash
make all
```

This builds:

* `myshell` (server)
* `client`
* `demo`

To clean compiled files:

```bash
make clean
```

---

## How to Run

### Step 1: Start the Server

```bash
./myshell
```

### Step 2: Start the Client (in a new terminal)

```bash
./client
```

---

## Usage

### Basic Commands

```
myshell % ls
myshell % pwd
myshell % whoami
```

### Compound Commands

```
myshell % cat < file.txt | sort > output.txt
myshell % echo -e hello\nworld | grep e
```

### Compile & Execute Programs

```
myshell % gcc hello.c
myshell % ./hello
```

### Scheduler Command

```
myshell % ./demo 10
```

* Runs a task with a burst time of 10 seconds
* Scheduled alongside other tasks using the custom scheduler

---

## Scheduler Behavior

* Tasks are inserted into a shared queue
* Scheduler selects tasks based on:

  * Shortest burst time
  * Fair rotation (no consecutive execution)
* Supports **preemption** when shorter jobs arrive
* Uses:

  * `pthread`
  * `semaphore`
  * `mutex`

---

## Notes

* The server must always be started **before** the client
* Multiple clients can connect simultaneously
* Non-scheduler commands execute immediately
* Scheduler only applies to `./demo` tasks

---

## References

* POSIX Threads (`pthread`)
* Socket Programming in C
* `execvp`, `fork`, `wait` system calls
* Course materials and lab examples

---
