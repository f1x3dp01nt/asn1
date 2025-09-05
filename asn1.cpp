#include <iomanip>
#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cstdint>
#include <stdbool.h>
#include <memory>
using namespace std;

namespace tags
{
    const uint8_t integer = 0x2;
    const uint8_t bit_string = 03;
    const uint8_t octet_string = 0x4;
    const uint8_t null = 0x5;
    const uint8_t oid = 0x6;
    const uint8_t utf8_string = 0xc;
    const uint8_t printable_string = 0x13;
    const uint8_t utc_time = 0x17;
    // bad taxonomy but fine for now
    const uint8_t constructed = 0x20;
    // TODO find out what's the deal with 0x30 vs 0x10 and 0x31 vs 0x11
    const uint8_t sequence = 0x30;
    const uint8_t set = 0x31;
}

// YYMMDDhhmm
const size_t MIN_UTC_TIME_LEN = 10;
// YYMMDDhhmmss-NNNN
const size_t MAX_UTC_TIME_LEN = 17;

class FormatError : public runtime_error
{
public:
    FormatError(const char* what) : runtime_error(what)
    {
    }
};

class DecoderVisitor
{
public:
    virtual void do_integer(
        unique_ptr<vector<uint8_t>> integer, bool negative) = 0;
    virtual void do_null() = 0;
    virtual void do_bit_string(unique_ptr<vector<uint8_t>> bs) = 0;
    virtual void do_oid(unique_ptr<vector<vector<uint8_t>>> oid) = 0;
    virtual void do_printable_string(unique_ptr<string> s) = 0;
    virtual void do_utc_time(unique_ptr<string> s) = 0;
    virtual void do_constructed_start() = 0;
    virtual void do_sequence_start() = 0;
    virtual void do_sequence_end() = 0;
    virtual void do_set_start() = 0;
    virtual void do_set_end() = 0;
};
    
class DecoderPrintVisitor : public DecoderVisitor
{
private:
    ostream& _out;
public:
    DecoderPrintVisitor(ostream& out) : _out(out)
    {
    }

    virtual void do_integer(unique_ptr<vector<uint8_t>> integer, bool negative) override
    {
        _out << "INTEGER ";
        if (negative)
        {        
            _out << "-";
        }
        for (uint8_t b : *integer)
        {
            _out << hex << setfill('0') << setw(2);
            _out << (uint64_t)b;
        }
        _out << endl;
    }

    virtual void do_null() override
    {
        _out << "NULL" << endl;
    }

    virtual void do_bit_string(unique_ptr<vector<uint8_t>> bs) override
    {
        _out << "BIT STRING ";
        for (uint8_t b : *bs)
        {
            _out << hex << setfill('0') << setw(2);
            _out << (uint64_t)b;
        }
        _out << endl;
    }

    virtual void do_oid(unique_ptr<vector<vector<uint8_t>>> oid) override
    {
        _out << "OID(";
        for (vector<uint8_t>& component : *oid)
        {
            for (uint8_t b : component)
            {
                _out << hex << setfill('0') << setw(2);
                _out << (uint64_t)b;
            }
            _out << ".";
        }
        _out << ")" << endl;
    }

    virtual void do_printable_string(unique_ptr<string> s) override
    {
        _out << "PrintableString " << *s << endl;
    }

    virtual void do_constructed_start() override
    {
        _out << "(constructed)" << endl;
    }

    virtual void do_sequence_start() override
    {
        _out << "SEQUENCE(" << endl;
    }

    virtual void do_sequence_end() override
    {
        _out << ")" << endl;
    }

    virtual void do_set_start() override
    {
        _out << "SET{" << endl;
    }

    virtual void do_set_end() override
    {
        _out << "}" << endl;
    }

    virtual void do_utc_time(unique_ptr<string> s) override
    {
        _out << "UTCTime " << *s << endl;
    }

};

