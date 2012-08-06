#pragma once

#include "UserInfo.hpp"
#include "UserStorage.h"
#include <string>
#define NULLPOINT			-1
#define LOGIN_OK			1
#define TEST_FALSE			2


class CUserManage
{

public:
	UserStorage *_us;
	static CUserManage* GetInstance()
	{
		return &_instance;
	}
	//����ֵ�� LOGIN_OK ��½�ɹ� TEST_FALSE ��֤ʧ�� NULLPOINT ָ����Ч
	//�����½�ɹ������û���Ϣ
	void initial(UserStorage *us)
	{
		_us = us;
	}
	int Authenticate(char* szUser, char* pPassword, userinfo* pUserInfo, int nPassLen = 40)
	{
	

		if ( !_us->IfUserExist(szUser) )//�˺��Ƿ���� �����ݿ�
		{
			return TEST_FALSE;
		}
		char strPass[40] = {0};	
		_us->GetPassword(szUser, strPass, 40);

		int i = 0; //������֤
		do
		{
			if (strPass[i] != pPassword[i])
			{
				return TEST_FALSE;
			}
			++i;
		} while (i < nPassLen);

		if ((int)pUserInfo == 0)
		{
			return NULLPOINT;
		}

		//����û���Ϣ
		_us->GetUserInfo(szUser, pUserInfo);

		return LOGIN_OK;
	}	
private:
	static CUserManage _instance;
};
