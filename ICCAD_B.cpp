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

// 讀入.txt的資料們
string NumTechnologies;
int num_LibCell;
int DieSize_LL_X, DieSize_LL_Y, DieSize_UR_X, DieSize_UR_Y;
int TopDieMaxUtil, BottomDieMaxUtil;
int TopDieRows_X, TopDieRows_Y, TopDieRows_row_len, TopDieRows_row_height, TopDieRows_repeat_count;
int BottomDieRows_X, BottomDieRows_Y, BottomDieRows_row_len, BottomDieRows_row_height, BottomDieRows_repeat_count;
string TopDieTech, BottomDieTech;
int TerminalSize_X, TerminalSize_Y, TerminalSpacing, TerminalCost;
int NumInstances;
int NumNets;

// 其他全域

typedef struct
{
    string pinName;
    int pinLocationX, pinLocationY;
} Pin;

class Instance
{
    bool isMacro;
    string libCellName;
    int libCellSizeX, libCellSizeY, pinCount;
    Pin *pin;
    Instance(bool isMacro, string libCellName, int libCellSizeX, int libCellSizeY, int pinCount)
    {
        int index = stoi(TA[0].Name.erase(0, 2)) - 1;
        pin = TA[index].pin;
    }
};

struct LibCell
{
    bool is_Macro;
    string Name;
    int size_X;
    int size_Y;
    int Pin_count;
    Pin *pin;
};

LibCell *TA, *TB;

int main(int argc, char *argv[])
{
    fstream fin;
    fin.open("ProblemB_case1.txt", ios::in);
    fstream fout;
    fout.open("o.txt", ios::out);

    if (fin)
    {
        string lineStr;
        // NumTechnologies
        getline(fin, lineStr);
        NumTechnologies = lineStr[lineStr.length() - 1];
        // Tech
        getline(fin, lineStr);
        istringstream iss(lineStr);
        vector<string> words;
        string word;
        while (iss >> word)
        {
            words.push_back(word);
        }
        num_LibCell = stoi(words[2]);
        TA = new LibCell[num_LibCell];
        for (int i = 0; i < num_LibCell; i++)
        {
            words.clear();
            getline(fin, lineStr);
            istringstream iss(lineStr);
            while (iss >> word)
            {
                words.push_back(word);
            }

            TA[i].is_Macro = (words[1] == "N") ? 0 : 1;
            TA[i].Name = words[2];
            TA[i].size_X = stoi(words[3]);
            TA[i].size_Y = stoi(words[4]);
            TA[i].Pin_count = stoi(words[5]);
            TA[i].pin = new Pin[TA[i].Pin_count];
            for (int j = 0; j < TA[i].Pin_count; j++)
            {
                words.clear();
                getline(fin, lineStr);
                istringstream iss(lineStr);
                while (iss >> word)
                {
                    words.push_back(word);
                }
                TA[i].pin[j].pinName = words[1];
                TA[i].pin[j].pinLocationX = stoi(words[2]);
                TA[i].pin[j].pinLocationY = stoi(words[3]);
            }
        }

        if (NumTechnologies == "2")
        {
            getline(fin, lineStr);
            TB = new LibCell[num_LibCell];
            for (int i = 0; i < num_LibCell; i++)
            {
                words.clear();
                getline(fin, lineStr);
                istringstream iss(lineStr);
                while (iss >> word)
                {
                    words.push_back(word);
                }

                TB[i].is_Macro = (words[1] == "N") ? 0 : 1;
                TB[i].Name = words[2];
                TB[i].size_X = stoi(words[3]);
                TB[i].size_Y = stoi(words[4]);
                TB[i].Pin_count = stoi(words[5]);
                TB[i].pin = new Pin[TB[i].Pin_count];
                for (int j = 0; j < TB[i].Pin_count; j++)
                {
                    words.clear();
                    getline(fin, lineStr);
                    istringstream iss(lineStr);
                    while (iss >> word)
                    {
                        words.push_back(word);
                    }
                    TB[i].pin[j].pinName = words[1];
                    TB[i].pin[j].pinLocationX = stoi(words[2]);
                    TB[i].pin[j].pinLocationY = stoi(words[3]);
                }
            }
        }

        else
        {
            TB = TA;
        }
    }
}