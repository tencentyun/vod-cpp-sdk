#include "vod.h"

#include <map>

#include "boost/thread/thread.hpp"
#include <boost/bind.hpp>

#include "cos_api.h"
#include "util/file_util.h"
#include "util/http_sender.h"

#include "apply_upload.h"
#include "commit_upload.h"
#include "vodapi.h"
#include "json/json.h"

#include <windows.h>

//头文件
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

// 简单定义
typedef boost::shared_mutex Lock;                  
typedef boost::unique_lock< Lock >  WriteLock;
typedef boost::shared_lock< Lock >  ReadLock;

namespace qcloud_vod{

struct Task 
{
	int m_task_id;
	std::string m_sign; 

	std::string m_bucket,m_object,m_upload_id, m_cover_object, m_cover_upload_id;
	uint64_t m_cos_appid;
	
	std::string m_local,m_name,m_type, m_cover_local,m_cover_name,m_cover_type;
	std::string m_play_url, m_cover_url;
	uint64_t m_file_size,m_cover_size;
	uint64_t m_last_modify_time;
	std::string m_err;
	VodTaskStatus m_stat;
	boost::thread* m_t;
	qcloud_cos::CosAPI *m_cosapi;

	Poco::SharedPtr<qcloud_cos::TransferHandler> handler;
	uint64_t m_upload_size;

	std::string m_vod_session_key;

	char m_req_key[50];


