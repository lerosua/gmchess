project('gmchess', 'cpp',
  version: '0.29.6.3',
  meson_version: '>=1.4.0',
  license: 'GPL2',
  default_options : [ 'warning_level=1', 'buildtype=debugoptimized', 'c_std=gnu11'],
)

package_name = meson.project_name()
package_version = meson.project_version()

soversion = 2

package_string = '@0@-@1@.0'.format(package_name, soversion)

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
includedir = join_paths(prefix, get_option('includedir'))
libdir = join_paths(prefix, get_option('libdir'))
libexecdir = join_paths(prefix, get_option('libexecdir'))
datadir = join_paths(prefix, get_option('datadir'))

localedir = join_paths(datadir, 'locale')
pkgdatadir = join_paths(datadir, package_name)

config_h = configuration_data()
config_h.set_quoted('PACKAGE', package_name)
config_h.set_quoted('PACKAGE_NAME', package_name)
config_h.set_quoted('PACKAGE_STRING', package_string)
config_h.set_quoted('PACKAGE_VERSION', package_version)
config_h.set_quoted('GETTEXT_PACKAGE', package_name)
config_h.set_quoted('VERSION', package_version)

config_h.set_quoted('DATA_DIR', pkgdatadir)
config_h.set_quoted('GMCHESS_LOCALEDIR', localedir)

# Compiler & Project arguments
add_project_arguments([
  '-DHAVE_CONFIG_H',
  '-I' + meson.project_build_root(),
  ], language: 'cpp')

# get suported warning flags
test_args = [
  '-fstack-protector-strong',
  '-Wcast-align',
  '-Wchar-subscripts',
  ['-Werror=format-security', '-Werror=format=2'],
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wnested-externs',
  '-Wpointer-arith',
  '-Wshadow',
  '-Wsign-compare',
  '-Wwrite-strings',
  '-fpermissive',
  ]

cpp = meson.get_compiler('cpp')

foreach arg: test_args
  if cpp.has_multi_arguments(arg)
    add_project_arguments(arg, language : 'cpp')
  endif
endforeach

# byte order
if build_machine.endian() == 'big'
  config_h.set('WORDS_BIGENDIAN', 1)
endif

gnome = import('gnome')
i18n = import('i18n')
pkg = import('pkgconfig')

# Paths
rootdir = include_directories('.')

srcdir = meson.current_source_dir()
builddir = meson.current_build_dir()

prefix = get_option('prefix')

buildtype = get_option('buildtype')

bindir = join_paths(prefix, get_option('bindir'))
includedir = join_paths(prefix, get_option('includedir'))
libdir = join_paths(prefix, get_option('libdir'))
libexecdir = join_paths(prefix, get_option('libexecdir'))
datadir = join_paths(prefix, get_option('datadir'))
pkgincludedir = join_paths(includedir, package_string)
pkgconfigdir = join_paths(libdir, 'pkgconfig')
pkglibdir = join_paths(libdir, package_name)
pkgdatadir = join_paths(datadir, package_name)

dbusdir = join_paths(datadir, 'dbus-1')
glibdir = join_paths(datadir, 'glib-2.0')
gtkdocdir = join_paths(datadir, 'gtk-doc')
localdir = join_paths(datadir, 'local')
vapidir = join_paths(datadir, 'vala', 'vapi')

# Dependencies
gtkmm_req = '>= 2.24.0'

gtkmm_dep = dependency('gtkmm-2.4', version: gtkmm_req)

libdl_dep = cpp.find_library('dl', required: true)
if not libdl_dep.found()
  error('can not found libdl')
endif

libiconv_dep = cpp.find_library('iconv', required: true)
if not libiconv_dep.found()
  error('can not found libiconv')
endif

configure_file(
  output: 'config.h',
  configuration: config_h
)

subdir('doc')
subdir('data')
subdir('src')
subdir('po')

summary = [
  '',
  'marco-@0@:'.format(meson.project_version()),
  '',
  '	prefix:                   @0@'.format(prefix),
  '	source code location:     @0@'.format(srcdir),
  '	compiler:                 @0@'.format(cpp.get_id()),
  '',
  '',
]

meson.add_install_script(
  'meson_post_install.py',
  libdir, datadir,
)

message('\n'.join(summary))
