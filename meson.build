project(
  'wqpng',
  ['cpp'],
  version: '0.1.0',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
  ],
)

# ── Options ────────────────────────────────────────────────────────
opt_decoder = get_option('decoder')
opt_encoder = get_option('encoder')
opt_tests = get_option('tests')
opt_install = get_option('install')

# ── Subprojects / External deps ────────────────────────────────────
wqcolor_dep = dependency(
  'wqcolor',
  version: '>=0.1.1',
  fallback: ['wqcolor', 'wqcolor_dep'],
  required: true,
)
wqfile_dep = dependency(
  'wqfile',
  fallback: ['wqfile', 'wqfile_dep'],
  required: true,
)

wuffs_needed = (opt_decoder == 'wuffs' or opt_encoder == 'wuffs')
lodepng_needed = (opt_decoder == 'lodepng' or opt_encoder == 'lodepng')
fpng_needed = (opt_encoder == 'fpng')

if wuffs_needed
  wuffs_dep = dependency('wuffs', fallback: ['wuffs', 'wuffs_dep'])
endif

if fpng_needed
  fpng_dep = dependency('fpng', fallback: ['fpng', 'fpng_dep'], required: true)
endif

if lodepng_needed
  lodepng_dep = dependency('lodepng', fallback: ['lodepng', 'lodepng_dep'], required: true)
endif

# ── Decoder selection ──────────────────────────────────────────────
decoder_deps = []
decoder_defs = []

if opt_decoder == 'wuffs'
  decoder_deps += [wuffs_dep]
  decoder_defs += ['-DWQPNG_USE_WUFFS_FOR_DECODING=1']
elif opt_decoder == 'lodepng'
  decoder_deps += [lodepng_dep]
  decoder_defs += ['-DWQPNG_USE_LODEPNG_FOR_DECODING=1']
else
  error('Unknown decoder: ' + opt_decoder)
endif

# ── Encoder selection ──────────────────────────────────────────────
encoder_deps = []
encoder_defs = []

if opt_encoder == 'fpng'
  encoder_deps += [fpng_dep]
  encoder_defs += ['-DWQPNG_USE_FPNG_FOR_ENCODING=1']
elif opt_encoder == 'lodepng'
  encoder_deps += [lodepng_dep]
  encoder_defs += ['-DWQPNG_USE_LODEPNG_FOR_ENCODING=1']
else
  error('Unknown encoder: ' + opt_encoder)
endif

# ── Export macro (platform) ────────────────────────────────────────
if host_machine.system() == 'windows'
  wqpng_private_defs = ['-DWQPNG_EXPORT=__declspec(dllexport)']
  wqpng_interface_defs = ['-DWQPNG_EXPORT=__declspec(dllimport)']
else
  wqpng_private_defs = ['-DWQPNG_EXPORT=']
  wqpng_interface_defs = ['-DWQPNG_EXPORT=']
endif

# ── Sources ────────────────────────────────────────────────────────
sources = files(
  'sources/decoders/lodepng.cc',
  'sources/decoders/wuffs.cc',
  'sources/encoders/fpng.cc',
  'sources/encoders/lodepng.cc',
)

includes = include_directories('headers')

# ── Library ────────────────────────────────────────────────────────
wqpng_lib = library(
  'wqpng',
  sources,
  include_directories: includes,
  dependencies: decoder_deps + encoder_deps + [wqcolor_dep, wqfile_dep],
  cpp_args: decoder_defs + encoder_defs + wqpng_private_defs,
  install: opt_install,
)

wqpng_dep = declare_dependency(
  link_with: wqpng_lib,
  include_directories: includes,
  compile_args: wqpng_interface_defs,
  dependencies: [wqcolor_dep, wqfile_dep],
)

# ── Install ────────────────────────────────────────────────────────
if opt_install
  install_headers('headers/weqeqq/png.h', subdir: 'weqeqq')
endif

# ── Tests ──────────────────────────────────────────────────────────
if opt_tests
  gtest_dep = dependency('gtest', fallback: ['gtest', 'gtest_dep'], required: true)
  gtest_main_dep = dependency('gtest_main', fallback: ['gtest', 'gtest_main_dep'], required: true)

  png_test = executable(
    'png-test',
    'tests/png_test.cc',
    dependencies: [wqpng_dep, gtest_dep, gtest_main_dep],
  )

  test('png_test', png_test, protocol: 'gtest')
endif
