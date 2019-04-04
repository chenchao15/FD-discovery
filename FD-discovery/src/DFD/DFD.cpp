#include "DFD.h"

bool resCmp(RESULT a, RESULT b)
{
	int i = 0;
	while(1)
	{
		if(a.getleft(i) == b.getleft(i) && a.getleft(i) == -1)
			return a.getright() < b.getright();
		else if(a.getleft(i) == b.getleft(i))
		{
			i++;
			continue;
		}
		else
			return a.getleft(i) < b.getleft(i);
	}
}

vector<int> setSubtraction(vector<int> S, vector<int> P)
{
	vector<int> res;
	map<int, bool> mp;
	for (auto p : P)
		mp[p] = true;
	for (auto s : S)
		if (mp.find(s) == mp.end())
			res.push_back(s);
	return res;
}

vector<int> setSubtraction2(vector<int> S, set<int> P)
{
	vector<int> res;
	map<int, bool> mp;
	for (auto p : P)
		mp[p] = true;
	for (auto s : S)
		if (mp.find(s) == mp.end())
			res.push_back(s);
	return res;
}

vector<int> minimize(vector<int> new_seeds)
{
	vector<int> res;
	int len = new_seeds.size();
	vector<bool> flag(len, false);
	for (int i = 0; i < len; i++)
		if (!flag[i])
			for (int j = i; j < len; j++)
			{
				if (new_seeds[i] & new_seeds[j] == new_seeds[j])
					flag[i] = true;
				if (new_seeds[i] & new_seeds[j] == new_seeds[i])
					flag[j] = true;
			}
	for (int i = 0; i < len; i++)
		if (!flag[i])
			res.push_back(new_seeds[i]);
	return res;
}

DFD::DFD(string infile, string outfile, int num):
	DFD_attributions(num)
{
	DFD_input_txt_file = infile;
	DFD_output_txt_file = outfile;
	importData();
	DFD_totalCount = 0;
	int total_key = (1 <<  DFD_attributions);
	DFD_attrCombination = (1 << DFD_attributions) - 1;
	DFD_partitions.assign(total_key, vector<vector<int> >());
	DFD_visited.assign(total_key, 0);
	DFD_type.assign(total_key, 0);
	DFD_exists.assign(total_key, 0);
	DFD_piLen.assign(total_key, 0);
	DFD_RHS.assign(total_key, 0);
	DFD_levels.assign(DFD_attributions, set<int>());
	for (int i = 0, index = 1; i < DFD_attributions; i++, index <<= 1)
	{
		DFD_RHS[index] = total_key - 1;
		DFD_levels[0].insert(index);
	}
}

void DFD::generateFD()
{
	ofstream outfile(DFD_output_txt_file);
	
	time_t begin_time, end_time;
	begin_time = clock();

/*	for (int level = 1; level < DFD_attributions; level++)
	{
		generateNextLevel(level);
		computeDependencies(level, outfile);
	}
*/

	for (int i = 0, attr = 1; i < DFD_attributions; i++, attr <<= 1)
	{
		if (isUnique(attr))
		{
			DFD_attrCombination -= attr;
			for (int j = i + 1, single = 1; j < DFD_attributions; j++, single <<= 1)
				if (single & DFD_attrCombination)
					DFD_dependencies.push_back(make_pair(attr, single));
		}
	}
	for (int i = 0, attr = 1; i < DFD_attributions; i++, attr <<= 1)
	{
		if (attr & DFD_attrCombination)
		{
			variableClear();
			findLHS(attr);
			for (auto k : DFD_minDeps)
				DFD_dependencies.push_back(make_pair(k, attr));
		}
	}

	end_time = clock();
	cout << "函数依赖成功生成……" << endl;
	cout << "共包含" << DFD_dependencies.size() << "个函数依赖项" << endl;
	cout << "运行时间：" << (double)(end_time - begin_time) / CLOCKS_PER_SEC << "s" << endl;
	
	int len = DFD_dependencies.size();
	for (int i = 0; i < len; i++)
	{
		int left = DFD_dependencies[i].first;
		int right = DFD_dependencies[i].second;
		for (int i = 1, index = 1; i <= DFD_attributions; i++, index <<= 1)
			if (left & index)
				cout << index << " ";
		cout << "-> " << right << endl;
	}
	
	//sort(res,res+DFD_totalCount,resCmp);
	//print(outfile);
	outfile.close();
}

