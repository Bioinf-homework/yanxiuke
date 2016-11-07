
// MFCApplication1Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MFCApplication1.h"
#include "MFCApplication1Dlg.h"
#include "afxdialogex.h"
#include "mmsystem.h"
#include "windows.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, WORD dwInstance,
	WAVEHDR* dwParam1, DWORD dwParam2);
void closewave();
void startwork();



#define MAXWAVEBLOCK  100//输入内存块数
#define MAXWAVEBUFFER 320//每块内存大小 20ms


static WAVEHDR WaveHdrIn[MAXWAVEBLOCK];  //
static char WaveInBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//记录录音数据用内存块
static HWAVEIN hWaveIn;

//new code here
#define SLIDEWINDOWLEN  500//滑动窗长度，为帧数
#define ENERGEONLY 1//如果为1表示仅用能量特征，否则用能量特征和过零率特征
#define ENERGETHRESHOLD  4000*160//能量门限
#define MATCHTHRESHOLD  5.3//匹配距离门限

bool m_record_flag;          //当前录音状态，false-停止录音  true-正在录音
bool m_recognition_flag;     //当前识别状态,false-停止识别，true-正在识别
bool m_trigger_flag;     //当前触发状态，0-未触发，1--触发

typedef struct{
	float m_EN[SLIDEWINDOWLEN];//模板的能量特征存储区
	float m_ZC[SLIDEWINDOWLEN];//模板的过零率特征存储区	
	short m_cur_pos;//当前写入位置
} MyTemplate;

int m_pre_en;//前帧能量
int m_pre_zc;//前帧过零率
MyTemplate m_template;  //模板数据
MyTemplate m_slide_data;//滑动窗数据

void trigger(int en,int zc);
void record(int en,int zc,short *data);
void recognition(int en,int zc,short *data);
void write_template_file();
void load_template_file();
void init_template_data();
int get_en(short *sound);
int get_zc(short *sound);
bool match();

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 对话框



CMFCApplication1Dlg::CMFCApplication1Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMFCApplication1Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCApplication1Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCApplication1Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCApplication1Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCApplication1Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCApplication1Dlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CMFCApplication1Dlg 消息处理程序

BOOL CMFCApplication1Dlg::OnInitDialog()
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

	// TODO:  在此添加额外的初始化代码
	m_record_flag = 0;
	m_recognition_flag = 0;
	m_trigger_flag = 0;

	startwork();
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCApplication1Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CMFCApplication1Dlg::OnPaint()
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
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication1Dlg::OnBnClickedButton1()
{
	// TODO:  在此添加控件通知处理程序代码
	
	m_trigger_flag = 0;
	init_template_data();
	m_record_flag = 1;
}




void startwork()
{

	int sta;

	int i;

	//初始化录音设备
	WAVEFORMATEX waveFormatEx;
	waveFormatEx.cbSize = 0;
	waveFormatEx.nAvgBytesPerSec = 16000;
	waveFormatEx.nBlockAlign = 2;
	waveFormatEx.nChannels = 1;
	waveFormatEx.nSamplesPerSec = 8000;
	waveFormatEx.wBitsPerSample = 16;
	waveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	hWaveIn = NULL;
	if (sta = waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormatEx, (unsigned long)waveInProc, 1L, CALLBACK_FUNCTION))
	{
		::MessageBox(0, L"声卡设置失败", L"信息窗口", 0);
		return;//录音设备打开失败，可能声卡不支持全双工。
	}


	for (i = 0; i < MAXWAVEBLOCK; i++)
	{//增加录音内存
		WaveHdrIn[i].lpData = WaveInBuffer[i];
		WaveHdrIn[i].dwBufferLength = MAXWAVEBUFFER;
		WaveHdrIn[i].dwFlags = 0;
		WaveHdrIn[i].dwUser = 0;
		if (waveInPrepareHeader(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))
		{//无法锁定该内存
			closewave();
			::MessageBox(0, L"声卡设置失败", L"信息窗口", 0);
			return;
		};
		if (waveInAddBuffer(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))
		{//无法增加该内存
			closewave();
			::MessageBox(0, L"声卡设置失败", L"信息窗口", 0);
			return;
		}
	}
	//打开声卡
	waveInStart(hWaveIn);


}
void closewave()
{

	if (hWaveIn){
		waveInStop(hWaveIn);//停止录音
		waveInReset(hWaveIn);//清空录音缓存
		waveInClose(hWaveIn);//关闭录音设备
		hWaveIn = NULL;

	}
}



