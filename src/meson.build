fs = import('fs')
qt5_mod = import('qt5')

# sources
sources = files('main.cpp', 'broadcast/broadcast.cpp', 'events.cpp',
                'sftp/server.cpp', 'sftp/client.cpp', 'storage/storage.cpp', 'sftp/server_handler.cpp')

subdir('windows/app')
tests = [ ]

deps = [
  dependency('boost', modules: ['json']),
  dependency('libssh'),
  dependency('qt5', modules: ['Core', 'Widgets', 'Network'])
]

# test preperation
gtest_proj = subproject('gtest') 

gtest_dep = gtest_proj.get_variable('gtest_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')
gtest_dep = dependency('gtest', main : true, required : false)
gtest_main_dep = dependency('gtest_main', main : true, required : false)
gmock_dep = dependency('gmock', main : true, required : false)

gtest_lib = static_library('gtest', install : true)
gtest_main_lib = static_library('gtest_main', install : true)
gmock_lib = static_library('gmock', install: true)

# executable generation
executable(
  'sync',
  sources,
  dependencies: deps,
  install : true,
)

executable('sync_test',
           sources : tests,
           dependencies : [deps, gtest_dep, gmock_dep, gtest_main_dep],
           link_with : [gtest_lib, gmock_lib, gtest_main_lib])
