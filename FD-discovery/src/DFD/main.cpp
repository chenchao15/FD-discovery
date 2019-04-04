#include "DFD.h"

int main()
{
	cout << "请输入包含的属性数目：" << endl;
	int attributions = 0;
	cin >> attributions;
	cout << "正在运行代码……" << endl;
    DFD dfd = DFD("test_data.txt", "test_output_data.txt", attributions);
	dfd.generateFD();
	//system.pause();
	return 0;
}