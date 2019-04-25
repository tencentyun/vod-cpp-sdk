#include "commit_upload.h"

#include <iostream>

#include "json/json.h"

namespace qcloud_vod{

CommitUploadUGCReq::CommitUploadUGCReq(std::string &sign)
{
	m_sign = sign;
}

int CommitUploadUGCReq::GenParams()
{
	AddParam("Action", "CommitUploadUGC");
	return 0;
}
std::string CommitUploadUGCReq::GenJsonBody(){
	Json::Value jsonRoot; //������ڵ�
	jsonRoot["Signature"] = m_sign;
	jsonRoot["VodSessionKey"] = m_session_key;

	Json::FastWriter writer;
	return writer.write(jsonRoot);
}

bool CommitUploadUGCResp::ParseFromJsonString(const std::string& body)
{
	Json::Value jsonRoot; //������ڵ�

	std::string errs;
	Json::Reader reader;
	if (!reader.parse(body.data(), body.data()+body.size(), jsonRoot, &errs)) //��ifs�ж�ȡ���ݵ�jsonRoot
	{
		std::cout << errs << std::endl;
		return -1;
	}

	if (jsonRoot.isMember("code") && jsonRoot["code"].isInt()) {
		m_code = jsonRoot["code"].asInt();
		std::cout << m_code << std::endl;
	}

	if (jsonRoot.isMember("message") && jsonRoot["message"].isString()) {
		m_msg = jsonRoot["message"].asString();
		std::cout << m_msg << std::endl;
	}

	if (m_code != 0)
	{
		return 0;
	}

	if (jsonRoot.isMember("fileId") && jsonRoot["fileId"].isString()) {
		m_file_id = jsonRoot["fileId"].asString();
		std::cout << m_file_id << std::endl;
	}

	if (jsonRoot.isMember("video") && jsonRoot["video"].isObject())
	{
		Json::Value jsonItem = jsonRoot["video"];
		if (jsonItem.isMember("url") && jsonItem["url"].isString()) {
			m_v_url = jsonItem["url"].asString();
			std::cout << m_v_url << std::endl;
		}

		if (jsonItem.isMember("storagePath") && jsonItem["storagePath"].isString()) {
			m_v_path = jsonItem["storagePath"].asString();
			std::cout << m_v_path << std::endl;
		}
	}

	if (jsonRoot.isMember("cover") && jsonRoot["cover"].isObject())
	{
		Json::Value jsonItem = jsonRoot["cover"];
		if (jsonItem.isMember("url") && jsonItem["url"].isString()) {
			m_c_url = jsonItem["url"].asString();
			std::cout << m_c_url << std::endl;
		}

		if (jsonItem.isMember("storagePath") && jsonItem["storagePath"].isString()) {
			m_c_path = jsonItem["storagePath"].asString();
			std::cout << m_c_path << std::endl;
		}
		m_has_cover = true;
	}

	return 0;
}

Json::Value CommitUploadUGCResp::Report(unsigned app_id, uint64_t file_size, std::string file_type )
{
	CommitUploadUGCResp &resp = *this;
	Json::Value jsonRoot;

	jsonRoot["version"] = "1.0.0.1";       //applyupload: 10001, commitUpload: 10002
	jsonRoot["reqType"] = 10002;    //applyupload: 10001, commitUpload: 10002
	jsonRoot["errCode"] = resp.GetCode();        //����ذ��õ�
	jsonRoot["errMsg"] = resp.GetMsg();        //����ذ��õ�
	//jsonRoot["reqTimeCost"] = 123;  //���󻨷�ʱ�����
	jsonRoot["reqServerIp"] = "";   //����ķ�����ip
	jsonRoot["platform"] = 3000;    //ƽ̨��Ϣ��windows��3000��android��1000��ios��1000��
	jsonRoot["device"] = "pc";
	jsonRoot["osType"] = "windows";
	jsonRoot["netType"] = 5;        //wifi:1 4G:2 3G:3 2G:4 PC:5
	//jsonRoot["reqTime"] = 1111;     //�������ʱ�����
	jsonRoot["reportId"] = "";
	//jsonRoot["reqKey"] = req_key;
	jsonRoot["vodSessionKey"] = ""; //׼�����������Բ���
	jsonRoot["appId"] = app_id;         
	jsonRoot["fileSize"] = file_size;
	jsonRoot["fileType"] = file_type;
	//jsonRoot["uuid"] = uuid;          //�豸ΨһID
	jsonRoot["fileId"] = resp.GetFileId();
	return jsonRoot;
}

}