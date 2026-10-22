#include "bulletin.h"
#include "common.h"
#include "files.h"
#include "utils/methods.h"
#include "utils/type.h"
#include "utils/values.h"
#include "var.h"
#include "vartable.h"
#include "wreport/utils/string.h"
#include "wreport/vartable.h"
#include <algorithm>
#include <datetime.h>

using namespace std;
using namespace wreport::python;
using namespace wreport;

namespace {
struct RawBufrIterator;
struct RawCrexIterator;
struct BufrIterator;
struct CrexIterator;
struct BufrHeaderIterator;
struct CrexHeaderIterator;
struct BulletinDef;
struct BulletinSubsetIteratorDef;
struct SubsetDef;
struct SubsetVarIteratorDef;
} // namespace

extern "C" {
RawBufrIterator* raw_bufr_iterator = nullptr;
PyTypeObject* wrpy_RawBufrIterator_Type;
RawCrexIterator* raw_crex_iterator = nullptr;
PyTypeObject* wrpy_RawCrexIterator_Type;

BufrIterator* bufr_iterator = nullptr;
PyTypeObject* wrpy_BufrIterator_Type;
CrexIterator* crex_iterator = nullptr;
PyTypeObject* wrpy_CrexIterator_Type;

BufrHeaderIterator* bufr_header_iterator = nullptr;
PyTypeObject* wrpy_BufrHeaderIterator_Type;
CrexHeaderIterator* crex_header_iterator = nullptr;
PyTypeObject* wrpy_CrexHeaderIterator_Type;

BulletinDef* bulletin_def = nullptr;
PyTypeObject* wrpy_Bulletin_Type;
BulletinSubsetIteratorDef* bulletin_subset_iterator_def = nullptr;
PyTypeObject* wrpy_BulletinSubsetIterator_Type;

SubsetDef* subset_def = nullptr;
PyTypeObject* wrpy_Subset_Type;

SubsetVarIteratorDef* subset_var_iterator_def = nullptr;
PyTypeObject* wrpy_SubsetVarIterator_Type;
}

namespace {

template <typename Child, typename Impl>
struct RawIterator : public Type<Child, Impl>
{
    GetSetters<> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        if (self->file)
        {
            delete self->file;
            self->file = nullptr;
        }
        Py_TYPE(self)->tp_free(self);
    }

    static int _init(wrpy_RawBufrIterator* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"file", NULL};
        PyObject* arg_file          = nullptr;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "O", pass_kwlist(kwlist),
                                         &arg_file))
            return -1;

        try
        {
            self->file = new InputFilePython(arg_file);
        }
        WREPORT_CATCH_RETURN_INT;
        return 0;
    }

    static PyObject* _iter(wrpy_RawBufrIterator* self)
    {
        Py_INCREF(self);
        return (PyObject*)self;
    }

    static PyObject* _iternext(wrpy_RawBufrIterator* self)
    {
        std::string buf;
        off_t offset;
        try
        {
            if (!Child::do_read(*self->file, buf, offset))
            {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }

            pyo_unique_ptr bytes(throw_ifnull(
                PyBytes_FromStringAndSize(buf.data(), buf.size())));

            pyo_unique_ptr res(
                throw_ifnull(PyTuple_Pack(2, to_python(offset), bytes.get())));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct RawBufrIterator
    : public RawIterator<RawBufrIterator, wrpy_RawBufrIterator>
{
    constexpr static const char* name      = "RawBufrIterator";
    constexpr static const char* qual_name = "wreport.bulletin.RawBufrIterator";
    constexpr static const char* doc       = R"(
Read a file, generating all BUFR messages that it contains.

Each message is generated as a tuple ``(offset: int, message: str)``.

Messages are raw strings, which need to be decoded to access their contents.
)";

    static bool do_read(wreport::InputFile& file, std::string& buf,
                        off_t& offset)
    {
        return BufrBulletin::read_file(file, buf, &offset);
    }
};

struct RawCrexIterator
    : public RawIterator<RawCrexIterator, wrpy_RawCrexIterator>
{
    constexpr static const char* name      = "RawCrexIterator";
    constexpr static const char* qual_name = "wreport.bulletin.RawCrexIterator";
    constexpr static const char* doc       = R"(
Read a file, generating all BUFR messages that it contains.

Each message is generated as a tuple ``(offset: int, message: str)``.

Messages are raw strings, which need to be decoded to access their contents.
)";

    static bool do_read(wreport::InputFile& file, std::string& buf,
                        off_t& offset)
    {
        return CrexBulletin::read_file(file, buf, &offset);
    }
};

