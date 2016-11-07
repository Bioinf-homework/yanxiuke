#define VECTOR_ORDER   22   //矢量维数


//计算用结构
typedef double * Vector;

typedef struct {
	double sdis; //累计距离
	BYTE path;   //回溯路径 1-水平 2-垂直  3-对角
	BYTE hcount; //水平回溯计数
	double hdis; //水平回溯均值
} DTWStru;
