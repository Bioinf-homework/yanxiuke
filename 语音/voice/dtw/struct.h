#define VECTOR_ORDER   22   //ʸ��ά��


//�����ýṹ
typedef double * Vector;

typedef struct {
	double sdis; //�ۼƾ���
	BYTE path;   //����·�� 1-ˮƽ 2-��ֱ  3-�Խ�
	BYTE hcount; //ˮƽ���ݼ���
	double hdis; //ˮƽ���ݾ�ֵ
} DTWStru;
