#include "apply_upload.h"

#include <iostream>

#include "json/json.h"

namespace qcloud_vod{

ApplyUploadUGCReq::ApplyUploadUGCReq(std::string &sign, std::string &typ, std::string name, int64_t size)
{
	m_sign = sign;
	m_videoType = typ;
	m_videoName = name;
	m_videoSize = size;
	m_has_cover = false;
}

void ApplyUploadUGCReq::SetCover(std::string &typ, std::string name, int64_t size)
{
	m_has_cover = true;
	m_coverType = typ;
	m_coverName = name;
	m_coverSize = size;
}
void ApplyUploadUGCReq::SetSessionKey(std::string &session)
{
	m_session_key = session;
}

int ApplyUploadUGCReq::GenParams()
{
	AddParam("Action", "ApplyUploadUGC");

	return 0;
}

std::string ApplyUploadUGCReq::GenJsonBody(){
	Json::Value jsonRoot; //定义根节点
	jsonRoot["Signature"] = m_sign;

	jsonRoot["VideoType"] = m_videoType;
	jsonRoot["VideoName"] = m_videoName;
	jsonRoot["VideoSize"] = m_videoSize;

	if (m_has_cover)
	{
		jsonRoot["CoverType"]= m_coverType;
		jsonRoot["Covername"]= m_coverName;
		jsonRoot["CoverSize"]= m_coverSize;
	}

	if (m_session_key!= "") {
		jsonRoot["VodSessionKey"] = m_session_key;
	}

	Json::FastWriter writer;
	return writer.write(jsonRoot);
}

bool ApplyUploadUGCResp::ParseFromJsonString(const std::string& body)
{
	Json::Value jsonRoot; //定义根节点

	std::string errs;
	Json::Reader reader;
	if (!reader.parse(body.data(), body.data()+body.size(), jsonRoot)) //从ifs中读取数据到jsonRoot
	{
		std::cout << errs << std::endl;
		return -1;
	}

	if (jsonRoot.isMember("code") && jsonRoot["code"].isInt()) {
		m_code = jsonRoot["code"].asInt();
	}

	if (jsonRoot.isMember("message") && jsonRoot["message"].isString()) {
		m_msg = jsonRoot["message"].asString();
	}

	if (m_code != 0)
	{
		return -2;
	}

	jsonRoot = jsonRoot["data"];

	if (jsonRoot.isMember("storageBucket") && jsonRoot["storageBucket"].isString()) {
		m_bucket = jsonRoot["storageBucket"].asString();
	}

	if (jsonRoot.isMember("storageRegion") && jsonRoot["storageRegion"].isString()) {
		m_region = jsonRoot["storageRegion"].asString();
	}

	if (jsonRoot.isMember("storageRegionV5") && jsonRoot["storageRegionV5"].isString()) {
		m_region_V5 = jsonRoot["storageRegionV5"].asString();
	}

	if (jsonRoot.isMember("vodSessionKey") && jsonRoot["vodSessionKey"].isString()) {
		m_session_key = jsonRoot["vodSessionKey"].asString();
	}

	if (jsonRoot.isMember("storageAppId") && jsonRoot["storageAppId"].isInt()) {
		m_cos_appid = jsonRoot["storageAppId"].asUInt();
	}

	if (jsonRoot.isMember("video") && jsonRoot["video"].isObject())
	{
		Json::Value jsonItem = jsonRoot["video"];
		if (jsonItem.isMember("storageSignature") && jsonItem["storageSignature"].isString()) {
			m_v_sign = jsonItem["storageSignature"].asString();
		}

		if (jsonItem.isMember("storagePath") && jsonItem["storagePath"].isString()) {
			m_v_path = jsonItem["storagePath"].asString();
		}
	}

	if (jsonRoot.isMember("cover") && jsonRoot["cover"].isObject())
	{
		Json::Value jsonItem = jsonRoot["cover"];
		if (jsonItem.isMember("storageSignature") && jsonItem["storageSignature"].isString()) {
			m_c_sign = jsonItem["storageSignature"].asString();
		}

		if (jsonItem.isMember("storagePath") && jsonItem["storagePath"].isString()) {
			m_c_path = jsonItem["storagePath"].asString();
		}
	}

	if (jsonRoot.isMember("tempCertificate") && jsonRoot["tempCertificate"].isObject())
	{
		Json::Value jsonItem = jsonRoot["tempCertificate"];
		if (jsonItem.isMember("secretId") && jsonItem["secretId"].isString()) {
			m_id = jsonItem["secretId"].asString();
		}

		if (jsonItem.isMember("secretKey") && jsonItem["secretKey"].isString()) {
			m_key = jsonItem["secretKey"].asString();
		}
		
		if (jsonItem.isMember("token") && jsonItem["token"].isString()) {
			m_token = jsonItem["token"].asString();
		}
	}

	return 0;
}

Json::Value ApplyUploadUGCResp::Report(ApplyUploadUGCReq &req)
{
	Json::Value jsonRoot;
	ApplyUploadUGCResp &resp = *this;

	jsonRoot["version"] = "1.0.0.1";       //applyupload: 10001, commitUpload: 10002
	jsonRoot["reqType"] = 10001;    //applyupload: 10001, commitUpload: 10002
	jsonRoot["errCode"] = resp.GetCode();        //请求回包得到
	jsonRoot["errMsg"] = resp.GetMsg();        //请求回包得到
	//jsonRoot["reqTimeCost"] = 123;  //请求花费时间毫秒
	jsonRoot["reqServerIp"] = "";   //请求的服务器ip
	jsonRoot["platform"] = 3000;    //平台信息（windows：3000，android：1000，ios：1000）
	jsonRoot["device"] = "pc";
	jsonRoot["osType"] = "windows";
	jsonRoot["netType"] = 5;        //wifi:1 4G:2 3G:3 2G:4 PC:5
	//jsonRoot["reqTime"] = 1111;     //请求发起的时间毫秒
	jsonRoot["reportId"] = "";
	//jsonRoot["reqKey"] = req_key;
	jsonRoot["vodSessionKey"] = ""; //准备废弃，可以不填
	jsonRoot["appId"] = resp.GetCosAppId();         
	jsonRoot["fileSize"] = req.GetFileSize();
	jsonRoot["fileType"] = req.GetFileType();
	//jsonRoot["uuid"] = uuid;          //设备唯一ID
	jsonRoot["fileId"] = "";

	return jsonRoot;
}

}