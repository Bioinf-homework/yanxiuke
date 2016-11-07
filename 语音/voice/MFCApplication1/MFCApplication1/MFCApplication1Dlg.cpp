
// MFCApplication1Dlg.cpp : ʵ���ļ�
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



#define MAXWAVEBLOCK  100//�����ڴ����
#define MAXWAVEBUFFER 320//ÿ���ڴ��С 20ms


static WAVEHDR WaveHdrIn[MAXWAVEBLOCK];  //
static char WaveInBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//��¼¼���������ڴ��
static HWAVEIN hWaveIn;

//new code here
#define SLIDEWINDOWLEN  500//���������ȣ�Ϊ֡��
#define ENERGEONLY 1//���Ϊ1��ʾ�����������������������������͹���������
#define ENERGETHRESHOLD  4000*160//��������
#define MATCHTHRESHOLD  5.3//ƥ���������

bool m_record_flag;          //��ǰ¼��״̬��false-ֹͣ¼��  true-����¼��
bool m_recognition_flag;     //��ǰʶ��״̬,false-ֹͣʶ��true-����ʶ��
bool m_trigger_flag;     //��ǰ����״̬��0-δ������1--����

typedef struct{
	float m_EN[SLIDEWINDOWLEN];//ģ������������洢��
	float m_ZC[SLIDEWINDOWLEN];//ģ��Ĺ����������洢��	
	short m_cur_pos;//��ǰд��λ��
} MyTemplate;

int m_pre_en;//ǰ֡����
int m_pre_zc;//ǰ֡������
MyTemplate m_template;  //ģ������
MyTemplate m_slide_data;//����������

void trigger(int en,int zc);
void record(int en,int zc,short *data);
void recognition(int en,int zc,short *data);
void write_template_file();
void load_template_file();
void init_template_data();
int get_en(short *sound);
int get_zc(short *sound);
bool match();

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CMFCApplication1Dlg �Ի���



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


// CMFCApplication1Dlg ��Ϣ�������

BOOL CMFCApplication1Dlg::OnInitDialog()
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

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	m_record_flag = 0;
	m_recognition_flag = 0;
	m_trigger_flag = 0;

	startwork();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CMFCApplication1Dlg::OnPaint()
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
HCURSOR CMFCApplication1Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMFCApplication1Dlg::OnBnClickedButton1()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	
	m_trigger_flag = 0;
	init_template_data();
	m_record_flag = 1;
}




