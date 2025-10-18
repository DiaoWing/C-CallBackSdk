
// Image_GrabDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Image_Grab.h"
#include "Image_GrabDlg.h"
#include "afxdialogex.h"
#include <afxwin.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	//���ɼ���ͼƬ��ʾ����Ӧ��λ�õĲ���
	CDialogEx::DoDataExchange(pDX);
	
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//: CDialog(CBasicDemoDlg::IDD, pParent)
CImage_GrabDlg::CImage_GrabDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMAGE_GRAB_DIALOG, pParent)
	, m_pcMyCamera(NULL)
	, m_nDeviceCombo(0)
	, m_bOpenDevice(FALSE)
	, m_bStartGrabbing(FALSE)
	, m_hGrabThread(NULL)
	, m_bThreadState(FALSE)
	, m_nTriggerMode(MV_TRIGGER_MODE_OFF)
	, m_pSaveImageBuf(NULL)
	, m_nSaveImageBufSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//memset(m_chPixelFormat, 0, MV_MAX_SYMBOLIC_LEN);
	memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
}

CImage_GrabDlg::~CImage_GrabDlg()
{
	// 删除临界区
	DeleteCriticalSection(&m_hSaveImageMux);

	// 释放图像缓冲区
	if (m_pSaveImageBuf)
	{
		free(m_pSaveImageBuf);
		m_pSaveImageBuf = NULL;
	}
}


void CImage_GrabDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_COMBO, m_ctrlDeviceCombo);
	DDX_CBIndex(pDX, IDC_DEVICE_COMBO, m_nDeviceCombo);
}

BEGIN_MESSAGE_MAP(CImage_GrabDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CImage_GrabDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CImage_GrabDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CImage_GrabDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CImage_GrabDlg::OnBnClickedButton4)

	ON_BN_CLICKED(IDC_BUTTON6, &CImage_GrabDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON5, &CImage_GrabDlg::OnBnClickedButton5)
END_MESSAGE_MAP()





// CImage_GrabDlg ��Ϣ�������
BOOL CImage_GrabDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��


	CMvCamera::InitSDK();
	DisplayWindowInitial();             // ch:��ʾ���ʼ�� | en:Display Window Initialization
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	// 初始化临界区
	InitializeCriticalSection(&m_hSaveImageMux);

	// 初始化图像缓冲区
	m_pSaveImageBuf = NULL;
	m_nSaveImageBufSize = 0;
	memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CImage_GrabDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CImage_GrabDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CImage_GrabDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



// ch:��ʾ������Ϣ | en:Show error message
void CImage_GrabDlg::ShowErrorMsg(CString csMessage, int nErrorNum)
{
	CString errorMsg;
	if (nErrorNum == 0)
	{
		errorMsg.Format(_T("%s"), csMessage);
	}
	else
	{
		errorMsg.Format(_T("%s: Error = %x: "), csMessage, nErrorNum);
	}

	switch (nErrorNum)
	{
	case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
	case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
	case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
	case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
	case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
	case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
	case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
	case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
	case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
	case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
	case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
	case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
	case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
	case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
	case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
	case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
	case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
	}

	MessageBox(errorMsg, TEXT("PROMPT"), MB_OK | MB_ICONWARNING);
}

void CImage_GrabDlg::DisplayWindowInitial()
{
	CWnd *pWnd = GetDlgItem(IDC_DISPLAY_STATIC);
	if (pWnd)
	{
		m_hwndDisplay = pWnd->GetSafeHwnd();
	}
}
// ch:�ر��豸 | en:Close Device
int CImage_GrabDlg::CloseDevice()
{
	m_bThreadState = FALSE;
	if (m_hGrabThread)
	{
		WaitForSingleObject(m_hGrabThread, INFINITE);
		CloseHandle(m_hGrabThread);
		m_hGrabThread = NULL;
	}

	if (m_pcMyCamera)
	{
		m_pcMyCamera->Close();
		delete m_pcMyCamera;
		m_pcMyCamera = NULL;
	}

	m_bStartGrabbing = FALSE;
	m_bOpenDevice = FALSE;

	if (m_pSaveImageBuf)
	{
		free(m_pSaveImageBuf);
		m_pSaveImageBuf = NULL;
	}
	m_nSaveImageBufSize = 0;

	return MV_OK;
}



