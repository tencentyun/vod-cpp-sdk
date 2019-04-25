#pragma once

#include "base_cmd.h"

#include "json/json.h"

namespace qcloud_vod{

class CommitUploadUGCReq:public BaseReq
{
public:
	CommitUploadUGCReq(std::string &sign);
	int SetSessionKey(const std::string&session_key){m_session_key = session_key; return 0;}

	virtual int GenParams();
	virtual std::string GenJsonBody();

private:
	std::string m_session_key;

};

class CommitUploadUGCResp:public BaseResp
{
private:

	bool m_has_cover;
	std::string m_v_url;
	std::string m_v_path;
	std::string m_c_url;
	std::string m_c_path;

	std::string m_file_id;
public:

public:
	virtual bool ParseFromJsonString(const std::string& body);

	int GetErrInfo(std::string &errMsg) {errMsg = m_msg; return m_code;}

	std::string GetVideoPath(){return m_v_path;}
	std::string GetVideoUrl(){return m_v_url;}

	bool HasCover(){return m_has_cover;}
	std::string GetCoverPath(){return m_c_path;}
	std::string GetCoverUrl(){return m_c_url;}

	std::string GetFileId(){return m_file_id;}

	Json::Value Report(unsigned app_id, uint64_t file_size, std::string file_type );


};

}