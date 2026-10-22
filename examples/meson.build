toplevel_inc = include_directories('..')

runtest = find_program('../runtest')

ex_dump = executable('dump', 'dump.cc',
    link_with: [libwreport],
    include_directories: toplevel_inc,
)
test('dump', runtest, args: [files('test-dump')], depends: [ex_dump], suite: 'examples')

test('wigos', runtest, args: [files('test-wigos')], suite: 'examples')
