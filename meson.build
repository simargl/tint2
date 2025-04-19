project('tint2', 'c',
  version : '0.6.0',
  default_options : ['warning_level=2', 'buildtype=debug']
)

# Dependencies
deps = [
  dependency('cairo'),
  dependency('pangocairo'),
  dependency('x11'),
  dependency('xinerama'),
  dependency('xrender'),
  dependency('xrandr'),
  dependency('xdamage'),
  dependency('xcomposite'),
  dependency('imlib2'),
  dependency('glib-2.0')
]

# Source files (Assuming source files are now in src/ directory)
srcs = files(
  'src/area.c',
  'src/battery.c',
  'src/clock.c',
  'src/common.c',
  'src/config.c',
  'src/launcher.c',
  'src/panel.c',
  'src/server.c',
  'src/systraybar.c',
  'src/task.c',
  'src/taskbar.c',
  'src/taskbarname.c',
  'src/timer.c',
  'src/tint.c',
  'src/tooltip.c',
  'src/window.c',
  'src/xsettings-client.c',
  'src/xsettings-common.c'
)

# Build the executable
executable_name = 'tint2'
exe = executable(executable_name, srcs,
  include_directories : include_directories('src'),
  dependencies : deps,
  install : true,
  install_dir : '/usr/bin'  # Set binary install directory to /usr/bin
)

# Install default config file (Relative path from the top directory)
install_data('sample.tint2rc',
  install_dir : '/etc/xdg/tint2',  # Set config file directory to /etc/xdg/tint2
  rename : 'tint2rc'
)

# Install default icon to /usr/share/tint2 (Relative path from the top directory)
install_data('default_icon.png',
  install_dir : '/usr/share/tint2'  # Set icon directory to /usr/share/tint2
)
