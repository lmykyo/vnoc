#pragma once

#define PRODUCT_NAME	_T("VNOC")
#define PathSplit		_T('\\')

#define CONFIGXMLFILE	_T("config.xml")

#define _LogFileExt		_T(".log")
#define _LogFileMaxSizeBytes	64*1024*1024
#define _LogFormatBufferSize	1024
#define LogMask_ALL		0xffffffff

//////////////////////////////////////////////////////////////////////////
// log���

#define LogFile_Config	_T("Config")

class CGlobalUtilBase
{
public:
	CString m_file;
	CString	m_prefix;
};
