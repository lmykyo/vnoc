#include "../FrameWork/FrameBase.hpp"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

enum TestUnitID
{
	TestUnit_Begin = module_CppTest_PrivateModule_Begin,
	TestUnit_UserSession,
	TestUnit_End = module_CppTest_PrivateModule_End
};

template<typename TestUnitID TID>
class CTestBase : public CppUnit::TestFixture ,public CFrameBase
{
public:
	void setUp()
	{
		CComPtr<IFrameWork> pFrameWork;
		Global->GetIFrameModule(&pFrameWork);
		pFrameWork->RegisterModule(this);
	}
	void tearDown()
	{
		CComPtr<IFrameWork> pFrameWork;
		Global->GetIFrameModule(&pFrameWork);
		AddRef();	// ��ֹ���ü���Ϊ0������
		pFrameWork->RemoveModule(this);
	}
	CTestBase():CFrameBase((FrameModule)(UINT)TID){};
};