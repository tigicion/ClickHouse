#include <DataStreams/ValuesRowOutputStream.h>

#include <IO/WriteHelpers.h>
#include <Columns/IColumn.h>
#include <DataTypes/IDataType.h>


namespace DB
{


ValuesRowOutputStream::ValuesRowOutputStream(WriteBuffer & ostr_, const FormatSettings & format_settings)
    : ostr(ostr_), format_settings(format_settings)
{
}

void ValuesRowOutputStream::flush()
{
    ostr.next();
}

void ValuesRowOutputStream::writeField(const IColumn & column, const IDataType & type, size_t row_num)
{
    type.serializeTextQuoted(column, row_num, ostr, format_settings);
}

void ValuesRowOutputStream::writeFieldDelimiter()
{
    writeChar(',', ostr);
}

void ValuesRowOutputStream::writeRowStartDelimiter()
{
    writeChar('(', ostr);
}

void ValuesRowOutputStream::writeRowEndDelimiter()
{
    writeChar(')', ostr);
}

void ValuesRowOutputStream::writeRowBetweenDelimiter()
{
    writeCString(",", ostr);
}


}
