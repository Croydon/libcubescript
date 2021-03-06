project('libcubescript', ['cpp'],
    version: '0.1.0',
    default_options: ['buildtype=plain', 'cpp_std=c++2a'],
    meson_version: '>=0.56'
)

dir_prefix  = get_option('prefix')
dir_include = join_paths(dir_prefix, get_option('includedir'))
dir_data    = join_paths(dir_prefix, get_option('datadir'))
dir_lib     = join_paths(dir_prefix, get_option('libdir'))

dir_package_include = join_paths(dir_include, 'cubescript')

libcubescript_includes = [include_directories('include')]

tgt_compiler_id = meson.get_compiler('cpp').get_id()
if tgt_compiler_id == 'gcc' or tgt_compiler_id == 'clang'
    extra_cxxflags = ['-Wextra', '-Wshadow', '-Wold-style-cast']
else
    extra_cxxflags = []
endif

subdir('src')
subdir('tools')

pkg = import('pkgconfig')

pkg.generate(
    libraries: libcubescript_lib,
    version: meson.project_version(),
    name: 'libcubescript',
    filebase: 'libcubescript',
    url: 'https://git.octaforge.org/octaforge/libcubescript',
    description: 'An embeddable version of the CubeScript language'
)