template <typename Child, typename Impl>
struct BulletinIterator : public Type<Child, Impl>
{
    GetSetters<> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        if (self->file)
        {
            delete self->file;
            self->file = nullptr;
        }
        Py_TYPE(self)->tp_free(self);
    }

    static int _init(wrpy_RawBufrIterator* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"file", NULL};
        PyObject* arg_file          = nullptr;
        if (!PyArg_ParseTupleAndKeywords(args, kw, "O", pass_kwlist(kwlist),
                                         &arg_file))
            return -1;

        try
        {
            self->file = new InputFilePython(arg_file);
        }
        WREPORT_CATCH_RETURN_INT;
        return 0;
    }

    static PyObject* _iter(wrpy_RawBufrIterator* self)
    {
        Py_INCREF(self);
        return (PyObject*)self;
    }

    static PyObject* _iternext(wrpy_RawBufrIterator* self)
    {
        try
        {
            auto bulletin = Child::do_read(*self->file);
            if (!bulletin)
            {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }

            return bulletin_to_python(std::move(bulletin));
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct BufrIterator : public BulletinIterator<BufrIterator, wrpy_BufrIterator>
{
    constexpr static const char* name      = "BufrIterator";
    constexpr static const char* qual_name = "wreport.bulletin.BufrIterator";
    constexpr static const char* doc       = R"(
Read a file, generating all decoded BUFR bulletins that it contains.

It generates :py:class:`wreport.Bulletin` objects.
)";

    static std::unique_ptr<wreport::Bulletin> do_read(wreport::InputFile& file)
    {
        std::string buf;
        off_t offset;
        if (!BufrBulletin::read_file(file, buf, &offset))
            return std::unique_ptr<wreport::Bulletin>();
        return BufrBulletin::decode(buf, file.name().c_str(), offset);
    }
};

struct CrexIterator : public BulletinIterator<CrexIterator, wrpy_CrexIterator>
{
    constexpr static const char* name      = "CrexIterator";
    constexpr static const char* qual_name = "wreport.bulletin.CrexIterator";
    constexpr static const char* doc       = R"(
Read a file, generating all decoded CREX bulletins that it contains.

It generates :py:class:`wreport.Bulletin` objects.
)";

    static std::unique_ptr<wreport::Bulletin> do_read(wreport::InputFile& file)
    {
        std::string buf;
        off_t offset;
        if (!CrexBulletin::read_file(file, buf, &offset))
            return std::unique_ptr<wreport::Bulletin>();
        return CrexBulletin::decode(buf, file.name().c_str(), offset);
    }
};

struct BufrHeaderIterator
    : public BulletinIterator<BufrHeaderIterator, wrpy_BufrHeaderIterator>
{
    constexpr static const char* name = "BufrHeaderIterator";
    constexpr static const char* qual_name =
        "wreport.bulletin.BufrHeaderIterator";
    constexpr static const char* doc = R"(
Read a file, decoding BUFR headers only, generating all bulletins that it contains.

It generates :py:class:`wreport.Bulletin` objects.
)";

    static std::unique_ptr<wreport::Bulletin> do_read(wreport::InputFile& file)
    {
        std::string buf;
        off_t offset;
        if (!BufrBulletin::read_file(file, buf, &offset))
            return std::unique_ptr<wreport::Bulletin>();
        return BufrBulletin::decode_header(buf, file.name().c_str(), offset);
    }
};

struct CrexHeaderIterator
    : public BulletinIterator<CrexHeaderIterator, wrpy_CrexHeaderIterator>
{
    constexpr static const char* name = "CrexHeaderIterator";
    constexpr static const char* qual_name =
        "wreport.bulletin.CrexHeaderIterator";
    constexpr static const char* doc = R"(
Read a file, decoding CREX headers only, generating all bulletins that it contains.

It generates :py:class:`wreport.Bulletin` objects.
)";

    static std::unique_ptr<wreport::Bulletin> do_read(wreport::InputFile& file)
    {
        std::string buf;
        off_t offset;
        if (!CrexBulletin::read_file(file, buf, &offset))
            return std::unique_ptr<wreport::Bulletin>();
        return CrexBulletin::decode_header(buf, file.name().c_str(), offset);
    }
};

/*
 * Bulletin
 */

