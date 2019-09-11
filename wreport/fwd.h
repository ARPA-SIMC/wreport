#ifndef WREPORT_FWD_H
#define WREPORT_FWD_H

#include <cstdint>

namespace wreport {
class Var;
class Vartable;
class _Varinfo;
typedef const _Varinfo* Varinfo;
typedef uint16_t Varcode;

class Bulletin;
class BufrBulletin;
class CrexBulletin;

class BufrTableID;
class CrexTableID;

class Tables;
class DTable;
}

#endif