static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, WORD dwInstance,
	WAVEHDR* dwParam1, DWORD dwParam2)
{

	short buffer[MAXWAVEBUFFER/2];
	long buf_len;

	int l_en;
	int l_zc;

	switch (uMsg)
	{//判断回调类型
	case WIM_OPEN:
		//录音设备正常打开时，触发该事件
		//MessageBox(NULL,"Open success", "提示", MB_OK);
		break;
	case WIM_CLOSE:
		//录音设备正常关闭时，触发该事件
		//MessageBox(NULL,"Close success", "提示", MB_OK);
		break;
	case WIM_DATA:
		
		
		//录音设备返回数据时，触发该事件
		if (!waveInUnprepareHeader(hwi, dwParam1, sizeof(WAVEHDR)))
		{//可以释放内存。有的情况虽然返回数据，但不能释放内存，
			//如使用同一个WAVEHDR增加多个内存块时，会出现上述情况

			buf_len = dwParam1->dwBytesRecorded;//记录录音数据长度
			memcpy((void *)buffer, dwParam1->lpData, buf_len);//复制返回数据
			
			dwParam1->dwUser = 0;//恢复设置
			dwParam1->dwBufferLength = MAXWAVEBUFFER;
			dwParam1->dwFlags = 0;
			waveInPrepareHeader(hwi, dwParam1, sizeof(WAVEHDR));
			waveInAddBuffer(hwi, dwParam1, sizeof(WAVEHDR));

			//new code here
			if (m_record_flag)
			{
				l_en = get_en(buffer);
				l_zc = get_zc(buffer);
				if (m_trigger_flag)
					record(l_en, l_zc, buffer);
				trigger(l_en, l_zc);



			}
			if (m_recognition_flag)
			{
				l_en = get_en(buffer);
				l_zc = get_zc(buffer);
				if (m_trigger_flag)
					recognition(l_en, l_zc, buffer);
				trigger(l_en, l_zc);


			}
			



		}//if unprepare suceessed

		break;
	}
}


void CMFCApplication1Dlg::OnBnClickedButton2()
{
	// TODO:  在此添加控件通知处理程序代码
	
	m_trigger_flag = 0;
	init_template_data();
	load_template_file();
	m_recognition_flag = 1;
	
}

