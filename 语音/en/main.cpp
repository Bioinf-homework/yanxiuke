#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

#define SLIDEWINDOWLEN  75//滑动窗长度，为帧数   1.5s

#define MAXWAVEBUFFER 320

#define ENERGETHRESHOLD  2000*160    //能量门限

//match~~
#define Zhen 75

#define     fake    13
#define     PCEP    12
float ma[Zhen][13];

int main()
{
    int i ,j;
/**
 *读取音频数据
 */
    FILE *fp;
	fp = fopen("1.pcm", "rb");
	short data[SLIDEWINDOWLEN][MAXWAVEBUFFER/2];

	for(i = 0;i<SLIDEWINDOWLEN;i++)
    {
        for(j = 0;j<MAXWAVEBUFFER/2;j++)
        {
             fread(&data[i][j], sizeof(short), 1, fp);
             //cout << data[i][j] << "\t";
             //system("pause");
        }
        //cout << endl;
    }
	fclose(fp);
/**
 *计算能量
 */
    float en[SLIDEWINDOWLEN]={};

	for(i = 0;i<SLIDEWINDOWLEN;i++)
    {
        for(j = 0;j<MAXWAVEBUFFER/2;j++)
        {
            en[i] += abs(data[i][j]);
        }
        //cout<<i<<"\t"<<i/50.0<<"\t" <<en[i]<<endl;
        //system("pause");
    }
/**
 *端点检测,写入time.txt .more test   or other methods
 */
    int preflag = 0,endflag = 0,flag=0;

    for(i = 0;i<SLIDEWINDOWLEN;i++)
    {
        if(preflag==0)
        {
            if(en[i]>ENERGETHRESHOLD)
            {
                preflag = i;
            }
        }
        else
        {
            if(en[i]>ENERGETHRESHOLD)
            {
                flag = 0;
            }
            else
            {
                if(endflag==0)
                {
                    endflag = i;
                }
                else
                {
                    flag++;
                    //cout<<"++"<<i<<endl;
                    endflag = i-flag;
                }
                if(flag>=15)
                {
                    break;
                }

            }

        }
    }
    cout << preflag<<"\t"<<endflag<<endl;
    FILE *fptime;
    fptime = fopen("time.txt","w");
    fprintf(fptime,"%d\t%f\n%d\t%f",preflag,preflag/50.0,endflag,endflag/50.0);
    fclose(fptime);

/**
 *计算相对能量，写入      (把后两帧也算出来吧。。。要算一阶差分，二阶差分。
 */
    FILE *fpwrite;

    float enrate[SLIDEWINDOWLEN];

    fpwrite = fopen("en.txt", "w");

    for(i=preflag;i<endflag;i++)
    {
        enrate[i] = en[i]/en[i-1];
        //cout << en[i-1] << endl;
        fprintf(fpwrite,"%f\n",enrate[i]);
    }
    fclose(fpwrite);

/**
 *计算全帧MFCC，写入
 */
    system("MFCC.exe");

/**
 *读MFCC数据啦啦啦
 */
    FILE *fmain;

    fmain = fopen("cepstrum.txt","r");

    for(i=0;i<Zhen;i++)
    {
        for(j=0;j<fake;j++)
        {
            fscanf(fmain,"%f",&ma[i][j]);
        }
    }
    fclose(fmain);
/**
 *写出便于读的文件666.txt
 */

    FILE *fnew;
    fnew = fopen("666.txt","w");
    fprintf(fnew,"%d\n",endflag-preflag);
    for(i=preflag;i<endflag;i++)
    {
        for(j=0;j<PCEP;j++)
        {
            fprintf(fnew,"%f ",ma[i][j]);
        }
        for(j=0;j<PCEP;j++)
        {
            fprintf(fnew,"%f ",ma[i][j]-ma[i+1][j]);
        }
        for(j=0;j<PCEP;j++)
        {
            fprintf(fnew,"%f ",ma[i][j]-2*ma[i+1][j]+ma[i+2][j]);
        }
        fprintf(fnew,"%f ",enrate[i]);
        fprintf(fnew,"%f ",enrate[i]-enrate[i+1]);
        fprintf(fnew,"%f\n",enrate[i]-2*enrate[i+1]+enrate[i+2]);
    }
    fclose(fnew);


    //system("pause");
    return 0;
}
