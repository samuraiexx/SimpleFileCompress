#include <vector>
#include <fstream>
#include <map>
#include <queue>
#include <utility>
#include <algorithm>
#include <stack>
#include <functional>
#include <string>
#include <bitset>
#include <iostream>

using namespace std;


class ftree{
	class vnode{
	public:
		int l, r; // left child, right child
		vnode(int r, int l) : l(l), r(r){}
		vnode(){}
	};
	class q_vnode{
	public:
		int w, p;
		vnode vn;
		bool operator>(const q_vnode &v2)const{ return w > v2.w; }
		q_vnode(int w, int p, vnode vn) : w(w), p(p), vn(vn){}
		q_vnode(){}
	};
	static void dfs(const int p, char **seq_unt, vnode *T, map<string, string> &seq, const int &seq_size, string &s = string(""));
	static void cut_file(int seq_size, string file, vector<string> &cutted);
	static int gen_bin(string &buffer, map<string, string> &seq, const string &srcf, const int &seq_size);
public:
	static void zip(string srcf, string zipf, int seq_size);
	static void unzip(string src, string dest);
};

int main(){
	string type, file_name, save_name;
	do{
		cout << "Write \"zip\" to compress a file or \"unzip\" to decompress" << endl;
		cin >> type;
	} while (type != "zip" && type != "unzip");
	cout << endl << "What is the name of the file that will be compressed or decompressed?" << endl;
	cin >> file_name;
	cout << endl << "And what is the name you want to save the new file with?" << endl;
	cin >> save_name;
	if (type == "zip")
		ftree::zip(file_name, save_name, 1); // 1 is the number of bytes per leaf
	else
		ftree::unzip(file_name, save_name);
	return 0;
}

void ftree::dfs(const int p, char **seq_unt, vnode *tree, map<string, string> &seq, const int &seq_size, string &s)
{
	if (tree[p].l > 0) // If tree.l is negative, it's a leaf
	{
		s.push_back('0');
		dfs(tree[p].l, seq_unt, tree, seq, seq_size, s);
		s.pop_back();
	}
	else // Hit a char
		seq[string(seq_unt[-1 - tree[p].l], seq_unt[-1 - tree[p].l] + seq_size)] = s + '0';

	if (tree[p].r > 0)
	{
		s.push_back('1');
		dfs(tree[p].r, seq_unt, tree, seq, seq_size, s);
		s.pop_back();
	}
	else
		seq[string(seq_unt[-1-tree[p].r], seq_unt[-1-tree[p].r] + seq_size)] = s + '1';
}
void ftree::zip(string srcf, string zipf, int seq_size)
{
	map<string, string> seq;
	map<string, int> freq;
	vector<pair<int, string>> sortted_freq;
	priority_queue < q_vnode, vector<q_vnode>, greater<q_vnode>> q;
	ofstream zip; ifstream src;
	vector<vnode> tree(1);
	char **seq_unt;
	int *w_seq_unt;

	src.open(srcf, ifstream::binary);
	while (!src.eof())
	{
		string c;
		char u;
		for (int i = 0; i < seq_size; i++)
		{
			u = src.get();
			if (src.eof())
				break;
			c.push_back(u);
		}
		if (src.eof()) break;
		if (freq.count(c) == 0)
			freq[c] = 1;
		else
			freq[c]++;
	}
	src.close();

	seq_unt = new char*[freq.size()];
	w_seq_unt = new int[freq.size()];

	for (auto fq : freq)
		sortted_freq.push_back(make_pair(fq.second, fq.first));
	sort(sortted_freq.begin(), sortted_freq.end());
	
	int i = 0;
	for (auto fq : sortted_freq)
	{
		seq_unt[i] = new char[seq_size];
		for (int j = 0; j < seq_size; j++)
			seq_unt[i][j] = fq.second[j];
		w_seq_unt[i] = fq.first;
		i++;
	}

	i = 0;
	while (true)
	{
		q_vnode tmp1, tmp2;
		for (q_vnode* tmp : { &tmp1, &tmp2 }){
			if (q.empty() || (i < freq.size() && q.top().w > w_seq_unt[i]))
			{
				tmp->w = w_seq_unt[i];
				tmp->p = - 1 - i;
				i++;
			}
			else
			{
				*tmp = q.top();
				q.pop();
			}
		}
		if (q.size() > 0 || i < freq.size())
		{
			tree.push_back(vnode(tmp1.p, tmp2.p));
			q.push(q_vnode(tmp1.w + tmp2.w, tree.size() - 1, *(tree.end() - 1)));
		}
		else
		{
			tree[0] = vnode(tmp1.p, tmp2.p);
			break;
		}
	}

	dfs(0, seq_unt, &tree[0], seq, seq_size);

	string bin;
	int lb_size = gen_bin(bin, seq, srcf, seq_size);

	zip.open(zipf, ofstream::binary);
	int unt_amount = freq.size();
	int tree_size = tree.size();

	zip.write((char*)&seq_size, sizeof(int));
	zip.write((char*)&unt_amount, sizeof(int));
	zip.write((char*)&tree_size, sizeof(int));
	zip.write((char*)&lb_size, sizeof(int));

	for (int i = 0; i < unt_amount; i++)
		zip.write(seq_unt[i], seq_size);
	zip.write((char*)&tree[0], tree_size*sizeof(vnode));
	zip.write((char*)&bin[0], bin.size());
	zip.close();
	delete[] w_seq_unt;
	for (int i = 0; i < unt_amount; i++)
		delete[] seq_unt[i];
	delete[] seq_unt;
}