struct fname : public Getter<fname, wrpy_Bulletin>
{
    constexpr static const char* name = "fname";
    constexpr static const char* doc =
        "the name (str) of the file the bulletin "
        "comes from, or None if not available";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (!self->bulletin->fname.empty())
            {
                pyo_unique_ptr res(to_python(self->bulletin->fname));
                return res.release();
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct offset : public Getter<offset, wrpy_Bulletin>
{
    constexpr static const char* name = "offset";
    constexpr static const char* doc =
        "the offset (int) of the bulletin in ``fname``, "
        "or 0 if not available";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            pyo_unique_ptr res(to_python(self->bulletin->offset));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct master_table_number : public Getter<master_table_number, wrpy_Bulletin>
{
    constexpr static const char* name = "master_table_number";
    constexpr static const char* doc  = "Master table number (int).";
    constexpr static void* closure    = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            pyo_unique_ptr res(to_python(self->bulletin->master_table_number));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct data_category : public Getter<data_category, wrpy_Bulletin>
{
    constexpr static const char* name = "data_category";
    constexpr static const char* doc =
        "Data category (int | None, BUFR or CREX table A).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (self->bulletin->data_category != 0xff)
            {
                pyo_unique_ptr res(to_python(self->bulletin->data_category));
                return res.release();
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct data_subcategory : public Getter<data_subcategory, wrpy_Bulletin>
{
    constexpr static const char* name = "data_subcategory";
    constexpr static const char* doc =
        "Data subcategory (int | None, WMO Commmon Code table C-13).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (self->bulletin->data_subcategory != 0xff)
            {
                pyo_unique_ptr res(to_python(self->bulletin->data_subcategory));
                return res.release();
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct data_subcategory_local
    : public Getter<data_subcategory_local, wrpy_Bulletin>
{
    constexpr static const char* name = "data_subcategory_local";
    constexpr static const char* doc =
        "Local data subcategory.\n\n"
        "int | None, defined locally by automatic data processing centres "
        "(ADP).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (self->bulletin->data_subcategory_local != 0xff)
            {
                pyo_unique_ptr res(
                    to_python(self->bulletin->data_subcategory_local));
                return res.release();
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct originating_centre : public Getter<originating_centre, wrpy_Bulletin>
{
    constexpr static const char* name = "originating_centre";
    constexpr static const char* doc =
        "Identification of originating/generating centre.\n\n"
        "int | None, WMO Common Code table C-11.";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (self->bulletin->originating_centre != 0xffff)
            {
                pyo_unique_ptr res(
                    to_python(self->bulletin->originating_centre));
                return res.release();
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct originating_subcentre
    : public Getter<originating_subcentre, wrpy_Bulletin>
{
    constexpr static const char* name = "originating_subcentre";
    constexpr static const char* doc =
        "Identification of originating/generating subcentre.\n\n"
        "int | None, WMO Common Code table C-12.";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (self->bulletin->originating_subcentre != 0xffff)
            {
                pyo_unique_ptr res(
                    to_python(self->bulletin->originating_subcentre));
                return res.release();
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct update_sequence_number
    : public Getter<update_sequence_number, wrpy_Bulletin>
{
    constexpr static const char* name = "update_sequence_number";
    constexpr static const char* doc  = "Update sequence number (int).";
    constexpr static void* closure    = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            pyo_unique_ptr res(
                to_python(self->bulletin->update_sequence_number));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct reftime : public Getter<reftime, wrpy_Bulletin>
{
    constexpr static const char* name = "reftime";
    constexpr static const char* doc  = "Reference time (datetime | None).";
    constexpr static void* closure    = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (self->bulletin->rep_year == 0)
                Py_RETURN_NONE;
            return PyDateTime_FromDateAndTime(
                self->bulletin->rep_year, self->bulletin->rep_month,
                self->bulletin->rep_day, self->bulletin->rep_hour,
                self->bulletin->rep_minute, self->bulletin->rep_second, 0);
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct edition_number : public Getter<edition_number, wrpy_Bulletin>
{
    constexpr static const char* name = "edition_number";
    constexpr static const char* doc =
        "BUFR or CREX edition number (int | None).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        uint8_t edition_number;
        try
        {
            if (const auto* bufr =
                    dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            {
                edition_number = bufr->edition_number;
            }
            else if (const auto* crex =
                         dynamic_cast<const wreport::CrexBulletin*>(
                             self->bulletin))
            {
                edition_number = crex->edition_number;
            }
            else
            {
                Py_RETURN_NONE;
            }
            pyo_unique_ptr res(to_python(edition_number));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct master_table_version_number
    : public Getter<master_table_version_number, wrpy_Bulletin>
{
    constexpr static const char* name = "master_table_version_number";
    constexpr static const char* doc =
        "Version number of BUFR or CREX master table used (int | None).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        uint8_t master_table_version_number;
        try
        {
            if (const auto* bufr =
                    dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            {
                master_table_version_number = bufr->master_table_version_number;
            }
            else if (const auto* crex =
                         dynamic_cast<const wreport::CrexBulletin*>(
                             self->bulletin))
            {
                master_table_version_number = crex->master_table_version_number;
            }
            else
            {
                Py_RETURN_NONE;
            }
            pyo_unique_ptr res(to_python(master_table_version_number));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct master_table_version_number_local
    : public Getter<master_table_version_number_local, wrpy_Bulletin>
{
    constexpr static const char* name = "master_table_version_number_local";
    constexpr static const char* doc  = "Version number of local table used to "
                                        "augment the master table (int | None).";
    constexpr static void* closure    = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        uint8_t master_table_version_number_local;
        try
        {
            if (const auto* bufr =
                    dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            {
                master_table_version_number_local =
                    bufr->master_table_version_number_local;
            }
            else if (const auto* crex =
                         dynamic_cast<const wreport::CrexBulletin*>(
                             self->bulletin))
            {
                master_table_version_number_local =
                    crex->master_table_version_number_local;
            }
            else
            {
                Py_RETURN_NONE;
            }
            pyo_unique_ptr res(to_python(master_table_version_number_local));
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct master_table_version_number_bufr
    : public Getter<master_table_version_number_bufr, wrpy_Bulletin>
{
    constexpr static const char* name = "master_table_version_number_bufr";
    constexpr static const char* doc =
        "BUFR master table version number for CREX (int | None).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (const auto* crex =
                    dynamic_cast<const wreport::CrexBulletin*>(self->bulletin))
            {
                pyo_unique_ptr res(
                    to_python(crex->master_table_version_number_bufr));
                return res.release();
            }
            else
            {
                Py_RETURN_NONE;
            }
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct compression : public Getter<compression, wrpy_Bulletin>
{
    constexpr static const char* name = "compression";
    constexpr static const char* doc =
        "Whether the message is compressed (bool).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (const auto* bufr =
                    dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            {
                if (bufr->compression)
                    Py_RETURN_TRUE;
                else
                    Py_RETURN_FALSE;
            }
            else
            {
                Py_RETURN_FALSE;
            }
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct has_check_digit : public Getter<has_check_digit, wrpy_Bulletin>
{
    constexpr static const char* name = "has_check_digit";
    constexpr static const char* doc =
        "True if the CREX message uses the check digit feature "
        "(bool).";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (const auto* crex =
                    dynamic_cast<const wreport::CrexBulletin*>(self->bulletin))
            {
                if (crex->has_check_digit)
                    Py_RETURN_TRUE;
                else
                    Py_RETURN_FALSE;
            }
            else
            {
                Py_RETURN_FALSE;
            }
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct optional_section : public Getter<optional_section, wrpy_Bulletin>
{
    constexpr static const char* name = "optional_section";
    constexpr static const char* doc =
        "Optional section of the message (bytes | None)";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (const auto* bufr =
                    dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            {
                if (!bufr->optional_section.empty())
                {
                    return PyBytes_FromStringAndSize(
                        bufr->optional_section.data(),
                        bufr->optional_section.size());
                }
            }
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct btable : public Getter<btable, wrpy_Bulletin>
{
    constexpr static const char* name = "btable";
    constexpr static const char* doc =
        "Return the B table used (wreport.Vartable | None).\n\n"
        ":returns: None if the tables have not yet been loaded";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            if (const auto* table = self->bulletin->tables.btable)
                return vartable_create(table);
            Py_RETURN_NONE;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct datadesc : public GetSetter<datadesc, wrpy_Bulletin>
{
    constexpr static const char* name = "datadesc";
    constexpr static const char* doc =
        "Data descriptor section (tuple[str, ...])";
    constexpr static void* closure = nullptr;

    static PyObject* get(Impl* self, void* closure)
    {
        try
        {
            pyo_unique_ptr res(
                throw_ifnull(PyTuple_New(self->bulletin->datadesc.size())));
            for (unsigned pos = 0; pos < self->bulletin->datadesc.size(); ++pos)
            {
                pyo_unique_ptr code(throw_ifnull(
                    wrpy_varcode_format(self->bulletin->datadesc[pos])));
                PyTuple_SET_ITEM(res, pos, code.release());
            }
            return res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }

    static int set(Impl* self, PyObject* value, void* closure)
    {
        try
        {
            std::vector<Varcode> datadesc;

            if (not PySequence_Check(value))
            {
                PyErr_Format(PyExc_TypeError, "%R is not a sequence", value);
                return -1;
            }

            Py_ssize_t size = PySequence_Size(value);
            if (size == -1)
                return -1;

            for (unsigned pos = 0; pos < size; ++pos)
            {
                pyo_unique_ptr val(
                    throw_ifnull(PySequence_GetItem(value, pos)));
                std::string varname = from_python<std::string>(val);
                datadesc.emplace_back(varcode_parse(varname.c_str()));
            }

            self->bulletin->datadesc = datadesc;
            return 0;
        }
        WREPORT_CATCH_RETURN_INT;
    }
};

template <typename Child> struct decode_base : public ClassMethKwargs<Child>
{
    static PyObject* run(PyTypeObject* cls, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"raw", "fname", "offset", nullptr};
        const char* arg_raw         = nullptr;
        Py_ssize_t* arg_raw_size    = 0;
        PyObject* arg_fname         = nullptr;
        PyObject* arg_offset        = nullptr;
        if (!PyArg_ParseTupleAndKeywords(
                args, kw, "s#|OO", const_cast<char**>(kwlist), &arg_raw,
                &arg_raw_size, &arg_fname, &arg_offset))
            return nullptr;

        try
        {
            std::string raw(arg_raw, (size_t)arg_raw_size);
            std::string fname;
            size_t offset = 0;
            if (arg_fname)
            {
                auto path = path_from_python(arg_fname);
                fname     = path.native();
            }
            if (arg_offset)
                offset = int_from_python(arg_offset);

            return Child::decode(raw, fname.c_str(), offset);
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct decode_bufr_header : public decode_base<decode_bufr_header>
{
    constexpr static const char* name = "decode_bufr_header";
    constexpr static const char* signature =
        "raw: bytes, fname: str | None = None, offset: int | None = 0";
    constexpr static const char* returns = "wreport.Bulletin";
    constexpr static const char* summary =
        "Decode the header of a raw BUFR message.";
    constexpr static const char* doc = nullptr;

    static PyObject* decode(const std::string& raw, const char* fname,
                            size_t offset)
    {
        std::unique_ptr<BufrBulletin> bulletin =
            BufrBulletin::decode_header(raw, fname, offset);

        return bulletin_to_python(std::move(bulletin));
    }
};

struct decode_bufr : public decode_base<decode_bufr>
{
    constexpr static const char* name = "decode_bufr";
    constexpr static const char* signature =
        "raw: bytes, fname: str | None = None, offset: int | None = 0";
    constexpr static const char* returns = "wreport.Bulletin";
    constexpr static const char* summary = "Decode a raw BUFR message.";
    constexpr static const char* doc     = nullptr;

    static PyObject* decode(const std::string& raw, const char* fname,
                            size_t offset)
    {
        std::unique_ptr<BufrBulletin> bulletin =
            BufrBulletin::decode(raw, fname, offset);

        return bulletin_to_python(std::move(bulletin));
    }
};

struct decode_crex_header : public decode_base<decode_crex_header>
{
    constexpr static const char* name = "decode_crex_header";
    constexpr static const char* signature =
        "raw: bytes, fname: str | None = None, offset: int | None = 0";
    constexpr static const char* returns = "wreport.Bulletin";
    constexpr static const char* summary =
        "Decode the header of a raw CREX message.";
    constexpr static const char* doc = nullptr;

    static PyObject* decode(const std::string& raw, const char* fname,
                            size_t offset)
    {
        std::unique_ptr<CrexBulletin> bulletin =
            CrexBulletin::decode_header(raw, fname, offset);

        return bulletin_to_python(std::move(bulletin));
    }
};

struct decode_crex : public decode_base<decode_crex>
{
    constexpr static const char* name = "decode_crex";
    constexpr static const char* signature =
        "raw: bytes, fname: str | None = None, offset: int | None = 0";
    constexpr static const char* returns = "wreport.Bulletin";
    constexpr static const char* summary = "Decode a raw CREX message.";
    constexpr static const char* doc     = nullptr;

    static PyObject* decode(const std::string& raw, const char* fname,
                            size_t offset)
    {
        std::unique_ptr<CrexBulletin> bulletin =
            CrexBulletin::decode(raw, fname, offset);

        return bulletin_to_python(std::move(bulletin));
    }
};

struct bulletin_var : public MethKwargs<bulletin_var, wrpy_Bulletin>
{
    constexpr static const char* name = "var";
    constexpr static const char* signature =
        "code: str, value: Union[None, int, float, str, bytes] = None";
    constexpr static const char* returns = "wreport.Var";
    constexpr static const char* summary =
        "create a :py:class:`wreport.Var` using the bulletin's B table.";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"code", "value", nullptr};
        const char* code;
        PyObject* arg_value = nullptr;
        if (!PyArg_ParseTupleAndKeywords(
                args, kw, "s|O", const_cast<char**>(kwlist), &code, &arg_value))
            return nullptr;

        try
        {
            const Vartable* btable = self->bulletin->tables.btable;
            if (!btable)
            {
                PyErr_SetString(PyExc_AssertionError,
                                "bulletin tables have not yet been loaded");
                return nullptr;
            }

            Varinfo info = btable->query(varcode_parse(code));

            if (arg_value and arg_value != Py_None)
                return var_create(info, arg_value);
            else
                return var_create(info);
        }
        WREPORT_CATCH_RETURN_PYO
    }
};

struct bulletin_encode : public MethNoargs<bulletin_encode, wrpy_Bulletin>
{
    constexpr static const char* name      = "encode";
    constexpr static const char* signature = "";
    constexpr static const char* returns   = "bytes";
    constexpr static const char* summary =
        "encode the bulletin back to its source format (BUFR or CREX)";

    static PyObject* run(Impl* self)
    {
        try
        {
            std::string encoded = self->bulletin->encode();
            return PyBytes_FromStringAndSize(encoded.data(), encoded.size());
        }
        WREPORT_CATCH_RETURN_PYO
    }
};

struct BulletinDef : public Type<BulletinDef, wrpy_Bulletin>
{
    constexpr static const char* name      = "Bulletin";
    constexpr static const char* qual_name = "wreport.Bulletin";
    constexpr static const char* doc       = R"(
Decoded BUFR or CREX message.

Each bulletin has properties describing message header information, and works
as a Sequence of subsets with data.
)";
    GetSetters<fname, offset, master_table_number, data_category,
               data_subcategory, data_subcategory_local, originating_centre,
               originating_subcentre, update_sequence_number, reftime,
               edition_number, master_table_version_number,
               master_table_version_number_local,
               master_table_version_number_bufr, compression, has_check_digit,
               optional_section, btable, datadesc>
        getsetters;
    Methods<decode_bufr_header, decode_bufr, decode_crex_header, decode_crex,
            bulletin_var, bulletin_encode>
        methods;

    constexpr static unsigned long tp_flags =
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;

    static void _dealloc(Impl* self)
    {
        delete self->bulletin;
        Py_TYPE(self)->tp_free(self);
    }

    static PyObject* _str(Impl* self)
    {
        std::string res;
        if (dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            res += "BUFR";
        else if (dynamic_cast<const wreport::CrexBulletin*>(self->bulletin))
            res += "CREX";
        else
            res += "????";

        res += "@";

        if (not self->bulletin->fname.empty())
        {
            res += self->bulletin->fname;
            res += "+";
        }

        res += std::to_string(self->bulletin->offset);

        return PyUnicode_FromStringAndSize(res.data(), res.size());
    }

    static PyObject* _repr(Impl* self)
    {
        std::string res = "wreport.Bulletin(type=";
        if (dynamic_cast<const wreport::BufrBulletin*>(self->bulletin))
            res += "\"BUFR\"";
        else if (dynamic_cast<const wreport::CrexBulletin*>(self->bulletin))
            res += "\"CREX\"";
        else
            res += "\"????\"";

        res += ", fname=";
        if (not self->bulletin->fname.empty())
        {
            res += '"';
            res += str::encode_cstring(self->bulletin->fname);
            res += '"';
        }
        else
            res += "None";

        res += ", offset=";
        res += to_string(self->bulletin->offset);
        res += ")";

        return PyUnicode_FromStringAndSize(res.data(), res.size());
    }

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        PyErr_SetString(PyExc_NotImplementedError, "__init__");
        return -1;
    }

    static Py_ssize_t sq_length(Impl* self)
    {
        return self->bulletin->subsets.size();
    }

    static PyObject* sq_item(Impl* self, Py_ssize_t i)
    {
        try
        {
            if (i < 0 or (unsigned) i >= self->bulletin->subsets.size())
            {
                PyErr_SetString(PyExc_IndexError, "invalid subset index");
                return nullptr;
            }
            py_unique_ptr<wrpy_Subset> res(
                throw_ifnull(PyObject_New(wrpy_Subset, wrpy_Subset_Type)));
            Py_XINCREF(self);
            res->bulletin = self;
            res->subset   = i;
            return (PyObject*)res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }

    static PyObject* _iter(Impl* self)
    {
        try
        {
            py_unique_ptr<wrpy_BulletinSubsetIterator> res(
                throw_ifnull(PyObject_New(wrpy_BulletinSubsetIterator,
                                          wrpy_BulletinSubsetIterator_Type)));
            Py_XINCREF(self);
            res->bulletin = self;
            res->subset   = 0;
            return (PyObject*)res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct BulletinSubsetIteratorDef
    : public Type<BulletinSubsetIteratorDef, wrpy_BulletinSubsetIterator>
{
    constexpr static const char* name = "BulletinSubsetIterator";
    constexpr static const char* qual_name =
        "wreport.bulletin.BulletinSubsetIterator";
    constexpr static const char* doc = R"(
Iterate :py:class:`wreport.Subset` entries from a :py:class:`wreport.Bulletin`.
)";
    GetSetters<> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        Py_XDECREF(self->bulletin);
        Py_TYPE(self)->tp_free(self);
    }

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        PyErr_SetString(PyExc_NotImplementedError, "__init__");
        return -1;
    }

    static PyObject* _iter(Impl* self)
    {
        Py_INCREF(self);
        return (PyObject*)self;
    }

    static PyObject* _iternext(Impl* self)
    {
        try
        {
            if (self->subset >= self->bulletin->bulletin->subsets.size())
            {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }

            py_unique_ptr<wrpy_Subset> res(
                throw_ifnull(PyObject_New(wrpy_Subset, wrpy_Subset_Type)));
            Py_XINCREF(self->bulletin);
            res->bulletin = self->bulletin;
            res->subset   = self->subset;

            ++(self->subset);
            return (PyObject*)res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

struct subset_var : public MethKwargs<subset_var, wrpy_Subset>
{
    constexpr static const char* name = "var";
    constexpr static const char* signature =
        "code: str, value: Union[None, int, float, str, bytes] = None";
    constexpr static const char* returns = "wreport.Var";
    constexpr static const char* summary =
        "create a :py:class:`wreport.Var` using the subset bulletin's B table.";

    static PyObject* run(Impl* self, PyObject* args, PyObject* kw)
    {
        static const char* kwlist[] = {"code", "value", nullptr};
        const char* code;
        PyObject* arg_value = nullptr;
        if (!PyArg_ParseTupleAndKeywords(
                args, kw, "s|O", const_cast<char**>(kwlist), &code, &arg_value))
            return nullptr;

        try
        {
            const Vartable* btable = self->bulletin->bulletin->tables.btable;
            if (!btable)
            {
                PyErr_SetString(PyExc_AssertionError,
                                "bulletin tables have not yet been loaded");
                return nullptr;
            }

            Varinfo info = btable->query(varcode_parse(code));

            if (arg_value and arg_value != Py_None)
                return var_create(info, arg_value);
            else
                return var_create(info);
        }
        WREPORT_CATCH_RETURN_PYO
    }
};

struct SubsetDef : public Type<SubsetDef, wrpy_Subset>
{
    constexpr static const char* name      = "Subset";
    constexpr static const char* qual_name = "wreport.Subset";
    constexpr static const char* doc       = R"(
Data subset of a decoded BUFR or CREX message.

It presents as a Sequence of :py:class:`wreport.Var` instances.
)";
    GetSetters<> getsetters;
    Methods<subset_var> methods;

    static void _dealloc(Impl* self)
    {
        Py_XDECREF(self->bulletin);
        Py_TYPE(self)->tp_free(self);
    }

    static PyObject* _str(Impl* self)
    {
        std::string res;
        py_unique_ptr py_str(PyObject_Str((PyObject*)self->bulletin));
        if (!py_str)
        {
            PyErr_Clear();
            res = "[bulletin]";
        }
        else
            res = string_from_python(py_str);
        res += "[";
        res += std::to_string(self->subset);
        res += "]";
        return PyUnicode_FromStringAndSize(res.data(), res.size());
    }

    static PyObject* _repr(Impl* self)
    {
        std::string res;
        py_unique_ptr py_str(PyObject_Repr((PyObject*)self->bulletin));
        if (!py_str)
        {
            PyErr_Clear();
            res = "[bulletin]";
        }
        else
            res = string_from_python(py_str);
        res += "[";
        res += std::to_string(self->subset);
        res += "]";
        return PyUnicode_FromStringAndSize(res.data(), res.size());
    }

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        PyErr_SetString(PyExc_NotImplementedError, "__init__");
        return -1;
    }

    static PyObject* _richcompare(Impl* a, Impl* b, int op)
    {
        PyObject* result;
        bool cmp;

        // Make sure both arguments are Vars.
        if (!(wrpy_Subset_Check(a) && wrpy_Subset_Check(b)))
        {
            result = Py_NotImplemented;
            goto out;
        }

        switch (op)
        {
            case Py_EQ:
                cmp = a->bulletin == b->bulletin and a->subset == b->subset;
                break;
            case Py_NE:
                cmp = a->bulletin != b->bulletin or a->subset != b->subset;
                break;
            default: result = Py_NotImplemented; goto out;
        }
        result = cmp ? Py_True : Py_False;

    out:
        Py_INCREF(result);
        return result;
    }

    static Py_ssize_t sq_length(Impl* self)
    {
        return self->bulletin->bulletin->subsets[self->subset].size();
    }

    static PyObject* _iter(Impl* self)
    {
        try
        {
            py_unique_ptr<wrpy_SubsetVarIterator> res(throw_ifnull(PyObject_New(
                wrpy_SubsetVarIterator, wrpy_SubsetVarIterator_Type)));
            Py_XINCREF(self);
            res->subset = self;
            res->var    = 0;
            return (PyObject*)res.release();
        }
        WREPORT_CATCH_RETURN_PYO;
    }

    static PyObject* mp_subscript(Impl* self, PyObject* key)
    {
        const auto& subset = self->bulletin->bulletin->subsets[self->subset];
        if (PyLong_Check(key))
        {
            int pos = int_from_python(key);
            if (pos < 0)
                pos = subset.size() + pos;
            if (pos < 0 or (unsigned) pos >= subset.size())
            {
                PyErr_SetString(PyExc_IndexError, "invalid var index");
                return nullptr;
            }
            return var_create(subset[pos]);
        }
        else if (PySlice_Check(key))
        {
            Py_ssize_t start, stop, step;
            if (PySlice_Unpack(key, &start, &stop, &step) < 0)
                return nullptr;
            Py_ssize_t slicelength =
                PySlice_AdjustIndices(subset.size(), &start, &stop, step);

            if (step == 0)

            {
                PyErr_SetString(PyExc_ValueError, "slice step cannot be zero");
                return nullptr;
            }

            pyo_unique_ptr result(throw_ifnull(PyTuple_New(slicelength)));
            // fprintf(stderr,
            //         "SLICE len=%zd, start=%zd, stop=%zd, step=%zd, "
            //         "slicelength=%zd\n",
            //         subset.size(), start, stop, step, slicelength);
            for (Py_ssize_t pos = 0; pos < slicelength; ++pos)
            {
                Py_ssize_t idx = start + pos * step;
                // fprintf(stderr, "     res[%zd] = src[%zd]\n", pos, idx);

                pyo_unique_ptr var(throw_ifnull(var_create(subset[idx])));
                PyTuple_SET_ITEM(result, pos, var.release());
            }

            return result.release();
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                         "invalid subscript %R: expecting int or slice", key);
            return nullptr;
        }
    }

    static int mp_ass_subscript(Impl* self, PyObject* key, PyObject* value)
    {
        auto& subset = self->bulletin->bulletin->subsets[self->subset];
        if (PyLong_Check(key))
        {
            int pos = int_from_python(key);
            if (pos < 0)
                pos = subset.size() + pos;
            if (pos < 0 or (unsigned) pos >= subset.size())
            {
                PyErr_SetString(PyExc_IndexError, "invalid var index");
                return -1;
            }

            if (!wrpy_Var_Check(value))
            {
                PyErr_Format(PyExc_TypeError,
                             "variable to assign is %R: expected wreport.Var",
                             value);
                return -1;
            }
            subset[pos] = ((const wrpy_Var*)value)->var;
            return 0;
        }
        else if (PySlice_Check(key))
        {
            Py_ssize_t start, stop, step;
            if (PySlice_Unpack(key, &start, &stop, &step) < 0)
                return -1;
            Py_ssize_t slicelength =
                PySlice_AdjustIndices(subset.size(), &start, &stop, step);

            // Ensure we only assign to contiguous regions
            if (step != 1)
            {
                PyErr_SetString(
                    PyExc_NotImplementedError,
                    "slice assignment is only implemented for step=1");
                return -1;
            }

            // Access the Var instances to assign
            if (!PySequence_Check(value))
            {
                PyErr_Format(
                    PyExc_TypeError,
                    "value is %R: expecting a sequence %zd elements long",
                    value, slicelength);
                return -1;
            }
            Py_ssize_t size = PySequence_Size(value);
            if (size == -1)
                return -1;
            std::vector<Var> value_vars;
            value_vars.reserve(size);
            for (Py_ssize_t pos = 0; pos < size; ++pos)
            {
                pyo_unique_ptr val(
                    throw_ifnull(PySequence_GetItem(value, pos)));
                if (!wrpy_Var_Check(val))
                {
                    PyErr_Format(PyExc_TypeError,
                                 "value at position %zd is %R instead of "
                                 "wreport.Var",
                                 pos, val.get());
                    return -1;
                }
                value_vars.emplace_back(((const wrpy_Var*)val.get())->var);
            }

            Py_ssize_t overlap = std::min(size, slicelength);
            if (overlap > 0)
            {
                // fprintf(stderr, "SLICE ASSIGN assign %zd…%zd\n", start,
                //         start + overlap);
                for (unsigned pos = 0; pos < overlap; ++pos)
                    subset[start + pos] = std::move(value_vars[pos]);
            }

            if (slicelength > size)
            {
                // fprintf(stderr, "SLICE ASSIGN erase %zd…%zd\n", start + size,
                //         stop);
                subset.erase(subset.begin() + start + size,
                             subset.begin() + stop);
            }
            else if (slicelength < size)
            {
                // fprintf(stderr,
                //         "SLICE ASSIGN insert remaining %zd "
                //         "at %zd\n",
                //         value_vars.size() - slicelength, start +
                //         slicelength);
                subset.insert(subset.begin() + start + slicelength,
                              value_vars.begin() + slicelength,
                              value_vars.end());
            }

            return 0;
        }
        else
        {
            PyErr_Format(PyExc_TypeError,
                         "invalid subscript %R: expecting int or slice", key);
            return -1;
        }
    }
};
struct SubsetVarIteratorDef
    : public Type<SubsetVarIteratorDef, wrpy_SubsetVarIterator>
{
    constexpr static const char* name = "SubsetVarIterator";
    constexpr static const char* qual_name =
        "wreport.bulletin.SubsetVarIterator";
    constexpr static const char* doc = R"(
Iterate :py:class:`wreport.Var` entries from a :py:class:`wreport.Subset`.
)";
    GetSetters<> getsetters;
    Methods<> methods;

    static void _dealloc(Impl* self)
    {
        Py_XDECREF(self->subset);
        Py_TYPE(self)->tp_free(self);
    }

    static int _init(Impl* self, PyObject* args, PyObject* kw)
    {
        PyErr_SetString(PyExc_NotImplementedError, "__init__");
        return -1;
    }

    static PyObject* _iter(Impl* self)
    {
        Py_INCREF(self);
        return (PyObject*)self;
    }

    static PyObject* _iternext(Impl* self)
    {
        const Subset& subset =
            self->subset->bulletin->bulletin->subsets[self->subset->subset];
        try
        {
            if (self->var >= subset.size())
            {
                PyErr_SetNone(PyExc_StopIteration);
                return nullptr;
            }

            auto res = var_create(subset[self->var]);
            ++(self->var);
            return res;
        }
        WREPORT_CATCH_RETURN_PYO;
    }
};

} // namespace

namespace wreport {
namespace python {

PyObject* bulletin_to_python(std::unique_ptr<wreport::Bulletin> bulletin)
{
    py_unique_ptr<wrpy_Bulletin> res(
        throw_ifnull(PyObject_New(wrpy_Bulletin, wrpy_Bulletin_Type)));
    res->bulletin = bulletin.release();
    return (PyObject*)res.release();
}

void register_bulletin(PyObject* m, wrpy_c_api& c_api)
{
    /*
     * PyDateTimeAPI, that is used by all the PyDate* and PyTime* macros, is
     * defined as a static variable defaulting to NULL, and it needs to be
     * initialized on each and every C file where it is used.
     */
    if (!PyDateTimeAPI)
    {
        PyDateTime_IMPORT;
        if (PyErr_Occurred())
            throw PythonException();
    }

    raw_bufr_iterator = new RawBufrIterator;
    raw_bufr_iterator->define(wrpy_RawBufrIterator_Type, m);

    raw_crex_iterator = new RawCrexIterator;
    raw_crex_iterator->define(wrpy_RawCrexIterator_Type, m);

    bufr_iterator = new BufrIterator;
    bufr_iterator->define(wrpy_BufrIterator_Type, m);

    crex_iterator = new CrexIterator;
    crex_iterator->define(wrpy_CrexIterator_Type, m);

    bufr_header_iterator = new BufrHeaderIterator;
    bufr_header_iterator->define(wrpy_BufrHeaderIterator_Type, m);

    crex_header_iterator = new CrexHeaderIterator;
    crex_header_iterator->define(wrpy_CrexHeaderIterator_Type, m);

    bulletin_def = new BulletinDef;
    bulletin_def->define(wrpy_Bulletin_Type, m);

    bulletin_subset_iterator_def = new BulletinSubsetIteratorDef;
    bulletin_subset_iterator_def->define(wrpy_BulletinSubsetIterator_Type, m);

    subset_def = new SubsetDef;
    subset_def->define(wrpy_Subset_Type, m);

    subset_var_iterator_def = new SubsetVarIteratorDef;
    subset_var_iterator_def->define(wrpy_SubsetVarIterator_Type, m);
}

} // namespace python
} // namespace wreport
