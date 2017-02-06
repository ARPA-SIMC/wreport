#include "tests.h"
#include "conv.h"
#include "codetables.h"

using namespace wreport;
using namespace wreport::tests;
using namespace std;

namespace {

class Tests : public TestCase
{
    using TestCase::TestCase;

    void register_tests() override
    {
        add_method("units", []() {
            wassert(actual(convert_units("K", "K", 273.15)) == 273.15);
            wassert(actual(convert_units("C", "K", 0.7)).almost_equal(273.85, 4));
            //ensure(convert_units_allowed("C", "K"));
            //ensure(not convert_units_allowed("C", "M"));
            //wassert(actual(convert_units_get_mul("C", "K")) == 1.0);
            wassert(actual(convert_units("RATIO", "%", 1.0)) == 100.0);
            wassert(actual(convert_units("%", "RATIO", 100.0)) == 1.0);

            wassert(actual(convert_units("ms/cm", "S/M", 1.0)) == 0.1);
            wassert(actual(convert_units("S/M", "ms/cm", 0.1)) == 1.0);

            wassert(actual(convert_units("ug/m**3", "KG/M**3", 45)).almost_equal(4.5e-08, 10));
            wassert(actual(convert_units("KG/M**3", "ug/m**3", 0.000000051)).almost_equal(51.0, 10));

            wassert(actual(convert_units("S", "MINUTE", 60.0)) == 1.0);
            wassert(actual(convert_units("MINUTE", "S", 1.0)) == 60.0);

            wassert(actual(convert_units("J/M**2", "MJ/M**2", 1)) == 0.000001);
            wassert(actual(convert_units("MJ/M**2", "J/M**2", 1)) == 1000000);

            wassert(actual(convert_units("octants", "DEGREE TRUE", 0)) == 0);
            wassert(actual(convert_units("octants", "DEGREE TRUE", 1)) == 45);
            wassert(actual(convert_units("DEGREE TRUE", "octants", 45)) == 1);

            wassert(actual(convert_units("FLAG TABLE", "FLAG TABLE", 1)) == 1);
            wassert(actual(convert_units("CODE TABLE", "CODE TABLE", 1)) == 1);

            wassert(actual(convert_units("ppt", "PART PER THOUSAND", 1)) == 1);
            wassert(actual(convert_units("PART PER THOUSAND", "ppt", 1)) == 1);
        });
        add_method("vss", []() {
            // Vertical sounding significance conversion functions
            wassert(actual(convert_BUFR08001_to_BUFR08042(BUFR08001::ALL_MISSING)) == BUFR08042::ALL_MISSING);
            wassert(actual(convert_BUFR08001_to_BUFR08042(BUFR08001::TROPO)) == BUFR08042::TROPO);
            wassert(actual(convert_BUFR08001_to_BUFR08042(BUFR08001::SIGTH)) == (BUFR08042::SIGTEMP | BUFR08042::SIGHUM));

            wassert(actual(convert_BUFR08042_to_BUFR08001(BUFR08042::ALL_MISSING)) == BUFR08001::ALL_MISSING);
            wassert(actual(convert_BUFR08042_to_BUFR08001(BUFR08042::TROPO)) == BUFR08001::TROPO);
            wassert(actual(convert_BUFR08042_to_BUFR08001(BUFR08042::SIGTEMP)) == BUFR08001::SIGTH);
            wassert(actual(convert_BUFR08042_to_BUFR08001(BUFR08042::SIGHUM)) == BUFR08001::SIGTH);
        });
        add_method("octants", []() {
            wassert(actual(convert_octants_to_degrees(0)) ==   0.0);
            wassert(actual(convert_octants_to_degrees(1)) ==  45.0);
            wassert(actual(convert_octants_to_degrees(2)) ==  90.0);
            wassert(actual(convert_octants_to_degrees(3)) == 135.0);
            wassert(actual(convert_octants_to_degrees(4)) == 180.0);
            wassert(actual(convert_octants_to_degrees(5)) == 225.0);
            wassert(actual(convert_octants_to_degrees(6)) == 270.0);
            wassert(actual(convert_octants_to_degrees(7)) == 315.0);
            wassert(actual(convert_octants_to_degrees(8)) == 360.0);

            wassert(actual(convert_degrees_to_octants(  0.0)) ==  0);
            wassert(actual(convert_degrees_to_octants( 10.0)) ==  8);
            wassert(actual(convert_degrees_to_octants( 22.5)) ==  8);
            wassert(actual(convert_degrees_to_octants( 47.0)) ==  1);
            wassert(actual(convert_degrees_to_octants(360.0)) ==  8);
        });
    }
} tests("conv");

}