/** Decodes an ASN.1 payload from the supplied byte vector,
    feeding the results into the supplied DecoderVisitor.

    Does not support objects of length 2**64 or higher.

    @throws FormatError upon encountering non-conforming input
*/
class Decoder
{
private:
    vector<uint8_t> _data;
    size_t _offset = 0;
    DecoderVisitor& _visitor;
    //Spec _spec;

    void _chklen(size_t n)
    {
        if (_data.size() - _offset < n)
        {
            throw FormatError("data truncated");
        }
    }

    uint64_t dec_len()
    {
        _chklen(1);
        size_t len_len_code = _data[_offset++];

        if (len_len_code < 0x80)
        {
            return len_len_code;
        }
        else
        {
            // the max length-length is reserved
            if (len_len_code == 0xff)
            {
                throw FormatError("invalid length-length");
            }
            size_t len_len = len_len_code & 0x7f;
            if (len_len > 8)
            {
                throw FormatError("refuse to handle big length-length");
            }

            _chklen(len_len);

            uint64_t len = 0;
            for (size_t i = 0; i < len_len; i++)
            {
                len = (len << 8) + _data[_offset++];
            }
            return len;
        }
    }

    void dec_integer(uint64_t len)
    {
        _chklen(len);
        vector<uint8_t>& integer = *new vector<uint8_t>;
        vector<uint8_t>& tmp = *new vector<uint8_t>;

        integer.reserve(len);
        tmp.reserve(len);
        bool negative = false;
        for (size_t i = 0; i < len; i++) {
            uint8_t b = _data[_offset++];
            if (i == 0 && b > 0x7f)
                negative = true;
            tmp.push_back(b);
        }
        if (negative)
        {
            uint8_t carry = 0;
            for (auto i = tmp.rbegin(); i != tmp.rend(); i++)
            {
                uint8_t x = ~(*i);
                integer.push_back(x + carry);
                carry = (carry && x == 0xff);
            }
            if (carry)
                integer.push_back(1);
            reverse(integer.begin(), integer.end());
        }
        else
        {
            integer = tmp;
        }
        _visitor.do_integer(unique_ptr<vector<uint8_t>>(&integer), negative);
     }

    void dec_null(uint64_t len)
    {
        // hmm. the test cert had this so we have to allow it too
        if (len == 0)
        {
        }
        else
        {
            // TODO verify if it should require this
            if (len != 1)
                throw FormatError("wrong NULL length");
            _chklen(1);
            if (_data[_offset++] != 0)
                throw FormatError("wrong NULL value");
        }
        _visitor.do_null();
    }

    void dec_bit_string(uint64_t len)
    {
        vector<uint8_t>& v = *new vector<uint8_t>;
        if (len == 0)
        {
            throw FormatError("bitstring missing 'unused' prefix");
        }
        _chklen(1);
        uint8_t unused = _data[_offset++];
        for (size_t i = 0; i < len - 1; i++)
        {
            _chklen(1);
            v.push_back(_data[_offset++]);
        }
        if (v.size())
        {
            v[v.size() - 1] &= ~((1 << unused) - 1);
        }
        _visitor.do_bit_string(unique_ptr<vector<uint8_t>>(&v));
    }

    void dec_oid(uint64_t remaining)
    {
        vector<vector<uint8_t>>& oid = *new vector<vector<uint8_t>>;
        while (remaining)
        {
            vector<uint8_t> component;

            // collect all bytes of the component
            size_t i;
            for (i = 0; ; i++)
            {
                // TODO decide if we should handle exceeded `len` early here
                _chklen(1);
                uint8_t b = _data[_offset++];
                remaining--;
                component.push_back(b & ~0x80);
                if (!(b & 0x80))
                {
                    break;
                }
            }
            // condense bits rightward
            // i.e. if each byte is like [ bits ],
            // [ 0 a b c d e f g ] [ 0 h i j k l m n ] [ 0 o p q r s t u ]
            // becomes:
            // [ 0 0 0 a b c d e ] [ f g h i j k l m ] [ n o p q r s t u ]
            uint8_t h = 7;
            uint8_t b = component[i];
            do
            {
                uint8_t pb = i >= 1 ? component[i - 1] : 0;
                component[i] = (pb << h) | b;
                b = pb >> (8 - h);
                h--;
                if (h < 1)
                {
                    h = 7;
                }
            } while (i--);
            oid.push_back(component);
        }
        _visitor.do_oid(unique_ptr<vector<vector<uint8_t>>>(&oid));
    }

