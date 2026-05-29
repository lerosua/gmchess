subdir('sound')
subdir('themes')

i18n = import('i18n')
i18n.merge_file(
  input: 'gmchess.desktop.in',
  output: app_id + '.desktop',
  po_dir: join_paths(meson.project_source_root(), 'po'),
  type: 'desktop',
  install: true,
  install_dir : join_paths(datadir, 'applications')
  )

install_data(
  'gmchess.glade',
  'confwin.glade',
  'p1.png',
  'p2.png',
  'book.dat',
  'play.png',
  'gmchess.png',
  install_dir: join_paths(datadir, meson.project_name())
  )

install_data(
  'gmchess.png',
  install_dir: join_paths(datadir, 'pixmaps'),
 )

install_subdir(
  'icons',
  install_dir: datadir,
)

foreach icon_size: ['16x16', '24x24', '32x32', '48x48', '64x64', '128x128', '192x192', '256x256', '512x512']
  install_data(
    join_paths('icons', 'hicolor', icon_size, 'apps', 'gmchess.png'),
    rename: app_id + '.png',
    install_dir: join_paths(datadir, 'icons', 'hicolor', icon_size, 'apps'),
  )
endforeach
