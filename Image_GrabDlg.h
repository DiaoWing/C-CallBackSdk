
// Image_GrabDlg.h : 头文件
//
#include "MvCamera.h"
#include <afxwin.h>
#include "MvCamera.h"
#pragma once


// CImage_GrabDlg 对话框
class CImage_GrabDlg : public CDialogEx
{
// 构造
public:
	CImage_GrabDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CImage_GrabDlg();   //删除临界区
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMAGE_GRAB_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	static void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);
	

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void OnImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo);

	// 图像数据相关****
	unsigned char* m_pSaveImageBuf;
	unsigned int m_nSaveImageBufSize;
	MV_FRAME_OUT_INFO_EX m_stImageInfo;

private:

	BOOL                    m_bSoftWareTriggerCheck;
	double                  m_dExposureEdit;
	double                  m_dGainEdit;
	double                  m_dFrameRateEdit;
	char                    m_chPixelFormat[MV_MAX_SYMBOLIC_LEN];

	CComboBox               m_ctrlDeviceCombo;                // ch:枚举到的设备 | en:Enumerated device
	int                     m_nDeviceCombo;

	
private:
	/*ch:最开始时的窗口初始化 | en:Window initialization*/
	void DisplayWindowInitial();
	int CloseDevice();                   // ch:关闭设备 | en:Close Device
	void ShowErrorMsg(CString csMessage, int nErrorNum);
private:
	BOOL                    m_bOpenDevice;                        // ch:是否打开设备 | en:Whether to open device
	BOOL                    m_bStartGrabbing;                     // ch:是否开始抓图 | en:Whether to start grabbing
	int                     m_nTriggerMode;                       // ch:触发模式 | en:Trigger Mode
	CMvCamera*              m_pcMyCamera;               // ch:CMyCamera封装了常用接口 | en:CMyCamera packed commonly used interface
	HWND                    m_hwndDisplay;                        // ch:显示句柄 | en:Display Handle
	MV_CC_DEVICE_INFO_LIST  m_stDevList;
	//MV_FRAME_OUT_INFO_EX    m_stImageInfo;

	CRITICAL_SECTION        m_hSaveImageMux;

	void*                   m_hGrabThread;              // ch:取流线程句柄 | en:Grab thread handle
	BOOL                    m_bThreadState;
public:
	afx_msg void OnBnClickedButton1();   //查找设备
	afx_msg void OnBnClickedButton2();   //打开设备
	afx_msg void OnBnClickedButton3();   //关闭设备
	afx_msg void OnBnClickedButton4();   //开始采集
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton5();

	int SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType);
	BOOL SelectSaveFolder(CString & strFolderPath);
	BOOL H_File;
};
