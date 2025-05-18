# Tint2 Installation Instructions

## Prerequisites
Install the required development tools and dependencies.

### On Debian/Ubuntu:
```bash
sudo apt update
sudo apt install git meson ninja-build gcc g++ libx11-dev libimlib2-dev
```

### On Arch Linux:
```bash
sudo pacman -S git meson ninja gcc g++ libx11 imlib2
```

## Clone the Repository
```bash
git clone https://github.com/simargl/tint2.git
cd tint2
```

## Configure and Build Using Meson
```bash
meson build
ninja -C build
```

## Install Tint2
```bash
sudo ninja -C build install
```

## Run Tint2
```bash
tint2 &
```

## Additional Tips
- To customize Tint2, edit the configuration file located at `~/.config/tint2/tint2rc`.
- To start Tint2 automatically on login, add `tint2 &` to your desktop environment's startup applications.
