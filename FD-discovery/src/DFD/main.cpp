#include "DFD.h"

int main()
{
	cout << "�����������������Ŀ��" << endl;
	int attributions = 0;
	cin >> attributions;
	cout << "�������д��롭��" << endl;
    DFD dfd = DFD("test_data.txt", "test_output_data.txt", attributions);
	dfd.generateFD();
	//system.pause();
	return 0;
}