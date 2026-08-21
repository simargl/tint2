# Tint2

## Dependencies

### Debian/Ubuntu

```bash
sudo apt update
sudo apt install gcc pkg-config libcairo2-dev libpango1.0-dev libx11-dev libxinerama-dev libxrender-dev libxrandr-dev libxdamage-dev libxcomposite-dev libimlib2-dev libglib2.0-dev
```

### Arch Linux

```bash
sudo pacman -S gcc pkg-config cairo pango libx11 libxinerama libxrender libxrandr libxdamage libxcomposite imlib2 glib2
```

## Build

```bash
git clone https://github.com/simargl/tint2.git
cd tint2
make
```

## Install

```bash
sudo make install
```

## Run

```bash
tint2 &
```

## Uninstall

```bash
sudo make uninstall
```

## Clean

```bash
make clean
```

## Configuration

Edit:

```text
~/.config/tint2/tint2rc
```

To start Tint2 automatically, add `tint2 &` to your desktop's startup applications.
