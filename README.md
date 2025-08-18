# CLI Utilities 

A set of command-line tools for `macOS` that helps monitor system state and perform useful maintenance tasks directly from the terminal.

## Features

1. **Battery Monitoring**
    - Shows Battery percentage
    - Displays State (charging / discharging)
    - Reports battery Health and Cycle count
    - Estimates Time to full charge or discharge

2. **Junk Cleaner**
    - Scans `~/Library/Caches`, `Xcode temporary folders`, `Safari caches`
    - Suggests removing unnecessary files to free up disk space

3. **Code Counter**
    - Analyzes project files across `C++, Python, Java, JavaScript, and more`
    - Groups results by language and file type

4. **Wi-Fi Monitoring**
    - Lists all known networks
    - Shows current speed and signal strength

5. **Device Monitoring**
    - Tracks `USB drives`, `Headphones`, and `external monitors` when plugged/unplugged
    - Sends notifications directly to the `terminal`

---

## Installing

### Requirements
- macOS 
- CMake >= 3.31
- Clang or GCC

### Build
```bash
git clone https://github.com/Rigbir/CLI-Utilities
cd CLI-Utilities
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

---

## Usage

After installation, you can run the utility directly from the terminal:
```bash
cliutils
```

---

![Menu](images/menu.png)
![Help](images/help.png)