	int tt;
};

std::string g_err;

std::map<int, Task*> g_tasks;
Lock g_lock;
int g_task_id = 1;

std::string g_cfg_path;
Task g_resume_task;

int (*callbackfunction)(int taskId, qcloud_vod::VodTaskStatus status, int64_t uploadSize);

void SetCallback(int(*cb)(int taskId, qcloud_vod::VodTaskStatus status, int64_t uploadSize))
{
	callbackfunction = cb;
}

int SaveResumeCfg(Task *task) ;

void uploadprogress(const qcloud_cos::MultiUploadObjectReq *req, Poco::SharedPtr<qcloud_cos::TransferHandler> &handler) {
	std::cout << "callback data is :" << handler->GetProgress() << std::endl;
	WriteLock wLock(g_lock);
	for (auto it = g_tasks.begin(); it != g_tasks.end(); it++) {
		if (handler == it->second->handler){
			it->second->m_upload_size = handler->GetProgress();
			if (callbackfunction!=NULL)	
				callbackfunction(it->first, it->second->m_stat, it->second->m_upload_size);
			break;
		}
	}
}

void statusprogress(const qcloud_cos::MultiUploadObjectReq *req, Poco::SharedPtr<qcloud_cos::TransferHandler> &handler) {
	std::cout << "callback status is :" << handler->GetStatusString() << std::endl;
	Task *task = NULL;
	{
		WriteLock wLock(g_lock);
		for (auto it = g_tasks.begin(); it != g_tasks.end(); it++) {
			if (handler == it->second->handler) {
				task = it->second;
				break;
			}
		}
	}

	if (handler->GetStatus() != qcloud_cos::TransferStatus::COMPLETED) {
		WriteLock wLock(g_lock);
		task->m_stat = VodTaskStatus::Fail;
		task->m_err = "upload cos fail \r\n" + handler->m_result.GetErrorInfo();

		task->tt = int(handler->GetStatus());

		if (callbackfunction != NULL)
			callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
		return;
	}


	{
		g_resume_task.m_upload_id = "";
		SaveResumeCfg(&g_resume_task);
		WriteLock wLock(g_lock);
		task->m_stat = VodTaskStatus::CommitUpload;
		if (callbackfunction != NULL)
			callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
	}
	// 6. vod commit
	uint64_t cmd_start_time, cmd_end_time;
	cmd_start_time = qcloud_cos::HttpSender::GetTimeStampInUs() / 1000;
	qcloud_vod::CommitUploadUGCReq c_req(task->m_sign);
	qcloud_vod::CommitUploadUGCResp c_resp;
	c_req.SetSessionKey(task->m_vod_session_key);
	int ret = qcloud_vod::DoVodRequest(c_req, &c_resp);
	cmd_end_time = qcloud_cos::HttpSender::GetTimeStampInUs() / 1000;

	Json::Value report_data = c_resp.Report(task->m_cos_appid, task->m_file_size, task->m_type);
	report_data["uuid"] = GetUuid();
	report_data["reqKey"] = task->m_req_key;
	report_data["reqTime"] = cmd_start_time;
	report_data["reqTimeCose"] = cmd_end_time - cmd_start_time;
	RequestReport(report_data);

	if (ret != 0)
	{
		WriteLock wLock(g_lock);
		task->m_stat = VodTaskStatus::Fail;
		task->m_err = "commit fail \r\n" + c_resp.GetBody();
		if (callbackfunction != NULL)
			callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
		return;
	}

	{
		WriteLock wLock(g_lock);
		task->m_play_url = c_resp.GetVideoUrl();
		task->m_cover_url = c_resp.GetCoverUrl();
		task->m_stat = VodTaskStatus::Finish;

	}
	if (callbackfunction != NULL)
		callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
}

void SyncUpload(Task * task)
{
	uint64_t task_start_time;
	uint64_t cmd_start_time, cmd_end_time;
	{
		WriteLock wLock(g_lock);
		task->m_stat = VodTaskStatus::ApplyUpload;
		if (callbackfunction != NULL)
			callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
	}
	std::string typ = task->m_local.substr(task->m_local.find_last_of('.') + 1);
	std::string cover_typ;
	// 1. vod apply
	task_start_time = cmd_start_time = qcloud_cos::HttpSender::GetTimeStampInUs() / 1000;
	sprintf(task->m_req_key, "%lld;%lld", task->m_last_modify_time, task_start_time);
	qcloud_vod::ApplyUploadUGCReq apply_req(task->m_sign, typ, task->m_name, task->m_file_size);
	if (task->m_cover_local.size())
	{
		cover_typ = task->m_cover_local.substr(task->m_cover_local.find_last_of('.') + 1);
		apply_req.SetCover(cover_typ, task->m_cover_name, task->m_cover_size);
	}
	if (g_resume_task.m_upload_id != "" &&
		g_resume_task.m_vod_session_key != "" &&
		g_resume_task.m_local == task->m_local &&
		g_resume_task.m_last_modify_time == task->m_last_modify_time) {
		apply_req.SetSessionKey(g_resume_task.m_vod_session_key);
		task->m_upload_id = g_resume_task.m_upload_id;
	}

	qcloud_vod::ApplyUploadUGCResp apply_resp;
	int ret = qcloud_vod::DoVodRequest(apply_req, &apply_resp);
	cmd_end_time = qcloud_cos::HttpSender::GetTimeStampInUs() / 1000;

	Json::Value report_data = apply_resp.Report(apply_req);
	report_data["uuid"] = GetUuid();
	report_data["reqKey"] = task->m_req_key;
	report_data["reqTime"] = cmd_start_time;
	report_data["reqTimeCose"] = cmd_end_time - cmd_start_time;
	RequestReport(report_data);


	if (ret != 0)
	{
		WriteLock wLock(g_lock);
		task->m_stat = VodTaskStatus::Fail;
		task->m_err = "apply fail \r\n" + apply_resp.GetBody();
		if (callbackfunction != NULL)
			callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
		return;
	}

	qcloud_cos::CosConfig config(apply_resp.GetCosAppId(), apply_resp.GetAccessKey(), apply_resp.GetSecretKey(),
		apply_resp.GetRegionV5(), apply_resp.GetToken());
	qcloud_cos::CosAPI *cosapi = new qcloud_cos::CosAPI(config);
	{
		WriteLock wLock(g_lock);
		task->m_bucket = apply_resp.GetBucket();
		task->m_object = apply_resp.GetVideoPath();
		task->m_cover_object = apply_resp.GetCoverPath();
		task->m_cosapi = cosapi;

		task->m_vod_session_key = apply_resp.GetSessionKey();

		if (task->m_cover_object != "")
		{
			task->m_stat = VodTaskStatus::UploadCover;
			if (callbackfunction != NULL)
				callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
		}
		else
		{
			task->m_stat = VodTaskStatus::UploadMedia;
			if (callbackfunction != NULL)
				callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
		}
	}


	qcloud_cos::CosResult result;
	if (task->m_stat == VodTaskStatus::UploadCover)
	{
		// 2. cos upload cover
		qcloud_cos::PutObjectByFileReq put_req(task->m_bucket, task->m_cover_object, task->m_cover_local);
		qcloud_cos::PutObjectByFileResp put_resp;
		result = cosapi->PutObject(put_req, &put_resp);

		{
			WriteLock wLock(g_lock);
			task->m_stat = VodTaskStatus::UploadMedia;
			if (callbackfunction != NULL)
				callbackfunction(task->m_task_id, task->m_stat, task->m_upload_size);
		}
	}

	{
		qcloud_cos::MultiUploadObjectReq req(task->m_bucket,
			task->m_object, task->m_local);
		req.SetRecvTimeoutInms(1000 * 60);
		req.SetUploadProgressCallback(uploadprogress);
		req.SetTransferStatusUpdateCallback(statusprogress);
		qcloud_cos::MultiUploadObjectResp resp;

		Poco::SharedPtr<qcloud_cos::TransferHandler> handler = cosapi->TransferUploadObject(req, &resp);
		{
			WriteLock wLock(g_lock);
			task->handler = handler;
		}
		std::string id;

		while (id == "" && !handler->IsFinishStatus(handler->GetStatus()))
		{
			id = handler->GetUploadID();
			Sleep(1000);
		}
		{
			WriteLock wLock(g_lock);
			task->m_upload_id = id;
			g_resume_task = *task;
			SaveResumeCfg(&g_resume_task);
		}
		handler->WaitUntilFinish();
		return;

	}
}


int InitConfig(std::string cfg)
{
	CosSysConfig::SetAuthExpiredTime(1800);
	g_cfg_path = cfg;
	std::ifstream is(cfg.c_str(), std::ios::in);
    if (!is || !is.is_open()) {
        std::cout << "open config file fail " << cfg << std::endl;
        return false;
    }

	is>>g_resume_task.m_local;
	is>>g_resume_task.m_last_modify_time;
	is>>g_resume_task.m_vod_session_key;
	is>>g_resume_task.m_upload_id;
	return 0;
}

int SaveResumeCfg(Task *task) 
{
	std::ofstream os(g_cfg_path.c_str(), std::ios::out);
    if (!os || !os.is_open()) {
        std::cout << "open config file fail " << g_cfg_path << std::endl;
		g_err = "open config file fail";
        return false;
    }
	g_err = "open config file succ" + g_cfg_path;

	os<<task->m_local<<std::endl
		<<task->m_last_modify_time<<std::endl
		<<task->m_vod_session_key<<std::endl
		<<task->m_upload_id<<std::endl;

	return true;
}

int StartTask(std::string local_path, std::string name,
	std::string cover_local_path, std::string cover_name, std::string procedure, std::string sign)
{
	Task *task = new Task;
	task->m_upload_size = 0;
	task->m_task_id = g_task_id;
	task->m_local = local_path;
	task->m_cover_local = cover_local_path;
	task->m_name = name;
	task->m_cover_name = cover_name;
	task->m_sign = sign;

	task->m_file_size = qcloud_cos::FileUtil::GetFileLen(task->m_local);
	task->m_cover_size = qcloud_cos::FileUtil::GetFileLen(task->m_cover_local);
	if (task->m_local.find_last_of('.')==std::string::npos ||
		task->m_cover_local.size() && task->m_cover_local.find_last_of('.')==std::string::npos)
	{
		task->m_stat = VodTaskStatus::Fail;
		task->m_err = "file path not have ext";
		return -1;
	}

	task->m_last_modify_time = GetFileLastModifyTimeMs(task->m_local);

	task->m_t = new boost::thread(boost::bind(SyncUpload, task));
	task->m_cosapi = NULL;

	WriteLock wLock(g_lock);
	g_tasks[g_task_id] = task;

	return g_task_id++;
}

int DeleteTask(int task_id)
{
	if (g_tasks.count(task_id) == 1)
	{
		Task* task = g_tasks[task_id];
		task->m_t->join();
		WriteLock wLock(g_lock);
		task->handler = NULL;
		delete task->m_t;
		delete task->m_cosapi;
		delete task;
		g_tasks.erase(task_id);
		return 0;
	}
	return -1;
}

int AbortTask(int task_id)
{
	if (g_tasks.count(task_id) == 1)
	{
		Task* task= g_tasks[task_id];
		qcloud_cos::AbortMultiUploadReq req(task->m_bucket,task->m_object,task->m_upload_id);
		qcloud_cos::AbortMultiUploadResp resp;
		qcloud_cos::CosResult 	result = task->m_cosapi->AbortMultiUpload(req, &resp);
		if (!result.IsSucc()) {
			//task->m_stat = "fail";
			//task->m_err = "complete cos fail 11 \r\n" + result.DebugString();
			return -1;
		}

		return 0;
	}
	return -1;
}

int PauseTask(int task_id)
{
	if (g_tasks.count(task_id) == 1)
	{
		Task* task= g_tasks[task_id];
		if (!task->handler.isNull()){
			WriteLock wLock(g_lock);
			task->handler->Cancel();
		}
		return 0;
	}
	return 0;
}

uint64_t GetUploadedSize(int task_id)
{
	ReadLock rLock(g_lock);
	if (g_tasks.count(task_id) == 1)
	{
		Task *task = g_tasks[task_id];
		if (task->m_stat == VodTaskStatus::Finish)
			return task->m_file_size;
		return task->m_upload_size;
		if (task->m_upload_id=="" || task->m_cosapi== NULL)
			return 0;
		if (!task->handler.isNull()){
				return task->handler->GetProgress();
		}
	}
	return 0;
}
uint64_t GetFileSize(int task_id)
{
	ReadLock rLock(g_lock);
	if (g_tasks.count(task_id) == 1)
	{
		Task *task = g_tasks[task_id];
		return task->m_file_size;
	}
	return 0;
}
std::string GetShowInfo(int task_id)
{
	ReadLock rLock(g_lock);
	if (g_tasks.count(task_id) == 1)
	{
		Task *task = g_tasks[task_id];
		char a[100] = { 0 };
		std::string szMsg = "bucket: " + task->m_bucket +
			"\r\nlocal: " + task->m_local +
			"\r\ncos_path: " + task->m_object +
			"\r\nupload_id: " + task->m_upload_id +
			"\r\nstat: " + itoa(task->m_stat, a,10) +
			"\r\nerr: " + task->m_err +
			"\r\ntt: " +itoa(task->tt, a, 10) +
			"\r\ng_err: " + g_err;
		return szMsg;
	}
	return "";
}
VodTaskStatus GetTaskStatus(int task_id)
{
	ReadLock rLock(g_lock);
	if (g_tasks.count(task_id) == 1)
	{
		Task *task = g_tasks[task_id];
		return task->m_stat;
	}
	return VodTaskStatus::NotExist;
}
std::string GetPlayUrl(int task_id)
{
	ReadLock rLock(g_lock);
	if (g_tasks.count(task_id) == 1)
	{
		Task *task = g_tasks[task_id];
		return task->m_play_url;
	}
	return "";
}
std::string GetCoverUrl(int task_id)
{
	ReadLock rLock(g_lock);
	if (g_tasks.count(task_id) == 1)
	{
		Task *task = g_tasks[task_id];
		return task->m_cover_url;
	}
	return "";
}

uint64_t SetUploadPartSize(uint64_t partsize)
{
	qcloud_cos::CosSysConfig::SetUploadPartSize(partsize);
	return 0;
}

}