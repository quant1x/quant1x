project('quant1x', 'cpp',
  version: '0.8.19',
  default_options: [
    'cpp_std=c++20',
    'warning_level=1',
    'b_ndebug=if-release',
  ])

# 依赖全部来自 Conan（third_party 已移除）。构建前先执行：
#   conan install . -of build-meson -pr:h conan/profiles/windows-msvc-static \
#       -pr:b default -s build_type=Release --build=missing \
#       -g MesonToolchain -g PkgConfigDeps
#   meson setup build-meson --native-file build-meson/conan_meson_native.ini

cpp = meson.get_compiler('cpp')

deps = [
  dependency('fmt', method: 'pkg-config'),
  dependency('spdlog', method: 'pkg-config'),
  dependency('asio', method: 'pkg-config'),
  dependency('boost-headers', method: 'pkg-config'),
  dependency('backward-cpp', method: 'pkg-config'),
  dependency('bshoshany-thread-pool', method: 'pkg-config'),
  dependency('croncpp', method: 'pkg-config'),
  dependency('csv2', method: 'pkg-config'),
  dependency('date', method: 'pkg-config'),
  dependency('cpp-httplib', method: 'pkg-config'),
  dependency('magic_enum', method: 'pkg-config'),
  dependency('mio', method: 'pkg-config'),
  dependency('nlohmann_json', method: 'pkg-config'),
  dependency('tsl-robin-map', method: 'pkg-config'),
  dependency('protobuf', method: 'pkg-config'),
  dependency('xtensor', method: 'pkg-config'),
  dependency('cpr', method: 'pkg-config'),
  dependency('openssl', method: 'pkg-config'),
  dependency('zlib', method: 'pkg-config'),
  dependency('minizip', method: 'pkg-config'),
  dependency('libiconv', method: 'pkg-config'),
  dependency('yaml-cpp', method: 'pkg-config'),
  dependency('capnp', method: 'pkg-config'),
]

cpp_args = []
if cpp.get_id() == 'msvc'
  cpp_args += ['/utf-8', '/bigobj', '/EHsc', '/DSPDLOG_HEADER_ONLY']
else
  cpp_args += ['-DSPDLOG_HEADER_ONLY']
endif

inc = include_directories('.')

# mpb：quant1x 组织内的多语言进度条库，与本仓库同级（默认 ../mpb）
mpb_inc = include_directories('../mpb')
mpb_lib = static_library('mpb', files(
  '../mpb/format.cpp',
  '../mpb/style.cpp',
  '../mpb/term_like.cpp',
  '../mpb/in_memory.cpp',
  '../mpb/state.cpp',
  '../mpb/draw_target.cpp',
  '../mpb/multi.cpp',
  '../mpb/progress_bar.cpp',
  '../mpb/platform.cpp',
), include_directories: mpb_inc, cpp_args: cpp_args)
mpb_dep = declare_dependency(link_with: mpb_lib, include_directories: mpb_inc)

build_deps = deps + [mpb_dep]

