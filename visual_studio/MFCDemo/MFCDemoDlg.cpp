
// MFCDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCDemo.h"
#include "MFCDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CMFCDemoDlg 对话框



CMFCDemoDlg::CMFCDemoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCDemoDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCDemoDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCDemoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCDemoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMFCDemoDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON6, &CMFCDemoDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CMFCDemoDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDOK, &CMFCDemoDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMFCDemoDlg 消息处理程序

BOOL CMFCDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCDemoDlg::OnPaint()
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
HCURSOR CMFCDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCDemoDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	if (swift.connected) {
		AfxMessageBox(_T("已连接，无需重复连接"));
		return;
	}
	swift.connect();
	if (!swift.connected) {
		AfxMessageBox(_T("连接失败"));
		return;
	}

	if (swift.get_power_status() <= 0) {
		sleep_milliseconds(500);
	}

	int count = 0;
	while (!swift.power_status) {
		count += 1;
		int value = swift.get_power_status();
		if (!swift.power_status) {
			sleep_milliseconds(500);
		}
		if (count > 60 && !swift.power_status) {
			AfxMessageBox(_T("连接失败"));
			return;
		}
	}
	AfxMessageBox(_T("连接成功"));
}

void CMFCDemoDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	if (swift.connected) {
		swift.disconnect();
		AfxMessageBox(_T("已断开"));
	}
	else {
		AfxMessageBox(_T("未连接"));
	}
}


void CMFCDemoDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	if (swift.connected) {
		swift.reset();
	}
	else {
		AfxMessageBox(_T("未连接"));
	}
}


void CMFCDemoDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	if (swift.connected) {
		swift.set_position(250, 50, 120, 1000000);
		swift.set_position(250, -50, 120, 1000000);
	}
	else {
		AfxMessageBox(_T("未连接"));
	}
}


void CMFCDemoDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
	if (swift.connected) {
		swift.set_pump(true);
	}
	else {
		AfxMessageBox(_T("未连接"));
	}
}


void CMFCDemoDlg::OnBnClickedButton7()
{
	// TODO: 在此添加控件通知处理程序代码
	if (swift.connected) {
		swift.set_pump(false);
	}
	else {
		AfxMessageBox(_T("未连接"));
	}
}


void CMFCDemoDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}
