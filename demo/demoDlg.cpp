
// demoDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "demo.h"
#include "demoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include "vod.h"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CdemoDlg 对话框



CdemoDlg::CdemoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CdemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CdemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_VIDEO, &CdemoDlg::OnBnClickedButtonVideo)
	ON_BN_CLICKED(IDC_BUTTON_COVER, &CdemoDlg::OnBnClickedButtonCover)
	ON_BN_CLICKED(IDC_BUTTON_START, &CdemoDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CdemoDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CdemoDlg 消息处理程序

BOOL CdemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	qcloud_vod::InitConfig("D:\\code_base\\cos-cpp-sdk-v5\\2008_new\\Release\\config.cfg");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CdemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CdemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CdemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CdemoDlg::OnBnClickedButtonVideo()
{
	// TODO: 在此添加控件通知处理程序代码

	TCHAR szFilter[] = _T("mp4|*.mp4|flv|*.flv|all|*.*");   //可供选择的后缀  
	// 构造保存文件对话框     
	//第1个参数false是保存文件，true是打开文件。第2个参数是默认后缀，第3个参数是默认文件名  
	CFileDialog fileDlg(TRUE, _T("mp4"), _T("video"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString strFilePath;   //文件路径  

	// 显示保存文件对话框     
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里     
		strFilePath = fileDlg.GetPathName();
		SetDlgItemText(IDC_EDIT_VIDEO, strFilePath);
	}
}


void CdemoDlg::OnBnClickedButtonCover()
{
	// TODO: 在此添加控件通知处理程序代码

	TCHAR szFilter[] = _T("jpg|*.jpg|bmp|*.bmp|all|*.*");   //可供选择的后缀  
// 构造保存文件对话框     
//第1个参数false是保存文件，true是打开文件。第2个参数是默认后缀，第3个参数是默认文件名  
	CFileDialog fileDlg(TRUE, _T("jpg"), _T("picture"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString strFilePath;   //文件路径  

	// 显示保存文件对话框     
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里     
		strFilePath = fileDlg.GetPathName();
		SetDlgItemText(IDC_EDIT_COVER, strFilePath);
	}
}


void CdemoDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码

	SetTimer(1, 1000, NULL);
	SetDlgItemText(IDC_EDIT_INFO, _T("正在上传 0%..."));
	CProgressCtrl *pProgCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS1);
	pProgCtrl->SetRange(0, 100);
	pProgCtrl->SetStep(1);
	pProgCtrl->SetPos(0);

	CString local_path, c_local_path;
	GetDlgItemText(IDC_EDIT_VIDEO, local_path);
	GetDlgItemText(IDC_EDIT_COVER, c_local_path);

	//m_upload_id = UploadAysn(&local_path);
	std::string p = CT2A(local_path);
	std::string cp = CT2A(c_local_path);
	std::string sign = "DJUqxM8/2V8+7PlUsxbyAi61HVBzZWNyZXRJZD1BS0lEMHBtbHIyMkNLY3IwQ2l0OVo2a011V0dvS1BWVlpYZ3cmY3VycmVudFRpbWVTdGFtcD0xNTU1NTU3MDkxJmV4cGlyZVRpbWU9MTU1NTY0MzQ5MSZyYW5kb209OTk0NzMzMzkw";
	m_task_id = qcloud_vod::StartTask(p, "aa.mp4", cp, "bb.png", "", sign);

	//upload(local_path, NULL);

	//_beginthread(WorkProc,0,this->GetSafeHwnd());

	((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(FALSE);
}


void CdemoDlg::OnBnClickedButtonStop()
{
	// TODO: 在此添加控件通知处理程序代码

	SetDlgItemText(IDC_EDIT_INFO, _T("停止上传..."));

	((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(TRUE);
	qcloud_vod::PauseTask(m_task_id);
	//qcloud_vod::DeleteTask(m_task_id);
}


void CdemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CProgressCtrl *pProgCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS1);
	int cur = 0;
	CString cszMsg;

	std::string status = qcloud_vod::GetTaskStatus(m_task_id);
	if (status == "fail")
	{
		cszMsg = CString(_T("上传失败...\r\n"));
		cszMsg += qcloud_vod::GetShowInfo(m_task_id).c_str();
		((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(TRUE);
		qcloud_vod::DeleteTask(m_task_id);
		KillTimer(1);
	}
	else
	{
		uint64_t up = qcloud_vod::GetUploadedSize(m_task_id);
		uint64_t total = qcloud_vod::GetFileSize(m_task_id);
		if (total == 0)
			cur = 0;
		else cur = up * 100 / total;
		if (status=="finish")
		{
			cur = 100;
			cszMsg = CString(_T("上传完成...\r\n"));
			cszMsg += qcloud_vod::GetShowInfo(m_task_id).c_str();
			((CButton*)GetDlgItem(IDC_BUTTON_STOP))->EnableWindow(FALSE);
			((CButton*)GetDlgItem(IDC_BUTTON_START))->EnableWindow(TRUE);
			qcloud_vod::DeleteTask(m_task_id);
			KillTimer(1);
		}
		else
		{
			TCHAR szMsg[500];
			sprintf_s(szMsg, _T("正在上传 %d%%(%lld/%lld)...\r\n"), cur, up, total);
			cszMsg = CString(szMsg);
			cszMsg += qcloud_vod::GetShowInfo(m_task_id).c_str();
		}
	}

	pProgCtrl->SetPos(cur);
	SetDlgItemText(IDC_EDIT_INFO, cszMsg);

	CDialogEx::OnTimer(nIDEvent);
}
