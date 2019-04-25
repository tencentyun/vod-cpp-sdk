#include "vodapi.h"

#include "util/http_sender.h"
#include "util/codec_util.h"
//#include <algorithm>
#include <iostream>
#include <sstream>
#include<ctime>
#include "base_cmd.h"
#include "json/json.h"

#pragma comment(lib, "IPHLPAPI.lib")
#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <time.h>

namespace qcloud_vod{

std::string g_mac;


int DoVodRequest(qcloud_vod::BaseReq &req, qcloud_vod::BaseResp *resp)
{
	std::string resp_body;
	req.GenParams();
	int ret = RequestVodAPI(req.GetParam("Action"), req.GetParams(),req.GenJsonBody(),resp_body);
	if (ret!=0)
	{
		return ret;
	}
	resp->SetBody(resp_body);
	resp->ParseFromJsonString(resp_body);
	if (!resp->IsSucc())
		return -2;
	return 0;
}


int RequestVodAPI(std::string action, std::map<std::string,std::string> req_params,std::string body,std::string& resp_body)
{
	return RequestYunAPI("vod2.qcloud.com", "/v3/index.php", action, req_params,body,resp_body);
}

int RequestYunAPI(std::string host, std::string path, std::string action, std::map<std::string,std::string> req_params,std::string &body, std::string& resp_body, uint64_t connTimeoutInms, uint64_t recvTimeoutInms)
{
	std::string url = "http://" + host + path;

	std::map<std::string,std::string> req_headers, resp_headers;
	std::string err_msg;

	req_params["Action"] = action;
	req_params["Region"] = "gzp";
	
	std::stringstream stream;  
	stream << qcloud_cos::HttpSender::GetTimeStampInUs()/1000000;
	req_params["Timestamp"] = stream.str();
	stream >> req_params["Timestamp"];
	
	srand((unsigned)time(NULL));
	int nonce = rand();
	std::stringstream stream1;  
	stream1 << nonce;
	stream1 >> req_params["Nonce"];

    int http_code = qcloud_cos::HttpSender::SendRequest("GET", url, req_params, req_headers,
                                    body, connTimeoutInms, recvTimeoutInms,
                                    &resp_headers, &resp_body, &err_msg);
    if (http_code == -1) {
		printf("http_code: %d, msg: %s\n", http_code, err_msg.c_str());
		return http_code;
    }

	std::cout << "resp_body: " << resp_body <<std::endl;

	return 0;
}

int RequestReport(Json::Value jsonRoot,
				  uint64_t connTimeoutInms, uint64_t recvTimeoutInms)
{
	std::string url = "https://vodreport.qcloud.com/ugcupload";

	std::map<std::string,std::string> req_params, req_headers, resp_headers;
	std::string err_msg;

	Json::FastWriter fwriter;
	std::string req_body = fwriter.write(jsonRoot), resp_body;

	int http_code = qcloud_cos::HttpSender::SendRequest("POST", url, req_params, req_headers,
		req_body, connTimeoutInms, recvTimeoutInms,
		&resp_headers, &resp_body, &err_msg);
	if (http_code == -1) {
		//result.SetErrorInfo(err_msg);
		//return result;
		printf("http_code: %d, msg: %s\n", http_code, err_msg.c_str());
		return http_code;
	}

	std::cout << "resp_body: " << resp_body <<std::endl;

	return 0;
}

std::string GetUuid()
{
	if (g_mac!="")
		return g_mac;
	// 指定获取到的网络信息结构体链表的指针
	IP_ADAPTER_INFO *pAdapterInfo = NULL;

	// 保存获取到的网络信息结构体链表的长度
	ULONG ulOutBufLen = 0;

	// 返回调用编码
	DWORD dwRetVal;

	// 在轮循所有网络适配器信息时使用的单个结构体变量
	PIP_ADAPTER_INFO pAdapter;

	// 第1次调用GetAdaptersInfo()，获取返回结果的大小到ulOutBufLen中
	if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
	{
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
	}

	// 第2次调用GetAdaptersInfo()，获取本地网络信息到结构体pAdapterInfo中
	if((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS)
	{
		printf("GetAdaptersInfo Error! %d\n", dwRetVal);
	}

	// 从pAdapterInfo 获取并显示本地网络信息
	pAdapter = pAdapterInfo;
	while(pAdapter)
	{
		std::string mac_addr;
		for(int i=0; i<pAdapter->AddressLength; i++)
		{
			char mac_sub[4];
			if(i==(pAdapter->AddressLength -1))
				sprintf(mac_sub, "%.2X", (int)pAdapter->Address[i]);
			else
				sprintf(mac_sub, "%.2X-", (int)pAdapter->Address[i]);
			mac_addr += mac_sub;
		}

		if (strcmp(pAdapter->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
		{
			g_mac = mac_addr;
			break;
		}

		// 处理下一人网络适配器
		pAdapter = pAdapter->Next;
	}

	// 释放资源
	if(pAdapterInfo)
		free(pAdapterInfo);
	return g_mac;
}

uint64_t GetFileLastModifyTimeMs(std::string strPath)
{
	HANDLE hFile = CreateFileA(strPath.c_str(),              
		GENERIC_READ,  //必须有GENERIC_READ属性才能得到时间     
		FILE_SHARE_READ,                      
		NULL,                   
		OPEN_EXISTING,         
		FILE_ATTRIBUTE_NORMAL,
		NULL); 

	if (hFile != INVALID_HANDLE_VALUE) 
	{ 
		SYSTEMTIME sysTime;
		FILETIME fWriteTime, localTime;

		GetFileTime(hFile, NULL, NULL, &fWriteTime);//获取文件时间

		FileTimeToLocalFileTime(&fWriteTime,&localTime);//将文件时间转换为本地文件时间
		FileTimeToSystemTime(&localTime, &sysTime);//将文件时间转换为标准系统时间

		time_t clock;
		struct tm tm;
		tm.tm_year     = sysTime.wYear - 1900;
		tm.tm_mon     = sysTime.wMonth - 1;
		tm.tm_mday     = sysTime.wDay;
		tm.tm_hour     = sysTime.wHour;
		tm.tm_min     = sysTime.wMinute;
		tm.tm_sec     = sysTime.wSecond;
		tm.tm_isdst    = -1;
		clock = mktime(&tm);

		return clock*(uint64_t)1000 + sysTime.wMilliseconds;
	}
	return 0;
}

}