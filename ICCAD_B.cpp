#include <stdio.h>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
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

    if (inf)
    {
        string lineStr;
        while (getline(inf, lineStr))
        {
            istringstream iss(lineStr);
            vector<string> words;
            string word;

            while (iss >> word)
            {
                words.push_back(word);
            }
            for (const auto &w : words)
            {
                cout << w << endl;
            }

            cout << "---------------" << endl;
        }
    }
}