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
	// ָ����ȡ����������Ϣ�ṹ�������ָ��
	IP_ADAPTER_INFO *pAdapterInfo = NULL;

	// �����ȡ����������Ϣ�ṹ������ĳ���
	ULONG ulOutBufLen = 0;

	// ���ص��ñ���
	DWORD dwRetVal;

	// ����ѭ����������������Ϣʱʹ�õĵ����ṹ�����
	PIP_ADAPTER_INFO pAdapter;

	// ��1�ε���GetAdaptersInfo()����ȡ���ؽ���Ĵ�С��ulOutBufLen��
	if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS)
	{
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
	}

	// ��2�ε���GetAdaptersInfo()����ȡ����������Ϣ���ṹ��pAdapterInfo��
	if((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS)
	{
		printf("GetAdaptersInfo Error! %d\n", dwRetVal);
	}

	// ��pAdapterInfo ��ȡ����ʾ����������Ϣ
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

		// ������һ������������
		pAdapter = pAdapter->Next;
	}

	// �ͷ���Դ
	if(pAdapterInfo)
		free(pAdapterInfo);
	return g_mac;
}

uint64_t GetFileLastModifyTimeMs(std::string strPath)
{
	HANDLE hFile = CreateFileA(strPath.c_str(),              
		GENERIC_READ,  //������GENERIC_READ���Բ��ܵõ�ʱ��     
		FILE_SHARE_READ,                      
		NULL,                   
		OPEN_EXISTING,         
		FILE_ATTRIBUTE_NORMAL,
		NULL); 

	if (hFile != INVALID_HANDLE_VALUE) 
	{ 
		SYSTEMTIME sysTime;
		FILETIME fWriteTime, localTime;

		GetFileTime(hFile, NULL, NULL, &fWriteTime);//��ȡ�ļ�ʱ��

		FileTimeToLocalFileTime(&fWriteTime,&localTime);//���ļ�ʱ��ת��Ϊ�����ļ�ʱ��
		FileTimeToSystemTime(&localTime, &sysTime);//���ļ�ʱ��ת��Ϊ��׼ϵͳʱ��

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
		CloseHandle(hFile);
		return clock*(uint64_t)1000 + sysTime.wMilliseconds;
	}
	return 0;
}

}