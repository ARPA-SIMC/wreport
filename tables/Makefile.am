## Process this file with automake to produce Makefile.in

crexbtables = B000101.txt B000103.txt B000203.txt B001600.txt \
	      B001800.txt B001900.txt B002000.txt B002100.txt B002200.txt
crexdtables = D000101.txt D000103.txt D000203.txt D001600.txt \
	      D001800.txt D001900.txt D002000.txt D002100.txt D002200.txt
bufrbtables = B0000000000000012000.txt \
	      B0000000000000013000.txt \
	      B0000000000098013001.txt \
	      B0000000000098013102.txt \
	      B0000000000000014000.txt \
	      B0000000000085014001.txt \
	      B0000000000000016000.txt \
	      B0000000000098016001.txt \
	      B0000000000000017000.txt \
	      B0000000000000018000.txt \
	      B0000000000000019000.txt \
	      B0000000000254019001.txt \
	      B0000000000000020000.txt \
	      B0000000000000021000.txt \
	      B0000000000000022000.txt \
	      B000000000980000.txt \
	      B000000000980201.txt \
	      B000000000980600.txt \
	      B000000000980601.txt \
	      B000000000981101.txt \
	      B000000000981200.txt \
	      B000000000981201.txt \
	      B000000000001100.txt \
	      B000000002551104.txt \
	      B000000000981301.txt

bufrdtables = D0000000000000012000.txt \
	      D0000000000000013000.txt \
	      D0000000000098013001.txt \
	      D0000000000098013102.txt \
	      D0000000000000014000.txt \
	      D0000000000085014001.txt \
	      D0000000000000016000.txt \
	      D0000000000098016001.txt \
	      D0000000000000017000.txt \
	      D0000000000000018000.txt \
	      D0000000000000019000.txt \
	      D0000000000254019001.txt \
	      D0000000000000020000.txt \
	      D0000000000000021000.txt \
	      D0000000000000022000.txt \
	      D000000000980000.txt \
	      D000000000980201.txt \
	      D000000000980600.txt \
	      D000000000980601.txt \
	      D000000000981101.txt \
	      D000000000981200.txt \
	      D000000000981201.txt \
	      D000000000001100.txt \
	      D000000002551104.txt \
	      D000000000981301.txt

# Table 17 seems only to be available in .doc or .pdf format. I only managed to
# acquire a BUFR version of it, so for CREX I work around the issue by using a
# copy of table 18 instead.
generated_tables = \
	      B001700.txt D001700.txt B0000000000085014000.txt D0000000000085014000.txt

table_DATA = $(crexbtables) $(crexdtables) $(bufrbtables) $(bufrdtables) $(generated_tables)

B001700.txt: B001800.txt
	cp $< $@
D001700.txt: D001800.txt
	cp $< $@
B0000000000085014000.txt: B0000000000085014001.txt
	cp $< $@
D0000000000085014000.txt: D0000000000085014001.txt
	cp $< $@

CLEANFILES = $(generated_tables)

EXTRA_DIST = $(crexbtables) $(crexdtables) $(bufrbtables) $(bufrdtables)
