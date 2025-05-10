subdir('sound')
subdir('themes')

i18n = import('i18n')
desktop_file = 'gmchess.desktop'
i18n.merge_file(
  input: desktop_file + '.in',
  output: desktop_file,
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
