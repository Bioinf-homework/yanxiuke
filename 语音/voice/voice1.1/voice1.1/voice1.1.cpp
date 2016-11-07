// voice1.1.cpp : 定义控制台应用程序的入口点。
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

#define MAXWAVEBLOCK  100//输入内存块数
#define MAXWAVEBUFFER 320//每块内存大小 20ms


bool m_Opened;          //当前录音状态，false-停止录音  true-正在录音
static WAVEHDR WaveHdrIn[MAXWAVEBLOCK];  //
static char WaveInBuffer[MAXWAVEBLOCK][MAXWAVEBUFFER];//记录录音数据用内存块
static HWAVEIN hWaveIn;
FILE *fp = NULL;

void closewave();

void startwork();

void startwork( )
{
	
    

	int i;
	
	//初始化录音设备
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
	    //MessageBox(0,"声卡设置失败","信息窗口",0);
		std::cout << "声卡设置失败";
		return;//录音设备打开失败，可能声卡不支持全双工。
	}
	else
	{
		//std::cout << "waveInOpen ok~\n";
		
	}
	
	
	for(i=0;i < MAXWAVEBLOCK; i++)
	{//增加录音内存
		WaveHdrIn[i].lpData=WaveInBuffer[i];
		WaveHdrIn[i].dwBufferLength=MAXWAVEBUFFER;
		WaveHdrIn[i].dwFlags=0;
		WaveHdrIn[i].dwUser=0;
		if(waveInPrepareHeader(hWaveIn, &WaveHdrIn[i], sizeof(WAVEHDR)))
		{//无法锁定该内存
			closewave();
			//MessageBox(0,"声卡设置失败","信息窗口",0);
			std::cout << "声卡设置失败";
			return;
		}; 
		if(waveInAddBuffer(hWaveIn,&WaveHdrIn[i],sizeof(WAVEHDR)))
		{//无法增加该内存
			closewave();
			//MessageBox(0,"声卡设置失败","信息窗口",0);
			std::cout << "声卡设置失败";
			return;
		}
	}

	 //打开声卡
    waveInStart(hWaveIn);
	fopen_s(&fp, "hello.wav", "wb");
	
	fwrite("RIFF", 1, 4, fp);

	unsigned int dwDataLength = 1024 * 1024;
	unsigned int waveformatSize = sizeof WAVEFORMATEX;
	unsigned int Sec = (1024 * 1024 + waveformatSize);
	fwrite(&Sec, sizeof(Sec), 1, fp);//这里有点问题
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
	fwrite(&dwDataLength, sizeof(dwDataLength), 1, fp);//这里有点问题
	

  
}
void closewave()
{
	if(hWaveIn){
		waveInStop(hWaveIn);//停止录音
		waveInReset(hWaveIn);//清空录音缓存
		waveInClose(hWaveIn);//关闭录音设备
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
	{//判断回调类型
	case WIM_OPEN:
		//录音设备正常打开时，触发该事件
		//MessageBox(NULL,"Open success", "提示", MB_OK);
		std::cout << "WIM_OPEN\n";
		break;
	case WIM_CLOSE:
		//录音设备正常关闭时，触发该事件
		//MessageBox(NULL,"Close success", "提示", MB_OK);
		std::cout << "WIN_CLOSE";
		break;
	case WIM_DATA:
		//录音设备返回数据时，触发该事件
		//std::cout << "WIM_DATA\n";
		if(!waveInUnprepareHeader(hwi, dwParam1,sizeof(WAVEHDR)))
		{//可以释放内存。有的情况虽然返回数据，但不能释放内存，
		 //如使用同一个WAVEHDR增加多个内存块时，会出现上述情况
			
			buf_len = dwParam1->dwBytesRecorded;//记录录音数据长度
			//memcpy(buffer, dwParam1->lpData , buf_len);//复制返回数据
			fwrite(dwParam1->lpData, buf_len, 1, fp);


			dwParam1->dwUser=0;//恢复设置
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

