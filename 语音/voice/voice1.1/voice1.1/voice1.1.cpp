// voice1.1.cpp : �������̨Ӧ�ó������ڵ㡣
//
// dtw.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"



static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, WORD dwInstance,
						 WAVEHDR* dwParam1, DWORD dwParam2);



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAXWAVEBLOCK  100//�����ڴ����
#define MAXWAVEBUFFER 320//ÿ���ڴ��С 20ms


bool m_Opened;          //��ǰ¼��״̬��false-ֹͣ¼��  true-����¼��
static WAVEHDR WaveHdrIn[MAXWAVEBLOCK];  //
static char WaveInBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//��¼¼���������ڴ��
static HWAVEIN hWaveIn;
FILE *fp = NULL;

void closewave();

void startwork();

void startwork( )
{
	
    

	int i;
	
	//��ʼ��¼���豸
	WAVEFORMATEX waveFormatEx;
	waveFormatEx.cbSize=0;
	waveFormatEx.nAvgBytesPerSec=16000;
	waveFormatEx.nBlockAlign=2;
	waveFormatEx.nChannels=1;
	waveFormatEx.nSamplesPerSec=8000;
	waveFormatEx.wBitsPerSample=16;
	waveFormatEx.wFormatTag=WAVE_FORMAT_PCM;
        hWaveIn = NULL;
	if(waveInOpen(&hWaveIn, WAVE_MAPPER, &waveFormatEx, (unsigned long)waveInProc, 1L, CALLBACK_FUNCTION)) 
	{
	    //MessageBox(0,"��������ʧ��","��Ϣ����",0);
		std::cout << "��������ʧ��";
		return;//¼���豸��ʧ�ܣ�����������֧��ȫ˫����
	}
	else
	{
		//std::cout << "waveInOpen ok~\n";
		
	}
	
	
	for(i=0;i < MAXWAVEBLOCK; i++)
	{//����¼���ڴ�
		WaveHdrIn[i].lpData=WaveInBuffer[i];
		WaveHdrIn[i].dwBufferLength=MAXWAVEBUFFER;
		WaveHdrIn[i].dwFlags=0;
		WaveHdrIn[i].dwUser=0;
		if(waveInPrepareHeader(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))
		{//�޷��������ڴ�
			closewave();
			//MessageBox(0,"��������ʧ��","��Ϣ����",0);
			std::cout << "��������ʧ��";
			return;
		}; 
		if(waveInAddBuffer(hWaveIn,&WaveHdrIn[i],sizeof(WAVEHDR)))
		{//�޷����Ӹ��ڴ�
			closewave();
			//MessageBox(0,"��������ʧ��","��Ϣ����",0);
			std::cout << "��������ʧ��";
			return;
		}
	}

	 //������
    waveInStart(hWaveIn);
	fopen_s(&fp, "hello.wav", "wb");
	
	fwrite("RIFF", 1, 4, fp);

	unsigned int dwDataLength = 1024 * 1024;
	unsigned int waveformatSize = sizeof WAVEFORMATEX;
	unsigned int Sec = (1024 * 1024 + waveformatSize);
	fwrite(&Sec, sizeof(Sec), 1, fp);//�����е�����
	fwrite("WAVE", 4, 1, fp);
	fwrite("fmt ", 1, 4, fp);
	fwrite(&waveformatSize, sizeof(waveformatSize), 1, fp);
	fwrite(&waveFormatEx.wFormatTag, sizeof(waveFormatEx.wFormatTag), 1, fp);
	fwrite(&waveFormatEx.nChannels, sizeof(waveFormatEx.nChannels), 1, fp);
	fwrite(&waveFormatEx.nSamplesPerSec, sizeof(waveFormatEx.nSamplesPerSec), 1, fp);
	fwrite(&waveFormatEx.nAvgBytesPerSec, sizeof(waveFormatEx.nAvgBytesPerSec), 1, fp);
	fwrite(&waveFormatEx.nBlockAlign, sizeof(waveFormatEx.nBlockAlign), 1, fp);
	fwrite(&waveFormatEx.wBitsPerSample, sizeof(waveFormatEx.wBitsPerSample), 1, fp);
	fwrite(&waveFormatEx.cbSize, sizeof(waveFormatEx.cbSize), 1, fp);
	fwrite("data", 4, 1, fp);
	fwrite(&dwDataLength, sizeof(dwDataLength), 1, fp);//�����е�����
	

  
}
void closewave()
{
	if(hWaveIn){
		waveInStop(hWaveIn);//ֹͣ¼��
		waveInReset(hWaveIn);//���¼������
		waveInClose(hWaveIn);//�ر�¼���豸
		hWaveIn = NULL;
	}
}



static void CALLBACK waveInProc(HWAVEIN hwi, UINT uMsg, WORD dwInstance,
						 WAVEHDR* dwParam1, DWORD dwParam2)
{

	char buffer[MAXWAVEBUFFER];
	long buf_len;
    int i;
  
	switch(uMsg)
	{//�жϻص�����
	case WIM_OPEN:
		//¼���豸������ʱ���������¼�
		//MessageBox(NULL,"Open success", "��ʾ", MB_OK);
		std::cout << "WIM_OPEN\n";
		break;
	case WIM_CLOSE:
		//¼���豸�����ر�ʱ���������¼�
		//MessageBox(NULL,"Close success", "��ʾ", MB_OK);
		std::cout << "WIN_CLOSE";
		break;
	case WIM_DATA:
		//¼���豸��������ʱ���������¼�
		//std::cout << "WIM_DATA\n";
		if(!waveInUnprepareHeader(hwi, dwParam1,sizeof(WAVEHDR)))
		{//�����ͷ��ڴ档�е������Ȼ�������ݣ��������ͷ��ڴ棬
		 //��ʹ��ͬһ��WAVEHDR���Ӷ���ڴ��ʱ��������������
			
			buf_len = dwParam1->dwBytesRecorded;//��¼¼�����ݳ���
			//memcpy(buffer, dwParam1->lpData , buf_len);//���Ʒ�������
			fwrite(dwParam1->lpData, buf_len, 1, fp);


			dwParam1->dwUser=0;//�ָ�����
			dwParam1->dwBufferLength=MAXWAVEBUFFER;
			dwParam1->dwFlags=0;
			
			waveInPrepareHeader(hwi,dwParam1,sizeof(WAVEHDR));
 			waveInAddBuffer(hwi,dwParam1,sizeof(WAVEHDR));
			//std::cout << "ing\n";
		}//if unprepare suceessed

		break;
	}
}



int _tmain(int argc, _TCHAR* argv[])
{

	startwork();
	//closewave();
	Sleep(10000);
	closewave();
	system("pause");
	return 0;
}