void DFD::variableClear()
{
	int total_key = (1 <<  DFD_attributions);
	DFD_visited.assign(total_key, 0);
	DFD_type.assign(total_key, 0);
	DFD_minDeps.clear();
	DFD_maxNonDeps.clear();
	while(!DFD_trace.empty())
      DFD_trace.pop();
}

void DFD::findLHS(int attr)
{
	vector<int> seeds;
	int attr_remain = DFD_attrCombination;
	if (attr & DFD_attrCombination)
		attr_remain -= attr;
	for (int i = 0, single = 1; i < DFD_attributions; i++, single <<= 1)
		if (single & attr_remain)
			seeds.push_back(single);
	int node = 0;
	while(!seeds.empty())
	{
		node = seeds[0];      //  node <- pickSeed()
		do
		{
			if (DFD_visited[node])
			{
				if (DFD_type[node] == canmindep)
				{
					DFD_minDeps.insert(node);
					DFD_type[node] = mindep;
				}
				else if (DFD_type[node] == canmaxnondep)
				{
					DFD_maxNonDeps.insert(node);
					DFD_type[node] = maxnondep;
				}
			}
			else
			{
				DFD_type[node] = inferCategory(node);
				if (DFD_type[node] == nulltype)
					DFD_type[node] = isValid(node + attr, attr) ? canmindep : canmaxnondep;
			}
			DFD_visited[node] = 1;
			node = pickNextNode(attr, node);   // node   finally 变成了4095，导致上面isValid的node+attr超过最大值
		} while(node > 0);
		seeds = generateNextSeeds();
	}
}

int DFD::inferCategory(int node)
{
	for (auto md : DFD_minDeps)
		if (md & node == md)
			return dep;
	for (auto mnd : DFD_maxNonDeps)
		if (mnd & node == node)
			return nondep;
	return nulltype;
}

int DFD::pickNextNode(int attr, int node)
{
	vector<int> S, P;
	int next_node = 0;
	if (DFD_type[node] == canmindep)
	{
		S = uncheckedSubsets(node);
		P = prunedSets(node, S);
		S = setSubtraction(S, P);
		if (S.empty())
		{
			DFD_minDeps.insert(node);
			DFD_type[node] = mindep;
			return 0;
		}
		else
		{
			next_node = S[0];
			DFD_trace.push(node);
			return next_node;
		}
	}
	else if (DFD_type[node] == canmaxnondep)
	{
		S = uncheckedSupersets(node);
		P = prunedSupersets(node, S);
		S = setSubtraction(S, P);
		if (S.empty())
		{
			DFD_maxNonDeps.insert(node);
			DFD_type[node] = maxnondep;
		}
		else
		{
			next_node = S[0];
			DFD_trace.push(node);
			return next_node;
		}
	}
	else
	{
		next_node = DFD_trace.top();
		DFD_trace.pop();
		return next_node;
	}
}

vector<int> DFD::uncheckedSubsets(int node)
{
	vector<int> res;
	for (int i = 0, single = 1; i < DFD_attributions; i++, single <<= 1)
		if (single & node && single != node && !DFD_visited[node - single])
			res.push_back(node - single);
	return res;
}

vector<int> DFD::uncheckedSupersets(int node)
{
	vector<int> res;
	for (int i = 0, single = 1; i < DFD_attributions; i++, single <<= 1)
		if (!(single & node) && !DFD_visited[node + single])
			res.push_back(node + single);
	return res;
}

