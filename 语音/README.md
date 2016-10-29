voice_final1.1.1是下图的样子，其中录音程序可能无法使用..

voice_final1.1.2是改进性能版，目前正常使用。

match，dtw算法

en，计算MFCC一二阶差分扩充特征，端点检测



录音程序是老师给的，找不到原代码了。可以写成.pcm，加上文件头可以写成.wav

MFCC的计算借助https://github.com/weedwind/MFCC

界面是wxpython写的，借助Boa Constructor撸的。如今可能跑不动这个IDE了。。



![](./res.png)

录音键bind调用py_voice.exe。进行录音，生成.pcm或，.wav

训练键bind调用en.exe，en.exe调用MFCC，生成MFCC数据，再处理。

匹配键bind调用match.exe，dtw算法对比库中文件和该录音最接近的