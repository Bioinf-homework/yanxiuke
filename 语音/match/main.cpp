#include <iostream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define TEMPNUM 8
#define Zhen 75

#define     fake    13
#define     PCEP    39
using namespace std;

void match();
float distant(int x,int y,int k);
float dtw(int x,int y,int k);
void compare();

float ma[Zhen][PCEP],v[TEMPNUM][Zhen][PCEP];

int t[TEMPNUM]={};//记录每段音频的帧数
int time;//记录待测音频的帧数

float sum[TEMPNUM]={};

float dtwsum[TEMPNUM][Zhen][Zhen]={};//-1

float result[TEMPNUM]={};

int main()
{
/**
 *读取666.txt中刚录的音频的特征
 */
    int i,j,k;
    FILE *fmain;


    fmain = fopen("666.txt","r");

    fscanf(fmain,"%d\n",&time);

    for(i=0;i<time;i++)
    {
        for(j=0;j<39;j++)
        {
            fscanf(fmain,"%f",&ma[i][j]);
        }
    }

    fclose(fmain);

/**
 *读取v文件夹下的语料库--v
 *                       |--file_num.dat（不用。。
 *                       |--001.txt
 *                       |--002.txt
 *                       |--....
 */
    //FILE *fnum;
    //fnum = ("v/file_num.dat","r");
    int w1,w2,w3;
    char F[20]="v\\000.txt";
    for(k=0;k<TEMPNUM;k++)
    {
        FILE *f;
        w1 = (k+1)%10;
        w2 = (k+1)/10%10;
        w3 = (k+1) / 100;
        F[4] = char(w1+48);
        F[3] = char(w2+48);
        F[2] = char(w3+48);
        f = fopen(F,"r");

        fscanf(f,"%d\n",&t[k]);
        //cout << F;
        for(i=0;i<t[k];i++)
        {
            for(j=0;j<PCEP;j++)
            {
                fscanf(f,"%f",&v[k][i][j]);
            }
        }

        fclose(f);
    }

    match();

    compare();

    system("pause");
    return 0;
}




void match()
{
    for(int k=0;k<TEMPNUM;k++)
    {
        for(int i=0;i<t[k];i++)
        {
            for(int j=0;j<PCEP;j++)
            {
                sum[k]+= powf(abs(ma[i][j]-v[k][i][j]),2);
            }
        }
        sum[k]=sqrtf(sum[k]);
    }
    for(int k=0;k<TEMPNUM;k++)
    {
        cout<<"The distance to No."<< k+1<<" is:  "<< sum[k]<<endl;
    }
}

float distant(int x,int y,int k)
{
    float sum=0;
    for(int j=0;j<PCEP;j++)
    {
        sum += powf((ma[x][j]-v[k][y][j]),2);
    }
    return sum;
}
float dtw(int x,int y,int k)
{
    if(dtwsum[k][x][y]!=0)
    {
        return dtwsum[k][x][y];
    }

    if(x==0||y==0)
    {
        if(x==0&&y==0)
        {
            dtwsum[k][x][y]=distant(x,y,k);
            return dtwsum[k][x][y];
        }
        if(x==0)
        {
            dtwsum[k][x][y] = dtw(x,y-1,k)+distant(x,y,k)/2;
            //cout <<endl;
        }

        if(y==0)
        {
            dtwsum[k][x][y] = dtw(x-1,y,k)+distant(x,y,k)/2;
                  //cout <<endl;
        }

        //dtwsum[k][x][y]=distant(x,y,k);//这里错了。。。。

        //cout << x<<"\t"<< y <<":"<< dtwsum[k][x][y]<<endl;
    }
    else
    {
        float left = dtw(x-1,y,k)+distant(x,y,k)/2;
        float down = dtw(x,y-1,k)+distant(x,y,k)/2;
        float inclined  = dtw(x-1,y-1,k) + distant(x,y,k);
        if(inclined < min(left,down))
        {
            dtwsum[k][x][y] = inclined;
        }
        else
        {
            if(left < down)  dtwsum[k][x][y] = left;
            else  dtwsum[k][x][y] = down;
        }
    }
    return dtwsum[k][x][y];
}

void compare()
{
    int k,minFlag=0;
    float minResult;
    for(k=0;k<TEMPNUM;k++)
    {
        result[k] = dtw(time,t[k],k);
        cout << result[k] <<endl;
    }
    minResult = result[0];
    for(k=1;k<TEMPNUM;k++)
    {
        if(result[k]<minResult)
        {
            minResult = result[k];
            minFlag = k;
        }
    }
    cout << minFlag+1 <<"\t" << minResult;
}

