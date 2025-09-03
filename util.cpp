class IndentingOstream : public ostream
{
private:
    ostream& _stream;
    unsigned int _level = 0;
    int _at_start = 0;
public:
    IndentingOstream(ostream& stream) : _ostream(stream)
    {
    }

    f() override
    {
        if (x == '\n')
            _at_start = 1;
        if (_at_start)
        {
            for (unsigned int i = 0; i < _level; i++)
                _stream << " ";
        }
        _stream << x;
        _at_start = 0;
    }
}

