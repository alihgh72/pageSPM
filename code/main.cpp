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
void createBasicBlock(int);

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
int block_size = 10;
unsigned int minAddr = 0;
unsigned int maxAddr = 0;


int main()
{
	initial();
	readAssembly();
	createBasicBlock(block_size);

	cout << "codesize = " << maxAddr - minAddr << "Byte \n";
	int i = 0;
	int cntr = 1;
	while (i < ADDR_SPACE && addrIns[i] != 0)
	{
		if (i == 0 || startBlock[i] != startBlock[i-1])
		{
			//cout << cntr << "\t" << startBlock[i] << endl;
			cout << cntr << "\t" << startBlock[i]->start << "\t" << startBlock[i]->size << "\t" << startBlock[i]->next1<< "\t" << startBlock[i]->end << endl;
			cntr++;
		}
		i++;
	}
	

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
	minAddr = 0;
	maxAddr = 0;

}

void readAssembly()
{
	// TODO : read trace file also and 
	// find branch instruction 
	// and instruction like pc = rl
	// for return or CALL

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

void createBasicBlock(int block_size)
{
	int ins_cnt_total = (maxAddr - minAddr) / 4;

	startBlock[0] = new BB();
	startBlock[0]->start = addrIns[0];
	BB* curr = startBlock[0];
	cout << "new block at address " << addrIns[0] << endl;

	while (1)
	{
		curr->size = block_size;
		int branch = 0;
		for (unsigned int i = 0; i < curr->size; i++)
		{
			int x = (curr->start - minAddr) / 4 + i;
			startBlock[x] = curr;
			curr->end = curr->start + curr->size * 4;
			if ((targetAddr[x] != 0) && ((targetAddr[x] > curr->start + 4 * curr->size) || (targetAddr[x] < curr->start)))
			{
				branch++;
				if (branch > 1)
				{
					curr->size = i - 1;
					i = 0;
					branch = 0;
					cout << curr->start << " shrink block size " << curr->size << endl;
				}
			}

			for (unsigned int j = 0; j <= ins_cnt_total; j++)
			{
				if ((addrIns[x] == targetAddr[j]) && (startBlock[x]->start > targetAddr[j] || startBlock[x]->end < targetAddr[j]))
				{
					curr->size = i - 1;
					i = 0;
					branch = 0;
					cout << curr->start << " startWithjump shrink block size " << curr->size << endl;
					break;
				}
			}

		}
		

		if (curr->end > maxAddr)
		{
			curr->end = maxAddr;
			curr->size = (curr->end - curr->start) / 4;
			break;
		}

		int t = (curr->end - minAddr) / 4;
			startBlock[t] = new BB();
			startBlock[t]->start = addrIns[t];
			cout << "new block at address " << addrIns[t] << endl;
		curr->next2 = startBlock[t];
		curr = startBlock[t];

	}

	for (int i = 0; i < ins_cnt_total; i++)
	{
		int debugCntr = 0;
		unsigned int tar = (targetAddr[i] - minAddr) / 4;
		if (targetAddr[i] != 0 && startBlock[i] != startBlock[tar])
		{
			debugCntr++;
			if (debugCntr > 1)
				cout << "DEBUG_ERR: Basic Block " << startBlock[i]->start << " has more than one branch!!\n";
			startBlock[i]->next1 = startBlock[tar];
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
