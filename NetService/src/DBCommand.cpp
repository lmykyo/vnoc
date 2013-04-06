#include "DBCommand.h"
#include "DBConnection.h"
#include "DBDefine.h"
#include <ezlogger_headers.hpp>
#include <sqlite3/sqlite3.h>
#include "DBField.hpp"
#include "DBRecordset.hpp"

DBCommand::DBCommand()
{
    cleanup();
}

DBCommand::DBCommand(DBConnection* conn) 
    : _conn(conn)
{
}

DBCommand::DBCommand(DBConnection* conn, const DBString& commandText)
    : _conn(conn)
{
    setCommandText(commandText);
}

DBCommand::~DBCommand()
{
    cleanup();
}

void DBCommand::cleanup()
{
    _conn = NULL;
    _stmt = NULL;
    resetParamBindPos();
}

void DBCommand::setConnection(DBConnection* conn)
{
    _conn = conn;
}

bool DBCommand::setCommandText(const DBString& commandText)
{
#ifdef DB_UTF16
    int32 result = sqlite3_prepare16_v2(_conn, commandText.c_str(), -1, &_stmt, NULL);
#else
    int32 result = sqlite3_prepare_v2(_conn->connection(), commandText.c_str(), -1, &_stmt, NULL);
#endif

    if (result != SQLITE_OK)
    {
        return false;
    }

    _commandText = commandText;
    resetParamBindPos();
    return true;
}

DBCommand& DBCommand::operator << (const DBString& value)
{
#ifdef DB_UTF16
    int32 result = sqlite3_bind_text16(_stmt, _curIndex, value, -1, SQLITE_TRANSIENT);
#else
    int32 result = sqlite3_bind_text(_stmt, _paramBindPos, value.c_str(), -1, SQLITE_TRANSIENT);
#endif

    if (result != SQLITE_OK)
    {
        cout << "sqlite3_bind_text(16) failed." << endl;
    }

    _paramBindPos++;
    return *this;
}

DBCommand& DBCommand::operator << (int32 value)
{
    int32 result = sqlite3_bind_int(_stmt, _paramBindPos, value);
    if (result != SQLITE_OK)
    {
        throw "sqlite3_bind_int failed.";
    }

    _paramBindPos++;
    return *this;
}

DBCommand& DBCommand::operator << (int64 value)
{
    int32 result = sqlite3_bind_int64(_stmt, _paramBindPos, value);
    if (result != SQLITE_OK)
    {
        throw "sqlite3_bind_int64 failed.";
    }

    _paramBindPos++;
    return *this;
}

DBCommand& DBCommand::operator << (uint32 value)
{
    int32 result = sqlite3_bind_int(_stmt, _paramBindPos, value);
    if (result != SQLITE_OK)
    {
        throw "sqlite3_bind_int failed.";
    }

    _paramBindPos++;
    return *this;
}

DBCommand& DBCommand::operator << (uint64 value)
{
    int32 result = sqlite3_bind_int64(_stmt, _paramBindPos, value);
    if (result != SQLITE_OK)
    {
        throw "sqlite3_bind_int64 failed.";
    }

    _paramBindPos++;
    return *this;
}

DBCommand& DBCommand::operator << (double value)
{
    int32 result = sqlite3_bind_double(_stmt, _paramBindPos, value);
    if (result != SQLITE_OK)
    {
        throw "sqlite3_bind_double failed.";
    }

    _paramBindPos++;
    return *this;
}

DBCommand& DBCommand::operator << (Blob* blob)
{
    int32 result = sqlite3_bind_blob(_stmt, _paramBindPos, blob->blob, blob->size, NULL);
    if (result != SQLITE_OK)
    {
        throw "sqlite3_bind_blob failed.";
    }

    _paramBindPos++;
    return *this;
}

/*
DBCommand& DBCommand::operator [] (int32 filedIndex)
{

}


DBCommand& DBCommand::operator [] (const DBString& filedName)
{

}*/

bool DBCommand::query()
{
    _dbRecordset.recordset().clear();
    while(sqlite3_step(_stmt) == SQLITE_ROW)
    {
        int32 colCount = sqlite3_column_count(_stmt);
        
        Record record;
        for (int32 i = 0; i < colCount; ++i)
        {
#ifdef DB_UTF16    
            DBString fieldName = sqlite3_column_name16(_stmt, i);  
#else   
            DBString fieldName = sqlite3_column_name(_stmt, i);  
#endif

            DBField field(_stmt);
            field._fieldData.field_name = fieldName;
            field._fieldData.pos = i;

            record.StringKeySet.insert(std::make_pair(fieldName, &field));
            record.IntKeySet.insert(std::make_pair(i, &field));
        }

        _dbRecordset.recordset().push_back(record);
    }

    EZLOGGERVLSTREAM(axter::log_often) << "Query operation Done, " << rowsAffected() << " rows fetched." << endl;
    _isResultSet = true;
    return true;
}

bool DBCommand::query(const DBString& commandText)
{
    if (setCommandText(commandText) == false)
    {
        return false;
    }

    return query();
}

bool DBCommand::execute()
{
    int result = sqlite3_step(_stmt);
    if (result != SQLITE_OK)
    {
        return false;
    }

    _isResultSet = false;
    return (sqlite3_finalize(_stmt) == SQLITE_OK) ? true : false;
}

bool DBCommand::execute(const DBString& commandText)
{
    if (setCommandText(commandText) == false)
    {
        return false;
    }

    return execute();
}

int DBCommand::rowsAffected() const
{
    return sqlite3_changes(_conn->connection());
}

bool DBCommand::isResultSet() const
{
    return _isResultSet;
}