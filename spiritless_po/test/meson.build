# Build settings for Unit Test of prec_ctrl.
# Copyright © 2023, 2024 OOTA, Masato
# This is published under CC0 1.0.
# For more information, see CC0 1.0 Universal (CC0 1.0) at <https://creativecommons.org/publicdomain/zero/1.0/legalcode>.

project('test_spiritless_po', 'cpp',
    default_options: [
        'cpp_std=c++14', # Catch v3 requires C++14.
        'buildtype=debugoptimized',
        'warning_level=3',
        'b_sanitize=undefined',
        'cpp_debugstl=true',
    ],
    license: 'Boost',
    license_files: ['../LICENSE'],
)

srcs = [
    'Catalog.cpp',
    'MetadataParser.cpp',
    'PluralParser.cpp',
    'PoParser.cpp',
]
incdirs = ['../include']
deps = [dependency('catch2-with-main')]

exe_test = executable(
    'test_spiritless_po',
    srcs,
    include_directories: incdirs,
    dependencies: deps,
)
exe_bench = executable(
    'bench_spiritless_po',
    srcs,
    include_directories: incdirs,
    dependencies: deps,
    override_options: [
        'b_sanitize=none',
        'cpp_debugstl=false'
    ],
    cpp_args : '-DENABLE_BENCHMARK',
)
test('Unit Test', exe_test, timeout: 60)
benchmark('Bench', exe_bench, args: ['[!benchmark]'])
