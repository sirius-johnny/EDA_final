#include <stdio.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <set>
#include <math.h>
#include <ctime>
using namespace std;

int NumTechnologies;
int DieSize_X;
int DieSize_Y;
int TopDieMaxUtil;
int BottomDieMaxUtil;

int main(int argc, char *argv[])
{
    // fstream fin;
    // fin.open("ProblemB_case1.txt", ios::in);
    // fstream fout;
    // fout.open("o.txt", ios::out);

    const char *spaceChar = " ";
    ifstream inf("ProblemB_case1.txt");
    string lineStr;
    if (inf)
    {
        while (getline(inf, lineStr))
        {
            cout << lineStr << endl; // 输出每一行内容
            istringstream iss(lineStr);
            vector<string> words;
            string word;
        }
    }
}