void CImage_GrabDlg::OnBnClickedButton1()
{
	CString strMsg;

	m_ctrlDeviceCombo.ResetContent();
	memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

	int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);

	if (MV_OK != nRet)
	{
		return;
	}
	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
		if (NULL == pDeviceInfo)
		{
			continue;
		}

		char strUserName[256] = { 0 };
		wchar_t* pUserName = NULL;
		if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
		{
			int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
			int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
			int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
			int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

			if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
			{
				memset(strUserName, 0, 256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			else
			{
				memset(strUserName, 0, 256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName,
					pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			strMsg.Format(_T("[%d]GigE:    %s  (%d.%d.%d.%d)"), i, pUserName, nIp1, nIp2, nIp3, nIp4);
		}
		else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
		{
			if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
			{
				memset(strUserName, 0, 256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName,
					pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			else
			{
				memset(strUserName, 0, 256);
				sprintf_s(strUserName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName,
					pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
				DWORD dwLenUserName = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, NULL, 0);
				pUserName = new wchar_t[dwLenUserName];
				MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(strUserName), -1, pUserName, dwLenUserName);
			}
			strMsg.Format(_T("[%d]UsbV3:  %s"), i, pUserName);
		}
		else
		{
			ShowErrorMsg(TEXT("Unknown device enumerated"), 0);
		}
		m_ctrlDeviceCombo.AddString(strMsg);

		if (pUserName)
		{
			delete[] pUserName;
			pUserName = NULL;
		}
	}

	if (0 == m_stDevList.nDeviceNum)
	{
		ShowErrorMsg(TEXT("No device"), 0);
		return;
	}
	m_ctrlDeviceCombo.SetCurSel(0);
}


void CImage_GrabDlg::OnBnClickedButton2()
{
	if (TRUE == m_bOpenDevice || NULL != m_pcMyCamera)
	{
		return;
	}
	int nIndex = m_nDeviceCombo;
	if ((nIndex < 0) | (nIndex >= MV_MAX_DEVICE_NUM))
	{
		ShowErrorMsg(TEXT("Please select device"), 0);
		return;
	}

	// ch:���豸��Ϣ�����豸ʵ�� | en:Device instance created by device information
	if (NULL == m_stDevList.pDeviceInfo[nIndex])
	{
		ShowErrorMsg(TEXT("Device does not exist"), 0);
		return;
	}

	m_pcMyCamera = new CMvCamera;
	if (NULL == m_pcMyCamera)
	{
		return;
	}

	int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
	if (MV_OK != nRet)
	{
		delete m_pcMyCamera;
		m_pcMyCamera = NULL;
		ShowErrorMsg(TEXT("Open Fail"), nRet);
		return;
	}

	// ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
	if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
	{
		unsigned int nPacketSize = 0;
		nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
		if (nRet == MV_OK)
		{
			nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				ShowErrorMsg(TEXT("Warning: Set Packet Size fail!"), nRet);
			}
		}
		else
		{
			ShowErrorMsg(TEXT("Warning: Get Packet Size fail!"), nRet);
		}
	}

	m_bOpenDevice = TRUE;
}

void CImage_GrabDlg::OnBnClickedButton3()
{
	CloseDevice();
}

//开始采集
void CImage_GrabDlg::OnBnClickedButton4()
{
	if (FALSE == m_bOpenDevice || TRUE == m_bStartGrabbing || NULL == m_pcMyCamera)
	{
		return;
	}

	memset(&m_stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
	m_bThreadState = TRUE;
	unsigned int nThreadID = 0;
	
	int nRet = m_pcMyCamera->RegisterImageCallBack(CImage_GrabDlg::ImageCallBackEx,this);
	
	//int nRet = MV_CC_RegisterImageCallBackEx(m_pcMyCamera->outinterface(), CImage_GrabDlg::ImageCallBackEx, this);
	if (MV_OK != nRet)
	{
		return;
	}

	nRet = m_pcMyCamera->SetImageNodeNum(5);
	nRet = m_pcMyCamera->SetNetTransMode(MV_TRIGGER_MODE_OFF);

	nRet = m_pcMyCamera->StartGrabbing();
	if (MV_OK != nRet)
	{
		m_bThreadState = FALSE;
		ShowErrorMsg(TEXT("Start grabbing fail"), nRet);
		return;
	}
	m_bStartGrabbing = TRUE;
	Sleep(1000);
}


void __stdcall CImage_GrabDlg::ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	if (!pData || !pFrameInfo || !pUser)
	{
		return;
	}

	CImage_GrabDlg* pThis = (CImage_GrabDlg*)pUser;

	pThis->OnImageCallBack(pData, pFrameInfo);
}

void CImage_GrabDlg::OnImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo)
{

	// ========== 保存图像部分 ==========
	EnterCriticalSection(&m_hSaveImageMux);

	if (NULL == m_pSaveImageBuf || pFrameInfo->nFrameLen > m_nSaveImageBufSize)
	{
		if (m_pSaveImageBuf)
		{
			free(m_pSaveImageBuf);
			m_pSaveImageBuf = NULL;
		}

		m_pSaveImageBuf = (unsigned char*)malloc(pFrameInfo->nFrameLen);

		if (m_pSaveImageBuf == NULL)
		{
			LeaveCriticalSection(&m_hSaveImageMux);
			return;
		}
		m_nSaveImageBufSize = pFrameInfo->nFrameLen;
	}

	// 复制图像数据和帧信息
	memcpy(m_pSaveImageBuf, pData, pFrameInfo->nFrameLen);
	memcpy(&m_stImageInfo, pFrameInfo, sizeof(MV_FRAME_OUT_INFO_EX));

	LeaveCriticalSection(&m_hSaveImageMux);

	// ========== 显示图像部分 ==========
	if (m_hwndDisplay)
	{
		MV_CC_IMAGE stImageData = { 0 };
		stImageData.nWidth = pFrameInfo->nWidth;
		stImageData.nHeight = pFrameInfo->nHeight;
		stImageData.enPixelType = pFrameInfo->enPixelType;
		stImageData.nImageLen = pFrameInfo->nFrameLen;
		stImageData.pImageBuf = pData;

		m_pcMyCamera->DisplayOneFrame(m_hwndDisplay, &stImageData);
	}
}


void CImage_GrabDlg::OnBnClickedButton6()
{
	if (FALSE == m_bOpenDevice || FALSE == m_bStartGrabbing || NULL == m_pcMyCamera)
	{
		return;
	}

	m_bThreadState = FALSE;
	if (m_hGrabThread)
	{
		WaitForSingleObject(m_hGrabThread, INFINITE);
		CloseHandle(m_hGrabThread);
		m_hGrabThread = NULL;
	}

	int nRet = m_pcMyCamera->StopGrabbing();
	if (MV_OK != nRet)
	{
		ShowErrorMsg(TEXT("Stop grabbing fail"), nRet);
		return;
	}
	m_bStartGrabbing = FALSE;
}



void CImage_GrabDlg::OnBnClickedButton5()
{
	this->SaveImage(MV_Image_Jpeg);
}


int CImage_GrabDlg::SaveImage(MV_SAVE_IAMGE_TYPE enSaveImageType)
{
	// 检查是否有图像数据
	EnterCriticalSection(&m_hSaveImageMux);
	if (m_pSaveImageBuf == NULL)
	{
		LeaveCriticalSection(&m_hSaveImageMux);
		AfxMessageBox(_T("没有可保存的图像数据"));
		return MV_E_NODATA;
	}

	// 准备图像参数
	MV_CC_IMAGE stImage;
	memset(&stImage, 0, sizeof(MV_CC_IMAGE));
	MV_CC_SAVE_IMAGE_PARAM  stSaveImageParam;
	memset(&stSaveImageParam, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM));

	// 使用正确的字段（注意：应该是nWidth/nHeight，不是nExtendWidth/nExtendHeight）
	stImage.nWidth = m_stImageInfo.nWidth;
	stImage.nHeight = m_stImageInfo.nHeight;
	stImage.enPixelType = m_stImageInfo.enPixelType;
	stImage.pImageBuf = m_pSaveImageBuf;
	stImage.nImageLen = m_stImageInfo.nFrameLen;  // 使用nFrameLen而不是nFrameLenEx

												  // 设置保存参数
	stSaveImageParam.enImageType = enSaveImageType;
	stSaveImageParam.iMethodValue = 1;
	stSaveImageParam.nQuality = 99;

	// 选择保存文件夹
	CString strFolderPath;
	if (!SelectSaveFolder(strFolderPath))
	{
		LeaveCriticalSection(&m_hSaveImageMux);
		return 1; // 用户取消了文件夹选择
	}

	// 生成带时间戳的唯一文件名
	CString strFileName;
	CTime currentTime = CTime::GetCurrentTime();
	CString strTime = currentTime.Format(_T("%Y%m%d_%H%M%S"));

	if (MV_Image_Bmp == enSaveImageType)
	{
		strFileName.Format(_T("Image_%s_w%d_h%d.bmp"),
			strTime, m_stImageInfo.nWidth, m_stImageInfo.nHeight);
	}
	else if (MV_Image_Jpeg == enSaveImageType)
	{
		strFileName.Format(_T("Image_%s_w%d_h%d.jpg"),
			strTime, m_stImageInfo.nWidth, m_stImageInfo.nHeight);
	}
	else if (MV_Image_Tif == enSaveImageType)
	{
		strFileName.Format(_T("Image_%s_w%d_h%d.tif"),
			strTime, m_stImageInfo.nWidth, m_stImageInfo.nHeight);
	}
	else if (MV_Image_Png == enSaveImageType)
	{
		strFileName.Format(_T("Image_%s_w%d_h%d.png"),
			strTime, m_stImageInfo.nWidth, m_stImageInfo.nHeight);
	}

	// 构建完整路径
	CString strFullPath = strFolderPath + _T("\\") + strFileName;

	// 转换路径格式
	char chImagePath[MAX_PATH] = { 0 };
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, strFullPath, -1, chImagePath, MAX_PATH, NULL, NULL);
#else
	strcpy_s(chImagePath, MAX_PATH, strFullPath);
