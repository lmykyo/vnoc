#include "DBCommand.h"
#include "DBConnection.h"
#include "DBDefine.h"
#include <ezlogger_headers.hpp>
#include <sqlite3/sqlite3.h>
#include "DBFieldReader.hpp"

DBCommand::DBCommand()
{
    cleanup();
}

DBCommand::DBCommand(DBConnection* conn) 
{
    cleanup();
    setConnection(conn);
}

DBCommand::DBCommand(DBConnection* conn, const DBString& commandText)
{
    cleanup();
    setConnection(conn);
    setCommandText(commandText);
}

DBCommand::~DBCommand()
{
    sqlite3_finalize(_stmt);
    SAFE_DELETE(_fieldReader);
    cleanup();
}

void DBCommand::cleanup()
{
    _conn = NULL;
    _stmt = NULL;
    _recordCount = 0;
    _isResultSet = false;
    _fieldReader = NULL;
    resetParamBindPos();
}

void DBCommand::setConnection(DBConnection* conn)
{
    _conn = conn;
}

bool DBCommand::setCommandText(const DBString& commandText)
{
#ifdef DB_UTF16
    int32 result = sqlite3_prepare16_v2(_conn->connection(), commandText.c_str(), -1, &_stmt, NULL);
#else
    int32 result = sqlite3_prepare_v2(_conn->connection(), commandText.c_str(), -1, &_stmt, NULL);
#endif

    ToT << "stmt = " << _stmt << endl;

    if (result != SQLITE_OK)
    {
        return false;
    }

    SAFE_DELETE(_fieldReader);
    _commandText = commandText;
    return true;
}

bool DBCommand::fetchNext()
{
    while (sqlite3_step(_stmt) == SQLITE_ROW)
    {
        return true;
    }

    return false;
}

bool DBCommand::fetchFirst()
{
    int result = sqlite3_reset(_stmt);
    if (result == SQLITE_ROW)
    {
        return true;
    }

    return false;
}

bool DBCommand::fetchLast()
{
    while (sqlite3_step(_stmt) == SQLITE_DONE)
    {
        return true;
    }

    return false;
}

DBCommand& DBCommand::operator << (const DBString& value)
{
#ifdef DB_UTF16
    int32 result = sqlite3_bind_text16(_stmt, _paramBindPos, value.c_str(), -1, SQLITE_TRANSIENT);
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
        throw;
    }

    _paramBindPos++;
    return *this;
}


/*DBCommand& DBCommand::operator << (bool value)
{
    int32 result = sqlite3_bind_int(_stmt, _paramBindPos, value == 0 ? value : 1);
    if (result != SQLITE_OK)
    {
        throw;
    }

    _paramBindPos++;
    return *this;
}*/

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

const DBFieldReader& DBCommand::operator[](const DBString& fieldName)
{
    FieldKey_t index = _fieldTable[fieldName];
    _fieldReader->setReaderStmt(_stmt, index);
    return *_fieldReader;
}

const DBFieldReader& DBCommand::operator[](int32 fieldIndex)
{
    _fieldReader->setReaderStmt(_stmt, fieldIndex);
    return *_fieldReader;
}

bool DBCommand::query()
{
    //获取列数
    int32 colCount = sqlite3_column_count(_stmt);

    //获取所有列名
    _fieldTable.clear();
    for (int32 i = 0; i < colCount; ++i)
    {
        FieldKey_t fieldIndex = i;

#ifdef DB_UTF16
        DBString fieldName = (const wchar_t*)sqlite3_column_name16(_stmt, fieldIndex);
#else
        DBString fieldName = sqlite3_column_name(_stmt, fieldIndex);
#endif

        _fieldTable.insert(std::make_pair(fieldName, fieldIndex));
    }

    //遍历一次，得到返回的记录数
    _recordCount = 0;
    while (sqlite3_step(_stmt) == SQLITE_ROW)
    {
        _recordCount++;
    }
    
    fetchFirst();

    _fieldReader = new DBFieldReader();

    cout << "Query executed, " << recordCount() << " records." << endl;
    _isResultSet = true;
    resetParamBindPos();
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
    if (result != SQLITE_OK && result != SQLITE_DONE)
    {
        return false;
    }

    _isResultSet = false;
    resetParamBindPos();
    return true;
}

bool DBCommand::execute(const DBString& commandText)
{
    if (setCommandText(commandText) == false)
    {
        return false;
    }

    return execute();
}

uint32 DBCommand::rowsAffected() const
{
    return sqlite3_changes(_conn->connection());
}

uint32 DBCommand::recordCount() const
{
    return _recordCount;
}

bool DBCommand::isResultSet() const
{
    return _isResultSet;
}

DBConnection& DBCommand::connection() const
{
    return *_conn;
}