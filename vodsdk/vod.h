#pragma once

#include <string>
#include <stdint.h>

namespace qcloud_vod{

enum VodTaskStatus {
	NotStart,
	ApplyUpload,
	UploadCover,
	UploadMedia,
	CommitUpload,
	Finish,
	Fail,
	NotExist
};

//初始化函数，断点续传信息保存
int InitConfig(std::string cfg_path);

//设置回调通知函数
void SetCallback(int(*cb)(int taskId, qcloud_vod::VodTaskStatus status, int64_t uploadSize));

//设置分片上传的分片大小
uint64_t SetUploadPartSize(uint64_t partsize);   

//发起上传任务
//参数列表:
//local_path:本地视频路径；
//name:上传后的视频名称;
//cover_local_path:本地封面路径；
//cover_name:封面名称，目前没用;
//procedure:上传后对视频执行的任务流，可以转码、截图等;
//sign：从用户签名服务器获取的上传签名;
int StartTask(std::string local_path, std::string name,
			  std::string cover_local_path, std::string cover_name, std::string procedure, std::string sign);  


// 中断上传，执行后不可续传，可以查询任务信息
int AbortTask(int task_id);

// 停止上传，后面可以续传，可以查询任务信息
int PauseTask(int task_id);

// 删除任务，删除后查不到任务信息
int DeleteTask(int task_id);

// 获取已经上传的大小和文件总大小，可以用来展示，计算进度
uint64_t GetUploadedSize(int task_id);
uint64_t GetFileSize(int task_id);



// 查询任务状态
VodTaskStatus GetTaskStatus(int task_id);

// 用于demo展示任务调试信息
std::string GetShowInfo(int task_id);

// 任务成功后获取视频和封面的url
std::string GetPlayUrl(int task_id);
std::string GetCoverUrl(int task_id);

}