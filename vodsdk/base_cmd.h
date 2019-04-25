#pragma once

#include "request/base_req.h"
#include "response/base_resp.h"

namespace qcloud_vod{

class BaseReq:public qcloud_cos::BaseReq
{
public:
	std::string m_sign;
public:
	virtual int GenParams() = 0;
	virtual std::string GenJsonBody() = 0;
};

class BaseResp:public qcloud_cos::BaseResp
{
protected:
	int m_code;
	std::string m_msg;

public:
	virtual bool ParseFromJsonString(const std::string& body) = 0;
	virtual bool IsSucc() {return m_code==0;}
	virtual int GetCode() {return m_code;}
	virtual std::string GetMsg() {return m_msg;}
};

}