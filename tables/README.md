# Format of table files

Table files follow the format of the tables used by the [ECMWF bufrdc
library](https://software.ecmwf.int/wiki/display/BUFR/BUFRDC+Home).

Each table has columns of a fixed size, and there are no column headers.

## B table files

Column 0: 1 space.

Column 1: 6 characters, B table code.

Column 7: 1 space.

Column 8: 64 characters, description.

Column 72: 1 space.

Column 73: 24 characters, BUFR unit.

Column 97: 1 space.

Column 98: 3 characters, BUFR decimal scale.

Column 101: 1 space.

Column 102: 12 characters, binary reference value.

Column 114: 1 space.

Column 115: 3 characters, binary bit length.

Column 118: 1 space.

Column 119: 24 characters, CREX unit.

Column 143: 1 space.

Column 144: 2 characters: CREX decimal scale.

Column 146: 1 space.

Column 147: 9 characters: CREX character length.

## D table files

### First row in a sequence:

Column 0: 1 space.

Column 1: 6 characters, D table code.

Column 7: 1 space.

Column 8: 2 characters, length of code sequence.

Column 10: 1 space. 

Column 11: first B or D table code in the sequence.

### Following rows in a sequence:

Column 0: 1 space.

Column 1: 6 spaces.

Column 7: 1 space.

Column 8: 2 spaces.

Column 10: 1 space. 

Column 11: B or D table code in the sequence.

