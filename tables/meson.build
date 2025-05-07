crexbtables = files('B000101.txt', 'B000103.txt', 'B000203.txt', 'B003400.txt',
                    'B003500.txt', 'B003600.txt', 'B003700.txt', 'B003801.txt',
                    'B003900.txt', 'B004000.txt', 'B004100.txt')
crexdtables = files('D000101.txt', 'D000103.txt', 'D000203.txt', 'D003400.txt',
                    'D003500.txt', 'D003600.txt', 'D003700.txt', 'D003801.txt',
                    'D003900.txt', 'D004000.txt', 'D004100.txt')
bufrbtables = files(
    'B0000000000000012000.txt',
    'B0000000000000013000.txt',
    'B0000000000098013001.txt',
    'B0000000000098013102.txt',
    'B0000000000000014000.txt',
    'B0000000000085014001.txt',
    'B0000000000000015000.txt',
    'B0000000000000016000.txt',
    'B0000000000098016001.txt',
    'B0000000000000017000.txt',
    'B0000000000000018000.txt',
    'B0000000000000019000.txt',
    'B0000000000254019001.txt',
    'B0000000000000020000.txt',
    'B0000000000000021000.txt',
    'B0000000000000022000.txt',
    'B0000000000000023000.txt',
    'B0000000000000024000.txt',
    'B0000000000000025000.txt',
    'B0000000000000026000.txt',
    'B0000000000000027000.txt',
    'B0000000000000028000.txt',
    'B0000000000000029000.txt',
    'B0000000000000030000.txt',
    'B0000000000000031000.txt',
    'B0000000000000033000.txt',
    'B0000000000098033001.txt',
    'B0000000000098033002.txt',
    'B0000000000000034000.txt',
    'B0000000000000035000.txt',
    'B0000000000000036000.txt',
    'B0000000000000037000.txt',
    'B0000000000000038001.txt',
    'B0000000000000039000.txt',
    'B0000000000000040000.txt',
    'B0000000000000041000.txt',
    'B000000000980000.txt',
    'B000000000980201.txt',
    'B000000000980600.txt',
    'B000000000980601.txt',
    'B000000000981101.txt',
    'B000000000981200.txt',
    'B000000000981201.txt',
    'B000000000001100.txt',
    'B000000002551104.txt',
    'B000000000981301.txt',
)

bufrdtables = files(
    'D0000000000000012000.txt',
    'D0000000000000013000.txt',
    'D0000000000098013001.txt',
    'D0000000000098013102.txt',
    'D0000000000000014000.txt',
    'D0000000000085014001.txt',
    'D0000000000000015000.txt',
    'D0000000000000016000.txt',
    'D0000000000098016001.txt',
    'D0000000000000017000.txt',
    'D0000000000000018000.txt',
    'D0000000000000019000.txt',
    'D0000000000254019001.txt',
    'D0000000000000020000.txt',
    'D0000000000000021000.txt',
    'D0000000000000022000.txt',
    'D0000000000000023000.txt',
    'D0000000000000024000.txt',
    'D0000000000000025000.txt',
    'D0000000000000026000.txt',
    'D0000000000000027000.txt',
    'D0000000000000028000.txt',
    'D0000000000000029000.txt',
    'D0000000000000030000.txt',
    'D0000000000000031000.txt',
    'D0000000000000033000.txt',
    'D0000000000098033001.txt',
    'D0000000000098033002.txt',
    'D0000000000000034000.txt',
    'D0000000000000035000.txt',
    'D0000000000000036000.txt',
    'D0000000000000037000.txt',
    'D0000000000000038001.txt',
    'D0000000000000039000.txt',
    'D0000000000000040000.txt',
    'D0000000000000041000.txt',
    'D000000000980000.txt',
    'D000000000980201.txt',
    'D000000000980600.txt',
    'D000000000980601.txt',
    'D000000000981101.txt',
    'D000000000981200.txt',
    'D000000000981201.txt',
    'D000000000001100.txt',
    'D000000002551104.txt',
    'D000000000981301.txt',
)

cp = find_program('cp')

# Table 17 seems only to be available in .doc or .pdf format. I only managed to
# acquire a BUFR version of it, so for CREX I work around the issue by using a
# copy of table 18 instead.
generated_tables = [
    ['B0000000000085014000.txt', 'B0000000000085014001.txt'],
    ['D0000000000085014000.txt', 'D0000000000085014001.txt'],
]

foreach g: generated_tables
    custom_target(g[0], input: files(g[1]), output: g[0], command: [cp, '@INPUT@', '@OUTPUT@'], install: true, install_dir: table_dir)
endforeach

install_data(
    crexbtables,
    crexdtables,
    bufrbtables,
    bufrdtables,
    install_dir: table_dir
)

check_btable = find_program('check_btable')

tables_ok = custom_target('tables_ok',
    input: bufrbtables,
    output: 'tables_ok',
    command: [check_btable, bufrbtables],
    build_by_default: true,
)

# table_DATA = $(crexbtables) $(crexdtables) $(bufrbtables) $(bufrdtables) $(generated_tables)
# 
# B0000000000085014000.txt: B0000000000085014001.txt
# 	cp $< $@
# D0000000000085014000.txt: D0000000000085014001.txt
# 	cp $< $@
# 
# EXTRA_DIST = $(crexbtables) $(crexdtables) $(bufrbtables) $(bufrdtables) check_btable test.txt README.md