void startwork()
{

	int sta;

	int i;

	//��ʼ��¼���豸
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
		::MessageBox(0, L"��������ʧ��", L"��Ϣ����", 0);
		return;//¼���豸��ʧ�ܣ�����������֧��ȫ˫����
	}


	for (i = 0; i < MAXWAVEBLOCK; i++)
	{//����¼���ڴ�
		WaveHdrIn[i].lpData = WaveInBuffer[i];
		WaveHdrIn[i].dwBufferLength = MAXWAVEBUFFER;
		WaveHdrIn[i].dwFlags = 0;
		WaveHdrIn[i].dwUser = 0;
		if (waveInPrepareHeader(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))
		{//�޷��������ڴ�
			closewave();
			::MessageBox(0, L"��������ʧ��", L"��Ϣ����", 0);
			return;
		};
		if (waveInAddBuffer(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))
		{//�޷����Ӹ��ڴ�
			closewave();
			::MessageBox(0, L"��������ʧ��", L"��Ϣ����", 0);
			return;
		}
	}
	//������
	waveInStart(hWaveIn);


}
void closewave()
{

	if (hWaveIn){
		waveInStop(hWaveIn);//ֹͣ¼��
		waveInReset(hWaveIn);//���¼������
		waveInClose(hWaveIn);//�ر�¼���豸
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
	{//�жϻص�����
	case WIM_OPEN:
		//¼���豸������ʱ���������¼�
		//MessageBox(NULL,"Open success", "��ʾ", MB_OK);
		break;
	case WIM_CLOSE:
		//¼���豸�����ر�ʱ���������¼�
		//MessageBox(NULL,"Close success", "��ʾ", MB_OK);
		break;
	case WIM_DATA:
		
		
		//¼���豸��������ʱ���������¼�
		if (!waveInUnprepareHeader(hwi, dwParam1, sizeof(WAVEHDR)))
		{//�����ͷ��ڴ档�е������Ȼ�������ݣ��������ͷ��ڴ棬
			//��ʹ��ͬһ��WAVEHDR���Ӷ���ڴ��ʱ��������������

			buf_len = dwParam1->dwBytesRecorded;//��¼¼�����ݳ���
			memcpy((void *)buffer, dwParam1->lpData, buf_len);//���Ʒ�������
			
			dwParam1->dwUser = 0;//�ָ�����
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
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	
	m_trigger_flag = 0;
	init_template_data();
	load_template_file();
	m_recognition_flag = 1;
	
}

//new code here
//1��������ʼ���� 2�����ǿ�ʼ�����¼���ļ� 3 Ԥ��ǰ֡����
void trigger(int en,int zc)
{
	FILE *fp;
	if (m_trigger_flag ||en > ENERGETHRESHOLD)
	{
		if (!m_trigger_flag&&m_record_flag)
		{
			fopen_s(&fp, "d:\\ģ��¼��.pcm", "wb");
			fclose(fp);
		}
		if (!m_trigger_flag&&m_recognition_flag)
		{ 
			fopen_s(&fp, "d:\\ʶ��¼��.pcm", "wb"); 
            fclose(fp);
		}

		m_trigger_flag = 1;
		m_pre_en = en;
		m_pre_zc = zc;
	}
}
//ѵ��������ǰ֡
void record(int en, int zc,short *data){
	if (m_template.m_cur_pos >= SLIDEWINDOWLEN)
	{
		//���ģ��װ��������ѵ����д�ļ���ֹͣ¼��
		write_template_file();
		m_record_flag = 0;
		m_trigger_flag = 0;
		::MessageBox(0, L"ģ��ѵ������������", L"��Ϣ����", 0);
		return;
	}
	//�浱ǰ֡�����������������Թ�����
	m_template.m_EN[m_template.m_cur_pos] = (float)en / (float)m_pre_en;
	m_template.m_ZC[m_template.m_cur_pos] = (float)zc / (float)m_pre_zc;
	m_template.m_cur_pos++;
	//�Զ�¼��
	FILE *fp;
	fopen_s(&fp, "d:\\ģ��¼��.pcm", "ab");
	fwrite(data, sizeof(short), MAXWAVEBUFFER / 2, fp);
	fclose(fp);

}
//ʶ�𣬴���ǰ֡
void recognition(int en, int zc,short *data){
	//��󻬶�
	if (m_slide_data.m_cur_pos==SLIDEWINDOWLEN)
	{
		memcpy((void *)m_slide_data.m_EN, (void *)(m_slide_data.m_EN + 1), sizeof(float)*(SLIDEWINDOWLEN - 1));
		memcpy((void *)m_slide_data.m_ZC, (void *)(m_slide_data.m_ZC + 1), sizeof(float)*(SLIDEWINDOWLEN - 1));
		m_slide_data.m_cur_pos--;
	}
	//д�뵱ǰ֡�����������������Թ�����
	m_slide_data.m_EN[m_slide_data.m_cur_pos] = (float)en / (float)m_pre_en;
	m_slide_data.m_ZC[m_slide_data.m_cur_pos] = (float)zc / (float)m_pre_zc;
	m_slide_data.m_cur_pos++;
	//ƥ�䣬���ɹ����������
	if (match())
	{
		m_recognition_flag = 0;
		m_trigger_flag = 0;
		::MessageBox(0, L"�ɹ���⵽ģ����Ƶ���ݣ�����", L"��Ϣ����", 0);
		return;

	}
	//���ʱ���Զ�¼��
	FILE *fp;
	fopen_s(&fp, "d:\\ʶ��¼��.pcm", "ab");
	fwrite(data, sizeof(short), MAXWAVEBUFFER / 2, fp);
	fclose(fp);

}
//ƥ��ģ�����ݺͻ�����
bool match()
{
	if (m_slide_data.m_cur_pos < SLIDEWINDOWLEN) return 0;
	//��ŷʽ��
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
//дģ�����ݵ��ļ�
void write_template_file(){
	FILE *fp;
	fopen_s(&fp, "d:\\template.dat", "wb");
	fwrite(&m_template, sizeof(MyTemplate), 1, fp);
	fclose(fp);

}
//���ļ�����ģ������
void load_template_file(){
	FILE *fp;
	fopen_s(&fp, "d:\\template.dat", "rb");
	fread(&m_template, sizeof(MyTemplate), 1, fp);
	fclose(fp);

}
//��ʼ��ģ���������ͻ�����������
void init_template_data(){
	m_template.m_cur_pos = 0;
	m_slide_data.m_cur_pos = 0;
}
//���㱾֡����
int get_en(short *sound){
	int sum = 0;
	for (int i = 0; i < MAXWAVEBUFFER / 2; i++)
		sum += abs(sound[i]);
	return sum;
}
//���㱾֡������
int get_zc(short *sound){
	int cnt = 1;
	for (int i = 1; i < MAXWAVEBUFFER / 2; i++)
		if (sound[i] * sound[i-1]<0)
			cnt++;
	return cnt;
}

void CMFCApplication1Dlg::OnBnClickedButton3()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	m_recognition_flag = 0;
	m_trigger_flag = 0;
	m_record_flag = 0;
	::MessageBox(0, L"��ֹͣ��⣬δ��⵽ģ����Ƶ������", L"��Ϣ����", 0);
}
