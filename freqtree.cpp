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
	class node{
	public:
		node *r, *l;
		int w;
		char c;
		node(node *r, node *l) :r(r), l(l), c('\0'), w(r->w + l->w){}
		node(char c, int w) :r(NULL), l(NULL), c(c), w(w){}
		node() :r(NULL), l(NULL), c('\0'), w(0){}
		~node(){ if (this->r != NULL) delete this->r, delete this->l; }
		class comparator {
		public:
			bool operator()(const node *T1, const node *T2) const {
				return T1->w > T2->w;
			}
		};
	};
	ifstream input;
	node T;
	map<char, string> seq;
	map<char, int> freq;
	void dfs(node *T, map<char, string> &seq, string &s = string());
public:	
	~ftree();
	ftree(string file);
	void zip(string name);
	void unzip(string name);
};

void write_bchar(string input, string name){
	ofstream out(name);
	ifstream in(input);
	while (!in.eof())
	{
		char c = in.get();
		stack<char> st;
		if (in.eof())
			break;
		for (int i = 0; i < 8; i++, c/=2)
			if (c % 2 == 1)
				st.push('1');
			else
				st.push('0');
		while (!st.empty())
		{
			out << st.top();
			st.pop();
		}
	}
	out.close();
	in.close();
}

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
	ftree T(file_name);
	if (type == "zip")
		T.zip(save_name);
	else
		T.unzip(save_name);
	return 0;
}


ftree::ftree(string file) : input(file, ifstream::binary)
{
	priority_queue < node*, vector<node*>, node::comparator> q;
	char c;

	while (!input.eof())
	{
		c = input.get();
		if (input.eof())
			break;
		if (freq.count(c) == 0)
			freq[c] = 1;
		else
			freq[c]++;
	}
	input.clear();
	input.seekg(0, ios::beg);


	for (auto it = freq.begin(); it != freq.end(); it++)
	{
		node *T = new node(it->first, it->second);
		q.push(T);
	}

	while (q.size() > 1)
	{
		node *a = q.top();
		q.pop();
		node *b = q.top();
		q.pop();
		node *T = new node(a, b);
		q.push(T);
	}
	delete T.r, T.l;
	T = *q.top();
	q.top()->r = NULL; q.top()->l = NULL; // In order to delete only the top node
	delete q.top();
	dfs(&this->T, seq);
}

void ftree::dfs(node *T, map<char, string> &seq, string &s)
{
	if (T->l != NULL) // If T.l is different of NULL then, T.r is too
	{
		s.push_back('0');
		dfs(T->l, seq, s);
		s.pop_back();
		s.push_back('1');
		dfs(T->r, seq, s);
		s.pop_back();
	}
	else // Hit a char
		seq[T->c] = s;
	return;
}

void ftree::zip(string file){
	ofstream zip(file, ofstream::binary);
	vector<unsigned char> obuffer(4);
	string binary;
	int tree_size;

	input.seekg(0, input.end);
	int size = input.tellg();
	input.seekg(0);

	char *ibuffer = new char[size];
	input.read(ibuffer, size);

	string tree;
	for (auto it = seq.begin(); it != seq.end(); it++)
		tree = tree + it->first + ' ' + it->second + ' ';
	tree_size = tree.size();

	for (int i = 0; i < 4; i++)	// Insert the number of bytes spent to save the tree structure
		obuffer[3 - i] = (tree_size >> (i * 8));

	for (int i = 0; i < size; i++)
		binary += seq[ibuffer[i]];

	obuffer.push_back((unsigned char)binary.size() % 8); // Insert the number of bits of the last encipted byte

	for (auto it = tree.begin(); it != tree.end(); it++) // Insert the tree structure
		obuffer.push_back(*it);

	for (auto it = binary.begin(); it != binary.end();) // Insert EncriptedFile
	{
		unsigned char byte = 0;
		for (int i = 0; i < 8 && it != binary.end(); i++, it++)
			if (*it == '1') byte |= 1 << (7 - i);
		obuffer.push_back(byte);
	}

	zip.write((const char*) &obuffer[0], obuffer.size());
	zip.close();
}

void ftree::unzip(string name)
{
	ifstream &zip = input;
	char c;
	int i;

	zip.seekg(0, zip.end);
	long size = zip.tellg();
	zip.seekg(0);

	char *buffer = new char[size];
	zip.read(buffer, size);
	
	int tree_size = 0;
	unsigned char lastByte_size;

	for (i = 0; i < 4; i++)
		tree_size += (((unsigned char)buffer[i]) << 8*(3-i));
	lastByte_size = buffer[i++]; // i = 4 -> 5

	map<char, string> seq;

	while (i < tree_size + 5)
	{
		string s;
		char tmp;
		c = buffer[i++];
		i++; // jump space
		while ((tmp = buffer[i++]) != ' ')
			s.push_back(tmp);
		seq[c] = s;
	}
	
	node *unzipT = new node;

	for (auto it = seq.begin(); it != seq.end(); it++)
	{
		node *T = unzipT;
		for (auto c = it->second.begin(); c != it->second.end(); c++)
		{
			if (*c == '0')
			{
				if (T->l == NULL)
					T->l = new node();
				T = T->l;
			}
			else
			{
				if (T->r == NULL)
					T->r = new node();
				T = T->r;
			}
		}
		T->c = it->first;
	}

	ofstream out(name, ofstream::binary);
	node *T = unzipT;

	while (i < size)
	{
		bitset<8> byte = buffer[i++];
		if (i > size - 8)
			byte >> lastByte_size;
		string tmp = byte.to_string();
		for (int i = 0; i < 8; i++)
		{
			c = tmp[i];
			if (c == '0')
				T = T->l;
			else
				T = T->r;
			if (T->l == NULL || T->r == NULL)
			{
				out << T->c;
				T = unzipT;
			}
		}
	}

	delete unzipT;
	out.close();
}

ftree::~ftree(){
	if (input.is_open())
		input.close();
}
