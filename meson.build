project('tint2', 'c',
  version : '0.11.2025',
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

executable_name = 'tint2'
exe = executable(executable_name, srcs,
  include_directories : include_directories('src'),
  dependencies : deps,
  install : true,
  install_dir : '/usr/bin'
)

install_data('tint2rc',
  install_dir : '/etc/xdg/tint2',
)

install_data('default_icon.png',
  install_dir : '/usr/share/tint2'
)