void ftree::unzip(string src, string dest)
{
	int seq_size, unt_amount, tree_size, lb_size;
	vnode *tree;
	char c;
	char **seq_unt;
	string buffer, out;

	ifstream zip(src, ifstream::binary);
	zip.read((char*)&seq_size, sizeof(int));
	zip.read((char*)&unt_amount, sizeof(int));
	zip.read((char*)&tree_size, sizeof(int));
	zip.read((char*)&lb_size, sizeof(int));
	seq_unt = new char*[unt_amount];
	for (int i = 0; i < unt_amount; i++)
	{
		seq_unt[i] = new char[seq_size];
		zip.read(seq_unt[i], seq_size);
	}
	tree = new vnode[tree_size];
	zip.read((char*)tree, tree_size*sizeof(vnode));
	while (true){
		c = zip.get();
		if (zip.eof()) break;
		buffer.push_back(c);
	}
	zip.close();

	int p = 0;
	for (int i = 0; i < buffer.size(); i++)
	{
		int b_size = 8;
		bitset<8> byte = buffer[i];
		string s_byte = byte.to_string();
		if (i == buffer.size() - 1 && lb_size > 0)
			b_size = lb_size;
		for (int j = 0; j < b_size; j++)
		{
			char b = s_byte[j];
			if (b == '0')
			{
				if (tree[p].l > 0)
					p = tree[p].l;
				else
				{
					for (int k = 0; k < seq_size; k++)
						out.push_back(seq_unt[-1-tree[p].l][k]);
					p = 0;
				}
			}
			else
			{
				if (tree[p].r > 0)
					p = tree[p].r;
				else
				{
					for (int i = 0; i < seq_size; i++)
						out.push_back(seq_unt[-1-tree[p].r][i]);
					p = 0;
				}
			}
		}
	}

	ofstream unzipped(dest, ofstream::binary);
	unzipped.write(&out[0], out.size());
	unzipped.close();
	for (int i = 0; i < unt_amount; i++)
		delete[] seq_unt[i];
	delete[] seq_unt;
	delete[] tree;
}

void ftree::cut_file(int seq_size, string file, vector<string> &cutted)
{
	ifstream src(file, ofstream::binary);

	src.seekg(0, src.end);
	int size = src.tellg();
	src.seekg(0);

	char *buffer = new char[size];
	src.read(buffer, size);
	src.close();

	for (int i = 0; i < size;)
	{
		string tmp;
		for (int j = 0; j < seq_size && i < size; j++)
			tmp.push_back(buffer[i++]);
		cutted.push_back(tmp);
	}
}

int ftree::gen_bin(string &buffer, map<string, string> &seq, const string &srcf, const int &seq_size)
{
	vector<string> cutted;
	string binary;
	cut_file(seq_size, srcf, cutted);

	for (auto c : cutted)
		binary += seq[c];

	for (auto it = binary.begin(); it != binary.end();) // Insert the compressed file in the output buffer
	{
		unsigned char byte = 0;
		for (int i = 0; i < 8 && it != binary.end(); i++, it++)
			if (*it == '1') byte |= 1 << (7 - i);
		buffer.push_back(byte);
	}
	return binary.size() % 8; // return the last byte size
}


