Unit conversion
===============

.. toctree::
    :maxdepth: 2

Unit conversion functions
-------------------------

wreport has its own domain-specific unit conversion functions, working with
unit names from BUFR/CREX B tables.

.. doxygenfunction:: wreport::convert_units
.. doxygenfunction:: wreport::convert_icao_to_press
.. doxygenfunction:: wreport::convert_press_to_icao
.. doxygenfunction:: wreport::convert_octants_to_degrees
.. doxygenfunction:: wreport::convert_degrees_to_octants
.. doxygenfunction:: wreport::convert_AOFVSS_to_BUFR08042
.. doxygenfunction:: convert_WMO0500_to_BUFR20012
.. doxygenfunction:: convert_WMO0509_to_BUFR20012
.. doxygenfunction:: convert_WMO0515_to_BUFR20012
.. doxygenfunction:: convert_WMO0513_to_BUFR20012
.. doxygenfunction:: convert_WMO4677_to_BUFR20003
.. doxygenfunction:: convert_WMO4561_to_BUFR20004
.. doxygenfunction:: convert_BUFR20012_to_WMO0500
.. doxygenfunction:: convert_BUFR20012_to_WMO0509
.. doxygenfunction:: convert_BUFR20012_to_WMO0515
.. doxygenfunction:: convert_BUFR20012_to_WMO0513
.. doxygenfunction:: convert_BUFR20003_to_WMO4677
.. doxygenfunction:: convert_BUFR20004_to_WMO4561
.. doxygenfunction:: convert_BUFR08001_to_BUFR08042
.. doxygenfunction:: convert_BUFR08042_to_BUFR08001
.. doxygenfunction:: convert_units_get_mul
.. doxygenfunction:: convert_units_allowed
