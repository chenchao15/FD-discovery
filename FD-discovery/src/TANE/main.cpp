#include "FDTANE.h"

int main()
{
	cout << "�����������������Ŀ��" << endl;
	int attributions = 0;
	cin >> attributions;
	cout << "�������д��롭��" << endl;
	FDTANE ft = FDTANE("data.txt", "output_data.txt", attributions);
	ft.generateFD();
	return 0;
}