# New in version 3.40

* Fixed an accidental API/ABI break introduced in 3.39 (#61)
* Added `varinfo_create_bufr` and `varinfo_delete_bufr` to create `Varinfo`
  entries outside of tables. (#61)
* Moved those internal functions that are not used by reverse dependencies to
  wreport/internals

# New in version 3.39

* Add new WMO tables
* Allow to override master table version numbers via the
  `WREPORT_MASTER_TABLE_VERSION` environment variable (see
  [README.md](README.md) for details) (#58)
* Add a decoder trace snapshot in test data to detect unexpected behaviour
  changes in the decoder
* Fix incorrect padding when reading values of binary variables
* Redesign calculation of variable domain boundaries (#59)

# New in version 3.38

* Updated code to use C++17 features
* Building against wreport requires C++17 from now on
* API and ABI are kept consistent, except for `wreport/utils/tests.h`

# New in version 3.37

* Reinclude table v. 33 which was excluded with meson

# New in version 3.36

* Include cstdint in wreport/utils/string.cc (#55)

# New in version 3.35

* Fixed a corner case in decoding of associated fields (#52)

# New in version 3.34

* Fixed build on Fedora 36 (#49)

# New in version 3.33

* Fixed decoding of missing values in compressed MODES messages (#48)

# New in version 3.33

* Conversion V <-> mV and NTU <-> LM

# New in version 3.31

* Fixed decoding of associated fields in compressed messages (#47)

# New in version 3.30

* Fixed dtable parser (#43)
* Improved documentation (#42)

# New in version 3.29

* Implemented `var_hook_domain_errors` option, required by dballe:#241

# New in version 3.28

* Implemented `var_clamp_domain_errors` option, required by dballe:#241

# New in version 3.27

* Support Lua 5.4

# New in version 3.26

* Decode compressed string with non-empty reference values (#38)
* Updated wobble and wobblepy
* Always include wreport/version.h so that code that needs it can be compatibile with old versions

# New in version 3.25

* Added NEWS.md (#34)
* Updated lookup/guessing of reference year century on BUFR ed.3 (#36)
* Fix bug with C table modifier 207YYY (#37)
* Improved documentation