quant1x_sources = files(
  'quant1x/base/strings.cpp',
  'quant1x/base/time.cpp',
  'quant1x/base/filesystem.cpp',
  'quant1x/base/util.cpp',
  'quant1x/base/numeric.cpp',
  'quant1x/base/safe.cpp',
  'quant1x/base/simd.cpp',
  'quant1x/base/cpu_info.cpp',
  'quant1x/base/affinity.cpp',
  'quant1x/base/globmatch.cpp',
  'quant1x/base/argparser/spec.cpp',
  'quant1x/base/argparser/error.cpp',
  'quant1x/base/argparser/parser.cpp',
  'quant1x/base/argparser/help.cpp',
  'quant1x/base/argparser/complete.cpp',
  'quant1x/core/base.cpp',
  'quant1x/runtime/core.cpp',
  'quant1x/runtime/crash.cpp',
  'quant1x/runtime/scheduler.cpp',
  'quant1x/io/http.cpp',
  'quant1x/log/router_sink.cpp',
  'quant1x/log/lazy_daily_sink.cpp',
  'quant1x/encoding/charsets.cpp',
  'quant1x/encoding/json.cpp',
  'quant1x/pandas/rule.cpp',
  'quant1x/contrib/data/tdx/client.cpp',
  'quant1x/contrib/data/tdx/config.cpp',
  'quant1x/contrib/data/tdx/protocol.cpp',
  'quant1x/config/base.cpp',
  'quant1x/config/cache.cpp',
  'quant1x/config/trading_session.cpp',
  'quant1x/config/trader_parameter.cpp',
  'quant1x/config/strategy_parameter.cpp',
  'quant1x/cache.cpp',
  'quant1x/command.cpp',
  'quant1x/formula.cpp',
  'quant1x/backtest/backtest.cpp',
  'quant1x/backtest/position.cpp',
  'quant1x/backtest/order.cpp',
  'quant1x/backtest/stats.cpp',
  'quant1x/backtest/trade.cpp',
  'quant1x/data/adapter.cpp',
  'quant1x/data/cache.cpp',
  'quant1x/data/market.cpp',
  'quant1x/contrib/data/tdx/xdxr.cpp',
  'quant1x/contrib/data/tdx/bar_raw.cpp',
  'quant1x/contrib/data/tdx/bar.cpp',
  'quant1x/contrib/data/tdx/trans.cpp',
  'quant1x/contrib/data/tdx/minute.cpp',
  'quant1x/contrib/data/tdx/chips.cpp',
  'quant1x/contrib/data/tdx/bar_minute.cpp',
  'quant1x/contrib/data/tdx/instruments.cpp',
  'quant1x/contrib/data/tdx/datasource.cpp',
  'quant1x/contrib/data/tdx/sector.cpp',
  'quant1x/data/meta/timestamp.cpp',
  'quant1x/data/meta/calendar.cpp',
  'quant1x/data/meta/session.cpp',
  'quant1x/factors/f10.cpp',
  'quant1x/factors/share_holder.cpp',
  'quant1x/factors/history.cpp',
  'quant1x/factors/notice.cpp',
  'quant1x/factors/financial_report.cpp',
  'quant1x/factors/safety_score.cpp',
  'quant1x/ta/trend.cpp',
  'quant1x/ta/waves.cpp',
  'quant1x/realtime/snapshot.cpp',
  'quant1x/plugins/plugin_loader.cpp',
  'quant1x/engine/rule_engine.cpp',
  'quant1x/engine/strategy.cpp',
  'quant1x/trader/fee.cpp',
  'quant1x/trader/order.cpp',
  'quant1x/trader/account.cpp',
  'quant1x/trader/trader.cpp',
  'quant1x/trader/holding.cpp',
  'quant1x/trader/order_cache.cpp',
  'quant1x/trader/position.cpp',
  'quant1x/trader/tracker.cpp',
  'quant1x/trader/order_state.cpp',
  'quant1x/app.cpp',
  'quant1x/learn/fpgrowth/fp_growth.cpp',
  'quant1x/learn/fpgrowth/fp_growth_core.cpp',
  'quant1x/distributed/id/crc32.cpp',
  'quant1x/distributed/id/error.cpp',
  'quant1x/distributed/id/id.cpp',
  'quant1x/distributed/id/state_store.cpp',
  'quant1x/distributed/id/hlc.cpp',
  'quant1x/distributed/id/queue.cpp',
  'quant1x/distributed/id/generator.cpp',
  'quant1x/proto/chips.pb.cc',
  'quant1x/proto/xdxr.pb.cc',
  'user/strategy-no0.cpp',
  'user/no0.cpp',
)

if host_machine.system() == 'windows'
  quant1x_sources += files('quant1x/runtime/daemon_service.cpp')
endif

quant1x_lib = static_library('quant1x', quant1x_sources,
  include_directories: inc,
  dependencies: build_deps,
  cpp_args: cpp_args,
  install: true)

quant1x_dep = declare_dependency(
  link_with: quant1x_lib,
  include_directories: inc,
  dependencies: build_deps)