#endif

#ifdef UNICODE
	// Unicode版本直接使用宽字符
	int nRet = m_pcMyCamera->SaveImageToFile(&stImage, &stSaveImageParam, CT2A(strFullPath));
#else
	// ANSI版本
	int nRet = m_pcMyCamera->SaveImageToFile(&stImage, &stSaveImageParam, strFullPath);
#endif

	LeaveCriticalSection(&m_hSaveImageMux);

	// 显示保存结果
	if (nRet == MV_OK)
	{
		CString strMsg;
		strMsg.Format(_T("Save image success: %s"), strFileName);
		AfxMessageBox(strMsg);
	}
	else
	{
		CString strMsg;
		strMsg.Format(_T("Save image failed, error: 0x%X"), nRet);
		AfxMessageBox(strMsg);
	}

	return nRet;
}

//选择文件夹
BOOL CImage_GrabDlg::SelectSaveFolder(CString & strFolderPath)
{
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = GetSafeHwnd();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	// 设置对话框标题
	//bi.lpszTitle = _T("选择图片保存文件夹");

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl == NULL)
	{
		return FALSE; // 用户取消了选择
	}

	TCHAR szPath[MAX_PATH];
	if (SHGetPathFromIDList(pidl, szPath))
	{
		strFolderPath = szPath;
		H_File = true;               //赋值
	}

	// 释放内存
	IMalloc* pMalloc = NULL;
	if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
	{
		pMalloc->Free(pidl);
		pMalloc->Release();
	}

	return !strFolderPath.IsEmpty();
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = GetSafeHwnd();
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

	// 设置对话框标题
	//bi.lpszTitle = _T("选择图片保存文件夹");

	pidl = SHBrowseForFolder(&bi);
	if (pidl == NULL)
	{
		return FALSE; // 用户取消了选择
	}

	if (SHGetPathFromIDList(pidl, szPath))
	{

		strFolderPath = szPath;
	}

	if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)
	{
		pMalloc->Free(pidl);
		pMalloc->Release();

	}

	return !strFolderPath.IsEmpty();
}