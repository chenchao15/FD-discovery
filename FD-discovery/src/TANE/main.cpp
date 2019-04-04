#include "FDTANE.h"

int main()
{
	cout << "请输入包含的属性数目：" << endl;
	int attributions = 0;
	cin >> attributions;
	cout << "正在运行代码……" << endl;
	FDTANE ft = FDTANE("data.txt", "output_data.txt", attributions);
	ft.generateFD();
	return 0;
}