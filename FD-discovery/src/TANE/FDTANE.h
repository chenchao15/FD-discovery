#pragma once

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <ctime>
#include <set>

using namespace std;

class RESULT
{
private:
	int left[10];
	int right;
public:
	RESULT()
	{
		left[0] = -1;
	}
	void leftAdd(int n)
	{
		int i = 0;
		while (left[i] != -1)i++;
		left[i] = n;
		left[i + 1] = -1;
	}
	void rightAdd(int n) { right = n; }
	int getleft(int pos) { return left[pos]; }
	int getright() { return right; }
	void print()
	{
		int i = 0;
		while (left[i] != -1)
		{
			cout << left[i] << ' ';
			i++;
		}
		cout << "-> ";
		cout << right << endl;
	}
};

class FDTANE
{
public:
	FDTANE() : FT_attributions(0) {}
	FDTANE(string infile, string outfile, int num);
	~FDTANE() {}

	void importData();
	void exportData(int left, int right, int count);
	void generateNextLevel(int level);
	void computeDependencies(int level, ostream& outputstream);
	bool isValid(int it, int attr);
	int computePi(int attr);
	int computePiProduct(int attr1, int attr2);
	void removeAttr(int it, int attr);
	void generateFD();
	void print(ostream& outputstream);

private:
	int FT_totalCount;
	const int FT_attributions;
	vector<vector<string> > FT_table;
	vector<set<int> > FT_levels;
	vector<vector<vector<int> > > FT_partitions;
	vector<int> FT_RHS;
	vector<int> FT_exists;
	vector<int> FT_piLen;
	string FT_input_txt_file;
	string FT_output_txt_file;
	RESULT res[1000];
};