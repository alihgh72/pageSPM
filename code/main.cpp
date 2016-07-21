#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>


//TODO check return instruction or call

using namespace std;

#define ADDR_SPACE 500

void initial();
unsigned int decodeInputIns(string tmp);
unsigned int decodeInputBranch(string tmp);
void readAssembly();
void createBasicBlock();
void createBlocks(int);
class BB
{
public:
	unsigned int start;
	unsigned int end;
	unsigned int size;
	BB* next1;
	BB* next2;

	BB::BB()
	{
		start = 0;
		end = 0;
		size = 0;
		next1 = NULL;
		next2 = NULL;
	}

	BB::~BB()
	{
		if (next1 != NULL)
		{
			delete next1;
			next1 = NULL;
		}
		if (next2 != NULL)
		{
			delete next2;
			next2 = NULL;
		}
	}
};

unsigned int addrIns[ADDR_SPACE];
unsigned int targetAddr[ADDR_SPACE];
BB* startBlock[ADDR_SPACE];
int block_size = 40;
unsigned int minAddr = 0;
unsigned int maxAddr = 0;

int main()
{
	initial();
	readAssembly();
	createBasicBlock();
	createBlocks(block_size);

	cout << "codesize = " << maxAddr - minAddr << "Byte \n";
	int i = 0;
	int cntr = 1;
	while (i < ADDR_SPACE && addrIns[i] != 0)
	{
		if (startBlock[i] != NULL)
		{
			cout << cntr << "\t" << startBlock[i]->start << "\t" << startBlock[i]->size << "\t" << startBlock[i]->next1<< "\t" << startBlock[i]->end << endl;
			cntr++;
		}
		i++;
	}
	//for (int i = 0; i < 100; i++)
		//cout << i << "\t" << addrIns[i] << endl;
	getchar();
	return 0;
}

void initial()
{
	for (int i = 0; i < ADDR_SPACE; i++)
	{
		addrIns[i] = 0;
		targetAddr[i] = 0;
		startBlock[i] = NULL;
	}
	block_size = 40;
	minAddr = 0;
	maxAddr = 0;

}

void readAssembly()
{
	//loc = location + inputFilename;
	string loc = "input1.txt";
	ifstream fin;
	fin.open(loc);
	if (!fin){
		cerr << "\n";
		cerr << "  Fatal error!\n";
		cerr << "  Cannot open the input assembly file!\n";
		getchar();
		exit(1);
	}

	
	loc = "output.txt";
	ofstream fout;
	fout.open(loc);
	if (!fin){
		cerr << "\n";
		cerr << "  Fatal error!\n";
		cerr << "  Cannot open output file!\n";
		getchar();
		exit(1);
	}

	string tmp;
	string cmd_hex;
	string cmd_str;
	string branch_addr;

	while (fin >> tmp)
	{
		unsigned int x, x1;
		x = decodeInputIns(tmp);
		if (x != -1)
		{
			if (minAddr == 0)
				minAddr = x;
			maxAddr = x;
			unsigned int index = (x - minAddr) / 4;
			fout << tmp.substr(0,6) << " " << x;
			addrIns[index] = x;

			fin >> cmd_hex;
			fin >> cmd_str;
			fin >> branch_addr;

			x1 = decodeInputBranch(branch_addr);
			if (x1 != -1)
			{
				fout << "\tbranch\t" << branch_addr << " " << x1;
				targetAddr[index] = x1;
			}
			fout << endl;
		}
	}

	fin.close();
	fout.close();
	return;
}

void createBasicBlock()
{
	int ins_cnt_total = (maxAddr - minAddr) / 4;

	startBlock[0] = new BB();
	startBlock[0]->start = addrIns[0];
	cout << "new block at address " << addrIns[0] << endl;

	for (int i = 0; i <= ins_cnt_total; i++)
	{
		if (targetAddr[i] != 0)
		{
			int t = (targetAddr[i] - minAddr) / 4;
			if (startBlock[t] == NULL)
			{
				startBlock[t] = new BB();
				startBlock[t]->start = targetAddr[i];
				cout << "new block at address " << targetAddr[i] << endl;
			}
		}
	}
}

void createRealBasicBlock()
{
	int ins_cnt_total = (maxAddr - minAddr) / 4;

	startBlock[0] = new BB();
	startBlock[0]->start = addrIns[0];
	cout << "new block at address " << addrIns[0] << endl;

	for (int i = 0; i <= ins_cnt_total; i++)
	{
		if (targetAddr[i] != 0)
		{
			int t = (targetAddr[i] - minAddr) / 4;
			if (startBlock[t] == NULL)
			{
				startBlock[t] = new BB();
				startBlock[t]->start = targetAddr[i];
				cout << "new block at address " << targetAddr[i] << endl;
			}

			t = (addrIns[i + 1] - minAddr) / 4;
			if (startBlock[t] == NULL)
			{
				startBlock[t] = new BB();
				startBlock[t]->start = addrIns[i + 1];
				cout << "new block at address " << addrIns[i + 1] << endl;
			}
		}
	}
}

unsigned int decodeInputIns(string tmp)
{
	unsigned int x = -1;
	if (tmp.size() == 7 && tmp.find_first_not_of("0123456789abcdef:") == std::string::npos && tmp[6] == ':')
	{
		tmp = tmp.substr(0,6);
		std::stringstream ss;
		ss << std::hex << tmp;
		ss >> x;
	}
	return x;
}

unsigned int decodeInputBranch(string tmp)
{
	unsigned int x = -1;
	if (tmp.size() == 6 && tmp.find_first_not_of("0123456789abcdef") == std::string::npos)
	{
		tmp = tmp.substr(0, 6);
		std::stringstream ss;
		ss << std::hex << tmp;	
		ss >> x;
	}
	return x;
}
void createBlocks(int block_size)
{

	int ins_cnt_total = (maxAddr - minAddr) / 4;
	BB* curr = startBlock[0];


	while (1)
	{
		curr->size = block_size;
		for (int i = 0; i < curr->size; i++)
		{
			int x = (curr->start - minAddr) / 4 + i;
			if (startBlock[x] != NULL && i != 0)
			{
				curr->next1 = NULL;
				curr->size = i - 1;
				i = 0;
				cout << curr->start << " shrink block size " << curr->size << endl;
			}
			else if ((targetAddr[x] != 0) && ((targetAddr[x] > curr->start + 4 * curr->size) || (targetAddr[x] < curr->start)))
			{
				int t = (targetAddr[x] - minAddr) / 4;
				if (curr->next1 == NULL)
				{
					if (startBlock[t] == NULL)
					{
						cout << "ERROR: there is target that is NULL\n";
						getchar();
						return;
					}
					curr->next1 = startBlock[t];
					cout << curr->start << "next1 is " << curr->next1->start << endl;
				}
				else
				{
					//TODO if its unconditional branch its ok
					curr->next1 = NULL;
					curr->size = i - 1;
					i = 0;
					cout << curr->start << " shrink block size " << curr->size << endl;
				}
			}
		}
		curr->end = curr->start + curr->size * 4;
		if (curr->end > maxAddr)
		{
			curr->end = maxAddr;
			curr->size = (curr->end - curr->start)/4;
			break;
		}
		int t = (curr->end - minAddr) / 4 + 1;
		if (startBlock[t] == NULL)
		{
			startBlock[t] = new BB();
			startBlock[t]->start = addrIns[t];
			cout << "new block at address " << addrIns[t] << endl;
		}
		curr = startBlock[t];
	}
}