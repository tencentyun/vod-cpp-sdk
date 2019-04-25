#pragma once

#include "base_cmd.h"
#include "json/json.h"

namespace qcloud_vod{

class ApplyUploadUGCReq:public BaseReq
{
public:
	ApplyUploadUGCReq(std::string &sign, std::string &typ, std::string name, int64_t size);

	void SetCover(std::string &typ, std::string name, int64_t size);
	void SetSessionKey(std::string &session);
	uint64_t GetFileSize(){return m_videoSize;}
	std::string GetFileType(){return m_videoType;}

	virtual int GenParams();
	virtual std::string GenJsonBody();

private:
	std::string m_videoType;
	std::string m_videoName;
	int64_t m_videoSize;

	bool m_has_cover;
	std::string m_coverType;
	std::string m_coverName;
	int64_t m_coverSize;

	std::string m_session_key;

};

class ApplyUploadUGCResp:public BaseResp
{
private:

	bool m_has_cover;
	std::string m_v_sign;
	std::string m_v_path;
	std::string m_c_sign;
	std::string m_c_path;

	unsigned int m_cos_appid;
	std::string m_bucket;
	std::string m_region;
	std::string m_region_V5;
	std::string m_session_key;

	std::string m_id;
	std::string m_key,m_token;

public:
	virtual bool ParseFromJsonString(const std::string& body);

	int GetErrInfo(std::string &errMsg) {errMsg = m_msg; return m_code;}
	
	std::string GetVideoPath(){return m_v_path;}
	std::string GetVideoSign(){return m_v_sign;}
	
	bool HasCover(){return m_has_cover;}
	std::string GetCoverPath(){return m_c_path;}
	std::string GetCoverSign(){return m_c_sign;}

	unsigned GetCosAppId(){return m_cos_appid;}
	std::string GetBucket(){return m_bucket;}
	std::string GetRegion(){return m_region;}
	std::string GetRegionV5(){return m_region_V5;}
	std::string GetSessionKey(){return m_session_key;}

	std::string GetAccessKey(){return m_id;}
	std::string GetSecretKey(){return m_key;}
	std::string GetToken(){return m_token;}


	Json::Value Report(ApplyUploadUGCReq &req);

};

}