vector<int> DFD::prunedSets(int node, vector<int> S)
{
	vector<int> res;
	for (auto s : S)
		for (auto mnd : DFD_maxNonDeps)
			if (s & mnd == s)
				res.push_back(s);
	return res;
}

vector<int> DFD::prunedSupersets(int node, vector<int> S)
{
	vector<int> res;
	for (auto s : S)
		for (auto md : DFD_minDeps)
			if (s & md == md)
				res.push_back(s);
	return res;
}

vector<int> DFD::generateNextSeeds()
{
	vector<int> seeds, new_seeds;
	for (auto mnd : DFD_maxNonDeps)
	{
		int cmnd = (1 << DFD_attributions) - 1 - mnd;
		if (seeds.empty())
		{
			for (int i = 0, single = 1; i < DFD_attributions; i++, single <<= 1)
				if (single & cmnd)
					seeds.push_back(single);
			sort(seeds.begin(),seeds.end());
			seeds.erase(unique(seeds.begin(), seeds.end()), seeds.end());
		}
		else
		{
			for (auto dep : seeds)
				for (int i = 0, single = 1; i < DFD_attributions; i++, single <<= 1)
					if (single & cmnd)
						new_seeds.push_back(single | dep);
			sort(new_seeds.begin(),new_seeds.end());
			new_seeds.erase(unique(new_seeds.begin(), new_seeds.end()), new_seeds.end());
			vector<int> minnewdeps = minimize(new_seeds);
			seeds.clear();
			for ( auto new_seed : minnewdeps )
				seeds.push_back(new_seed);
			new_seeds.clear();
		}
	}
	seeds = setSubtraction2(seeds, DFD_minDeps);
	return seeds;
}

bool DFD::isUnique(int attr)
{
	map<string, bool> mp;
	int tmp = attr, col = 0;
	for (; col < DFD_attributions && !(tmp % 2); col++, tmp >>= 1);
	for (int i = 0; i < DFD_table.size(); i++)
	{
		string str = DFD_table[i][col];
		if (mp.find(str) != mp.end())
			return false;
		else
			mp[str] = true;
	}
	return true;
}

void DFD::generateNextLevel(int level)
{
	if (level == 0)
		return;
	for (auto pre_index : DFD_levels[level - 1])
		for (int i = 0, index = 1; i < DFD_attributions; i++, index <<= 1)
			if (!(pre_index & index))
				DFD_levels[level].insert(pre_index + index);
}

void DFD::computeDependencies(int level, ostream& outputstream)
{
	set<int>::iterator set_end = DFD_levels[level].end();
	for (set<int>::iterator set_it = DFD_levels[level].begin(); set_it != set_end;)
	{
		int it = *set_it;
		int rhs = (1 << DFD_attributions) - 1;
		for (int i = 0, single_attr = 1; i < DFD_attributions; i++, single_attr <<= 1)
			if (it & single_attr)
				rhs = rhs & DFD_RHS[it - single_attr];
		DFD_RHS[it] = rhs;
		int composite_attr = it & DFD_RHS[it];
		for (int i = 0, single_attr = 1; i < DFD_attributions; i++, single_attr <<= 1)
		{
			if (composite_attr & single_attr && isValid(it, single_attr))
			{
				DFD_totalCount++;
				exportData(it - single_attr, single_attr, DFD_totalCount - 1);
				removeAttr(it, single_attr);
			}
		}
		if (DFD_RHS[it] == 0)
			set_it = DFD_levels[level].erase(set_it);
		else
			set_it++;
	}
}