    void dec_printable_string(uint64_t len)
    {
        string* s = new string;
        for (size_t i = 0; i < len; i++)
        {
            uint8_t chr = _data[_offset++];
            // TODO proper validation
            if (chr < 0x1f || chr > 0x7f)
            {
                throw FormatError("invalid date");
            }
            s->push_back(chr);
        }
        _visitor.do_printable_string(unique_ptr<string>(s));
    }

    void dec_sequence(uint64_t len)
    {
        _visitor.do_sequence_start();
        dec_asn1(len);
        _visitor.do_sequence_end();
    }

    void dec_set(uint64_t len)
    {
        _visitor.do_set_start();
        dec_asn1(len);
        _visitor.do_set_end();
    }

    void dec_utc_time(uint64_t len)
    {
        if (len < MIN_UTC_TIME_LEN || len > MAX_UTC_TIME_LEN)
            throw FormatError("invalid UTCTime len");
        _chklen(len);
        string* s = new string;
        for (size_t i = 0; i < len; i++)
        {
            uint8_t chr = _data[_offset++];
            // TODO proper validation
            if (chr < 0x1f || chr > 0x7f)
            {
                throw FormatError("invalid date");
            }
            s->push_back(chr);
        }
        _visitor.do_utc_time(unique_ptr<string>(s));
    }

    void dec_asn1(uint64_t len)
    {
        size_t start_offset = _offset;
        while (_offset - start_offset < len)
        {
            _chklen(1);
            uint8_t type = _data[_offset++];

            // record tag class
            uint8_t tag_class = (type & 0xc0) >> 6;
            // clear it from the type
            type &= ~0xc0;

            uint64_t len = dec_len();
            switch (type)
            {
            case tags::null:
                dec_null(len);
                break;
            case tags::bit_string:
                dec_bit_string(len);
                break;
            case tags::oid:
                dec_oid(len);
                break;
            case tags::printable_string:
                dec_printable_string(len);
                break;
            case tags::utc_time:
                dec_utc_time(len);
                break;
            case tags::integer:
                dec_integer(len);
                break;
            case tags::constructed:
                _visitor.do_constructed_start();
                break;
            case tags::sequence:
                dec_sequence(len);
                break;
            case tags::set:
                dec_set(len);
                break;
            default:
                throw FormatError("unknown type");
            }
        }
        if (_offset - start_offset != len)
        {
            throw FormatError("element(s) were too big");
        }
    }

public:
    Decoder(vector<uint8_t>& data, DecoderVisitor& visitor) :
        _data(data), _visitor(visitor)
    {
    }

    void dec()
    {
        dec_asn1(_data.size());
    }
};

int usage()
{
    cout << "usage: <file with ASN.1 payload>" << endl;
    return 2;
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        return usage();
    }

    char* input_file = argv[1];

    filebuf fb;
    if (fb.open(input_file, ios_base::binary | ios_base::in) == NULL)
    {
        cerr << "failed to open file" << endl;
        return 1;
    }
    vector<uint8_t> v;
    for(;;) {
        char buf[1024];
        streamsize n = fb.sgetn(buf, sizeof(buf));
        for (size_t i = 0 ; i < n; i++)
            v.push_back(buf[i]);
        if (n != sizeof(buf))
            break;
    }
    try
    {
        Decoder dec(v, *new DecoderPrintVisitor(cout));
        dec.dec();
    }
    catch (FormatError& fe)
    {
        cout << "decoding failed: " << fe.what() << endl;
    }
}
