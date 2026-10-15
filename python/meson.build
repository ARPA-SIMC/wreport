python3.install_sources('wreport/__init__.py', pure: false, subdir: 'wreport')

python_tests = [
    'test-version.py',
    'test-varinfo.py',
    'test-vartable.py',
    'test-var.py',
    'test-wreport.py',
]

runtest = find_program('../runtest')

test('python', runtest, args: ['PYTHON'],
     env: [
         'PYTHON=' + python3_path,
         'PYTHONPATH=' + meson.current_source_dir() + ':' + meson.current_build_dir(),
     ],
     depends:[py_wreport]
)
