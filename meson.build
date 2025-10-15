project('tinylytics', 'c')

srcs = files(
  # files go here
  'src/main.c',
  'src/db.c',
  'src/http.c',
  'src/ipc.c',
)

include = include_directories(
  'include'
)

sqlite_dep = dependency('sqlite3', required: true)
microhttpd_dep = dependency('libmicrohttpd', required: true)

executable('tinylytics', srcs,
  include_directories: include,
  install: true,
  dependencies: [sqlite_dep, microhttpd_dep]
)
