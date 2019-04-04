#pragma once

#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <ctime>
#include <stack>
#include <set>
#include <map>

using namespace std;

enum TYPE { nulltype, dep, mindep, canmindep, nondep, maxnondep, canmaxnondep};

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
		while(left[i]!=-1)i++;
		left[i] = n;
		left[i+1] = -1;
	}
	void rightAdd(int n){right = n;}
	int getleft(int pos){return left[pos];}
	int getright(){return right;}
	void print()
	{
		int i = 0;
		while(left[i]!=-1)
		{
			cout << left[i] << ' ';
			i++;
		}
		cout << "-> ";
		cout << right << endl;
	}
};

class DFD
{
public:
	DFD() : DFD_attributions(0){}
	DFD(string infile, string outfile, int num);
	~DFD() {}

	void importData();
	void exportData(int left, int right, int count);
	void generateNextLevel(int level);
	void computeDependencies(int level, ostream& outputstream);
	bool isValid(int it, int attr);
	int computePi(int attr);
	int computePiProduct(int attr1, int attr2);
	void removeAttr(int it, int attr);
	void generateFD();
	bool isUnique(int attr);
	void findLHS(int attr);
	int inferCategory(int node);
	int pickNextNode(int attr, int node);
	vector<int> uncheckedSubsets(int node);
	vector<int> prunedSets(int node, vector<int> S);
	vector<int> uncheckedSupersets(int node);
	vector<int> prunedSupersets(int node, vector<int> S);
	vector<int> generateNextSeeds();
	void variableClear();
	void print(ostream& outputstream);

private:
	int DFD_totalCount;
	const int DFD_attributions;
	int DFD_attrCombination;
	vector<pair<int, int> > DFD_dependencies;
	vector<vector<string> > DFD_table;
	vector<set<int> > DFD_levels;
	vector<vector<vector<int> > > DFD_partitions;
	vector<int> DFD_visited;
	vector<int> DFD_type;
	vector<int> DFD_RHS;
	vector<int> DFD_exists;
	vector<int> DFD_piLen;
	stack<int> DFD_trace;
	set<int> DFD_minDeps;
	set<int> DFD_maxNonDeps;
	string DFD_input_txt_file;
	string DFD_output_txt_file;
	RESULT res[1000];
};