//new code here
//1、声音开始点检测 2、若是开始点清空录音文件 3 预存前帧特征
void trigger(int en,int zc)
{
	FILE *fp;
	if (m_trigger_flag ||en > ENERGETHRESHOLD)
	{
		if (!m_trigger_flag&&m_record_flag)
		{
			fopen_s(&fp, "d:\\模板录音.pcm", "wb");
			fclose(fp);
		}
		if (!m_trigger_flag&&m_recognition_flag)
		{ 
			fopen_s(&fp, "d:\\识别录音.pcm", "wb"); 
            fclose(fp);
		}

		m_trigger_flag = 1;
		m_pre_en = en;
		m_pre_zc = zc;
	}
}
//训练，处理当前帧
void record(int en, int zc,short *data){
	if (m_template.m_cur_pos >= SLIDEWINDOWLEN)
	{
		//如果模板装满，结束训练，写文件，停止录音
		write_template_file();
		m_record_flag = 0;
		m_trigger_flag = 0;
		::MessageBox(0, L"模板训练结束！！！", L"信息窗口", 0);
		return;
	}
	//存当前帧特征，相对能量，相对过零率
	m_template.m_EN[m_template.m_cur_pos] = (float)en / (float)m_pre_en;
	m_template.m_ZC[m_template.m_cur_pos] = (float)zc / (float)m_pre_zc;
	m_template.m_cur_pos++;
	//自动录音
	FILE *fp;
	fopen_s(&fp, "d:\\模板录音.pcm", "ab");
	fwrite(data, sizeof(short), MAXWAVEBUFFER / 2, fp);
	fclose(fp);

}
//识别，处理当前帧
void recognition(int en, int zc,short *data){
	//向后滑动
	if (m_slide_data.m_cur_pos==SLIDEWINDOWLEN)
	{
		memcpy((void *)m_slide_data.m_EN, (void *)(m_slide_data.m_EN + 1), sizeof(float)*(SLIDEWINDOWLEN - 1));
		memcpy((void *)m_slide_data.m_ZC, (void *)(m_slide_data.m_ZC + 1), sizeof(float)*(SLIDEWINDOWLEN - 1));
		m_slide_data.m_cur_pos--;
	}
	//写入当前帧特征，相对能量，相对过零率
	m_slide_data.m_EN[m_slide_data.m_cur_pos] = (float)en / (float)m_pre_en;
	m_slide_data.m_ZC[m_slide_data.m_cur_pos] = (float)zc / (float)m_pre_zc;
	m_slide_data.m_cur_pos++;
	//匹配，若成功，结束检测
	if (match())
	{
		m_recognition_flag = 0;
		m_trigger_flag = 0;
		::MessageBox(0, L"成功检测到模板音频数据！！！", L"信息窗口", 0);
		return;

	}
	//检测时，自动录音
	FILE *fp;
	fopen_s(&fp, "d:\\识别录音.pcm", "ab");
	fwrite(data, sizeof(short), MAXWAVEBUFFER / 2, fp);
	fclose(fp);

}
//匹配模板数据和滑动窗
bool match()
{
	if (m_slide_data.m_cur_pos < SLIDEWINDOWLEN) return 0;
	//算欧式距
	float sum = 0.0;
	for (int i = 0; i < SLIDEWINDOWLEN; i++)
	{
		sum += (m_slide_data.m_EN[i] - m_template.m_EN[i])*(m_slide_data.m_EN[i] - m_template.m_EN[i]);
	}
	sum = sqrt(sum);
	if (!ENERGEONLY)
	{

	}
	
	if (sum < MATCHTHRESHOLD)
	{
		//debug
		FILE *fp;
		fopen_s(&fp, "d:\\debug.txt", "at");
		fprintf(fp, "%f\n", sum);
		fclose(fp);
		//debug end
		return 1;
	}
		
	return 0;
}
//写模板数据到文件
void write_template_file(){
	FILE *fp;
	fopen_s(&fp, "d:\\template.dat", "wb");
	fwrite(&m_template, sizeof(MyTemplate), 1, fp);
	fclose(fp);

}
//从文件载入模板数据
void load_template_file(){
	FILE *fp;
	fopen_s(&fp, "d:\\template.dat", "rb");
	fread(&m_template, sizeof(MyTemplate), 1, fp);
	fclose(fp);

}
//初始化模板数据区和滑动窗数据区
void init_template_data(){
	m_template.m_cur_pos = 0;
	m_slide_data.m_cur_pos = 0;
}
//计算本帧能量
int get_en(short *sound){
	int sum = 0;
	for (int i = 0; i < MAXWAVEBUFFER / 2; i++)
		sum += abs(sound[i]);
	return sum;
}
//计算本帧过零率
int get_zc(short *sound){
	int cnt = 1;
	for (int i = 1; i < MAXWAVEBUFFER / 2; i++)
		if (sound[i] * sound[i-1]<0)
			cnt++;
	return cnt;
}

void CMFCApplication1Dlg::OnBnClickedButton3()
{
	// TODO:  在此添加控件通知处理程序代码
	m_recognition_flag = 0;
	m_trigger_flag = 0;
	m_record_flag = 0;
	::MessageBox(0, L"已停止检测，未检测到模板音频！！！", L"信息窗口", 0);
}
