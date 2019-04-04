#include "FDTANE.h"

bool resCmp(RESULT a, RESULT b)
{
	int i = 0;
	while (1)
	{
		if (a.getleft(i) == b.getleft(i) && a.getleft(i) == -1)
			return a.getright() < b.getright();
		else if (a.getleft(i) == b.getleft(i))
		{
			i++;
			continue;
		}
		else
			return a.getleft(i) < b.getleft(i);
	}
}

FDTANE::FDTANE(string infile, string outfile, int num) :
	FT_attributions(num)
{
	FT_input_txt_file = infile;
	FT_output_txt_file = outfile;
	importData();
	FT_totalCount = 0;
	int total_key = (1 << FT_attributions);
	FT_partitions.assign(total_key, vector<vector<int> >());
	FT_exists.assign(total_key, 0);
	FT_piLen.assign(total_key, 0);
	FT_RHS.assign(total_key, 0);
	FT_levels.assign(FT_attributions, set<int>());
	for (int i = 0, index = 1; i < FT_attributions; i++, index <<= 1)
	{
		FT_RHS[index] = total_key - 1;
		FT_levels[0].insert(index);
	}
}

void FDTANE::generateFD()
{
	ofstream outfile(FT_output_txt_file);

	time_t begin_time, end_time;
	begin_time = clock();

	for (int level = 1; level < FT_attributions; level++)
	{
		generateNextLevel(level);
		computeDependencies(level, outfile);
	}
	end_time = clock();
	cout << "函数依赖成功生成……" << endl;
	cout << "共包含" << FT_totalCount << "个函数依赖项" << endl;
	cout << "运行时间：" << (double)(end_time - begin_time) / CLOCKS_PER_SEC << "s" << endl;
	sort(res, res + FT_totalCount, resCmp);
	print(outfile);
	outfile.close();
}

void FDTANE::generateNextLevel(int level)
{
	if (level == 0)
		return;
	for (auto pre_index : FT_levels[level - 1])
		for (int i = 0, index = 1; i < FT_attributions; i++, index <<= 1)
			if (!(pre_index & index))
				FT_levels[level].insert(pre_index + index);
}

void FDTANE::computeDependencies(int level, ostream& outputstream)
{
	set<int>::iterator set_end = FT_levels[level].end();
	for (set<int>::iterator set_it = FT_levels[level].begin(); set_it != set_end;)
	{
		int it = *set_it;
		int rhs = (1 << FT_attributions) - 1;
		for (int i = 0, single_attr = 1; i < FT_attributions; i++, single_attr <<= 1)
			if (it & single_attr)
				rhs = rhs & FT_RHS[it - single_attr];
		FT_RHS[it] = rhs;
		int composite_attr = it & FT_RHS[it];
		for (int i = 0, single_attr = 1; i < FT_attributions; i++, single_attr <<= 1)
		{
			if (composite_attr & single_attr && isValid(it, single_attr))
			{
				FT_totalCount++;
				exportData(it - single_attr, single_attr, FT_totalCount - 1);
				removeAttr(it, single_attr);
			}
		}
		if (FT_RHS[it] == 0)
			set_it = FT_levels[level].erase(set_it);
		else
			set_it++;
	}
}

bool FDTANE::isValid(int it, int attr)
{
	int	pi1 = computePi(it - attr);
	int pi2 = computePiProduct(it - attr, attr);
	return (FT_piLen[it - attr] - pi1 == FT_piLen[it] - pi2);
}

int FDTANE::computePi(int attr)
{
	if (FT_exists[attr])
		return FT_partitions[attr].size();
	int tmp = attr, col = 0;
	for (; col < FT_attributions && !(tmp % 2); col++, tmp /= 2);
	vector<vector<int> > store;
	unordered_map<string, int> hp;
	unordered_map<string, int>::const_iterator hp_it;
	for (int i = 0, index = 0; i < (int)FT_table.size(); i++)
	{
		string substr = FT_table[i][col];
		hp_it = hp.find(substr);
		if (hp_it == hp.end())
		{
			hp.insert(make_pair(substr, index++));
			vector<int> temp;
			temp.push_back(i);
			store.push_back(temp);
		}
		else
			store[hp_it->second].push_back(i);
	}
	for (auto it : store)
		if (it.size() != 1)
		{
			FT_partitions[attr].push_back(it);
			FT_piLen[attr] += it.size();
		}
	FT_exists[attr] = true;
	return FT_partitions[attr].size();
}

void FDTANE::removeAttr(int it, int attr)
{
	FT_RHS[it] -= attr;
	int comple = (1 << FT_attributions) - 1 - it;
	for (int i = 0, single_attr = 1; i < FT_attributions; i++, single_attr <<= 1)
		if (comple & single_attr && FT_RHS[it] & single_attr)
			FT_RHS[it] -= single_attr;
}

int FDTANE::computePiProduct(int attr1, int attr2)
{
	if (FT_exists[attr1 + attr2])
		return FT_partitions[attr1 + attr2].size();
	int pi1 = computePi(attr1), pi2 = computePi(attr2);
	vector<int> project(FT_table.size(), -1);
	vector<vector<int> > product(pi1, vector<int>());
	for (int i = 0; i < pi1; i++)
		for (auto it : FT_partitions[attr1][i])
			project[it] = i;
	for (int i = 0; i < pi2; i++)
	{
		for (auto it : FT_partitions[attr2][i])
			if (project[it] >= 0)
				product[project[it]].push_back(it);
		for (auto it : FT_partitions[attr2][i])
			if (project[it] >= 0)
			{
				if (product[project[it]].size() >= 2)
				{
					FT_partitions[attr1 + attr2].push_back(product[project[it]]);
					FT_piLen[attr1 + attr2] += product[project[it]].size();
				}
				product[project[it]].clear();
			}
	}
	FT_exists[attr1 + attr2] = true;
	return FT_partitions[attr1 + attr2].size();
}

void FDTANE::importData()
{
	ifstream infile(FT_input_txt_file);
	if (!infile || infile.eof())
		return;
	while (1)
	{
		string* tmp = new string[FT_attributions];
		for (int i = 0; i < FT_attributions - 1; i++)
			getline(infile, tmp[i], ',');
		getline(infile, tmp[FT_attributions - 1], '\n');
		if (tmp[0] == "")
		{
			delete[] tmp;
			break;
		}
		vector<string> str;
		for (int i = 0; i < FT_attributions; i++)
			str.push_back(tmp[i]);
		FT_table.push_back(str);
		delete[] tmp;
	}
	infile.close();
}

void FDTANE::exportData(int left, int right, int count)
{
	for (int i = 1, index = 1; i <= FT_attributions; i++, index <<= 1)
		if (left & index)
		{
			res[count].leftAdd(i);
		}
	for (int i = 1, index = 1; i <= FT_attributions; i++, index <<= 1)
		if (right & index)
		{
			res[count].rightAdd(i);
		}
}

void FDTANE::print(ostream& outputstream)
{
	int j;
	for (int i = 0; i < FT_totalCount; i++)
	{
		j = 0;
		while (res[i].getleft(j) != -1)
		{
			outputstream << res[i].getleft(j) << ' ';
			j++;
		}
		outputstream << "-> " << res[i].getright() << endl;
	}
}