bool DFD::isValid(int it, int attr)
{
	/*int pi1, pi2;
	if(DFD_exists[it-attr]) pi1 = DFD_partitions[it-attr].size();
	else pi1 = computePi(it-attr);
	if(DFD_exists[it]) pi1 =  DFD_partitions[it].size();
	else pi1 = computePiProduct(it - attr, attr);*/
	int pi1 = computePi(it - attr);
	int pi2 = computePiProduct(it - attr, attr);
	return (DFD_piLen[it - attr] - pi1 == DFD_piLen[it] - pi2);
}

int DFD::computePi(int attr)
{
	if (DFD_exists[attr])
		return DFD_partitions[attr].size();
	int tmp = attr, col = 0;
	for (; col < DFD_attributions && !(tmp % 2); col++, tmp >>= 1);
	vector<vector<int> > store;
	unordered_map<string, int> hp;
	unordered_map<string, int>::const_iterator hp_it;
	for (int i = 0, index = 0; i < (int)DFD_table.size(); i++)
	{
		string substr = DFD_table[i][col];
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
			DFD_partitions[attr].push_back(it);
			DFD_piLen[attr] += it.size();
		}
	DFD_exists[attr] = true;
	return DFD_partitions[attr].size();
}

void DFD::removeAttr(int it, int attr)
{
	DFD_RHS[it] -= attr;
	int comple = (1 << DFD_attributions) - 1 - it;
	for (int i = 0, single_attr = 1; i < DFD_attributions; i++, single_attr <<= 1)
		if (comple & single_attr && DFD_RHS[it] & single_attr)
			DFD_RHS[it] -= single_attr;
}

int DFD::computePiProduct(int attr1, int attr2)
{
	if (DFD_exists[attr1 + attr2])
		return DFD_partitions[attr1 + attr2].size();
	int pi1 = computePi(attr1), pi2 = computePi(attr2);
	vector<int> project(DFD_table.size(), -1);
	vector<vector<int> > product(pi1, vector<int>());
	for (int i = 0; i < pi1; i++)
		for (auto it : DFD_partitions[attr1][i])
			project[it] = i;
	for (int i = 0; i < pi2; i++)
	{
		for (auto it : DFD_partitions[attr2][i])
			if (project[it] >= 0)
				product[project[it]].push_back(it);
		for (auto it : DFD_partitions[attr2][i])
			if (project[it] >= 0)
			{
				if (product[project[it]].size() >= 2)
				{
					DFD_partitions[attr1 + attr2].push_back(product[project[it]]);
					DFD_piLen[attr1 + attr2] += product[project[it]].size();
				}
				product[project[it]].clear();
			}
	}
	DFD_exists[attr1 + attr2] = true;
	return DFD_partitions[attr1 + attr2].size();
}

void DFD::importData()
{
	ifstream infile(DFD_input_txt_file);
	if (!infile || infile.eof())
		return;
	while (1)
	{
		string* tmp = new string[DFD_attributions];
		for (int i = 0; i < DFD_attributions - 1; i++)
			getline(infile, tmp[i], ',');
		getline(infile, tmp[DFD_attributions - 1], '\n');
		if (tmp[0] == "") 
		{
			delete[] tmp;
			break;
		}
		vector<string> str;
		for (int i = 0; i < DFD_attributions; i++)
			str.push_back(tmp[i]);
		DFD_table.push_back(str);
		delete[] tmp;
	}
	infile.close();
}

void DFD::exportData(int left, int right, int count)
{
	for (int i = 1, index = 1; i <= DFD_attributions; i++, index <<= 1)
		if (left & index)
		{
			res[count].leftAdd(i);
		}
	for (int i = 1, index = 1; i <= DFD_attributions; i++, index <<= 1)
		if (right & index)
		{
			res[count].rightAdd(i);
		}
}

void DFD::print(ostream& outputstream)
{
	int j;
	for(int i = 0; i < DFD_totalCount; i++)
	{
		j = 0;
		while(res[i].getleft(j)!=-1)
		{
			outputstream << res[i].getleft(j) << ' ';
			j++;
		}
		outputstream << "-> " << res[i].getright() << endl;
	}
}
