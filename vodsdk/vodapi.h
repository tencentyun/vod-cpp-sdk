#pragma once

#include <string>
#include <map>
#include "base_cmd.h"
#include "cos_api.h"
#include "cos_sys_config.h"

#include "json/json.h"

namespace qcloud_vod{

int RequestYunAPI(std::string host, std::string path, std::string action, std::map<std::string,std::string> req_params,std::string& body,std::string& resp_body, uint64_t connTimeoutInms=5000, uint64_t recvTimeoutInms=5000);
int RequestVodAPI(std::string action, std::map<std::string,std::string> req_params,std::string body,std::string& resp_body);

int DoVodRequest( qcloud_vod::BaseReq &req, qcloud_vod::BaseResp *resp);

int RequestReport(Json::Value jsonRoot,
				  uint64_t connTimeoutInms = 5000, uint64_t recvTimeoutInms = 5000);

std::string GetUuid();
uint64_t GetFileLastModifyTimeMs(std::string strPath);

}