#include <cstdlib>

// Print information about the library
void do_info()
{
    printf("Tables search paths (tried in order):\n");
    printf("Extra tables directory: %s (env var WREPORT_EXTRA_TABLES)\n", getenv("WREPORT_EXTRA_TABLES"));
    printf("System tables directory: %s (env var WREPORT_TABLES)\n", getenv("WREPORT_TABLES"));
    printf("Compiled-in default tables directory: %s\n", TABLE_DIR);
}