git = find_program('git', required: false)
git_hash = ''
git_tag = ''
if git.found()
  git_hash = run_command(git, 'rev-parse', '--short', 'HEAD', check: false).stdout().strip()
  git_tag = run_command(git, 'describe', '--tags', '--abbrev=0', check: false).stdout().strip()
endif

conf = configuration_data()
conf.set('PROJECT_VERSION', meson.project_version())
conf.set('PROJECT_VERSION_MAJOR', '0')
conf.set('PROJECT_VERSION_MINOR', '8')
conf.set('PROJECT_VERSION_PATCH', '19')
conf.set('TAG_COMMIT_HASH', git_hash)
conf.set('LATEST_TAG', git_tag)
conf.set('GIT_AUTHOR_NAME', '')
conf.set('GIT_AUTHOR_EMAIL', '')
conf.set('GIT_AUTHOR_DATE', '')
foreach flag : ['SUPPORTS_C11', 'SUPPORTS_C17', 'SUPPORTS_CXX17', 'SUPPORTS_CXX20']
  conf.set(flag, '1')
endforeach
conf.set('CMAKE_HOST_SYSTEM_NAME', host_machine.system())
conf.set('CMAKE_SYSTEM', host_machine.system())
conf.set('CMAKE_SYSTEM_PROCESSOR', host_machine.cpu())
conf.set('BUILD_TYPE', get_option('buildtype'))
conf.set('TARGET_COMPILE_OPTIONS', ' '.join(cpp_args))
conf.set('C_STANDARD', '11')
conf.set('C_STANDARD_REQUIRED', 'ON')
conf.set('C_EXTENSIONS', 'ON')
conf.set('C_FLAGS', '')
conf.set('C_FLAGS_DEBUG', '')
conf.set('C_FLAGS_RELEASE', '')
conf.set('CXX_STANDARD', '20')
conf.set('CXX_STANDARD_REQUIRED', 'ON')
conf.set('CXX_EXTENSIONS', 'OFF')
conf.set('CXX_FLAGS', '')
conf.set('CXX_FLAGS_DEBUG', '')
conf.set('CXX_FLAGS_RELEASE', '')
conf.set('C_COMPILER_ID', '')
conf.set('C_COMPILER_VERSION', '')
conf.set('C_COMPILER_PATH', '')
conf.set('CXX_COMPILER_ID', cpp.get_id())
conf.set('CXX_COMPILER_VERSION', cpp.version())
conf.set('CXX_COMPILER_PATH', '')
conf.set('CUDA_FLAGS_DEBUG', 'N/A')
conf.set('CUDA_FLAGS_RELEASE', 'N/A')
conf.set('CMAKE_EXE_LINKER_FLAGS_DEBUG', '')
conf.set('CMAKE_EXE_LINKER_FLAGS_RELEASE', '')
foreach dep_name : [
  'DEPENDENCY_OPENSSL', 'DEPENDENCY_ASIO', 'DEPENDENCY_ICONV', 'DEPENDENCY_FMT',
  'DEPENDENCY_ZLIB', 'DEPENDENCY_XSIMD', 'DEPENDENCY_BSHOSHANY_THREAD_POOL',
  'DEPENDENCY_CRONCPP', 'DEPENDENCY_TSL_ROBIN_MAP', 'DEPENDENCY_ARGPARSE',
  'DEPENDENCY_YAML_CPP', 'DEPENDENCY_PROTOBUF', 'DEPENDENCY_CAPNP',
  'DEPENDENCY_MIMALLOC', 'DEPENDENCY_SPDLOG', 'DEPENDENCY_CPR',
  'DEPENDENCY_BENCHMARK', 'DEPENDENCY_CATCH2',
]
  conf.set(dep_name, '')
endforeach
conf.set('HAVE_MIMALLOC', '0')
conf.set('MIMALLOC_MODE', '')

configure_file(
  input: 'quant1x/build-info.h.in',
  output: 'build-info.h',
  configuration: conf)

q1x = executable('q1x', 'main.cpp',
  include_directories: [inc, include_directories('.')],
  dependencies: [quant1x_dep],
  cpp_args: cpp_args,
  install: true)
