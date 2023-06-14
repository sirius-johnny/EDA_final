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

class Instance
{
public:
    string instName, libCellName;
    LibCell *libCell;
    int locationX, locationY, rotate;
    bool top;
    Instance(string instName, string libCellName)
    {
        this->instName = instName;
        this->libCellName = libCellName;
        top = 0;
        int index = stoi(libCellName.erase(0, 2)) - 1;
        libCell = &TA[index];
        locationX = 0;
        locationY = 0;
        rotate = 0;
    }

private:
};

const std::vector<std::string> split(const std::string &str, const char &delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string tok;

    while (std::getline(ss, tok, delimiter))
    {
        result.push_back(tok);
    }
    return result;
}
int main(int argc, char *argv[])
{
    fstream fin;
    fin.open("ProblemB_case2.txt", ios::in);
    // fstream fout;
    // fout.open("o.txt", ios::out);

    if (fin)
    {
        string lineStr;
        vector<string> words;

        // NumTechnologies
        getline(fin, lineStr);
        words = split(lineStr, ' ');
        NumTechnologies = words[1];
        // Tech
        getline(fin, lineStr);
        words = split(lineStr, ' ');
        num_LibCell = stoi(words[2]);
        TA = new LibCell[num_LibCell];
        for (int i = 0; i < num_LibCell; i++)
        {

            getline(fin, lineStr);
            words = split(lineStr, ' ');

            TA[i].is_Macro = (words[1] == "N") ? 0 : 1;
            TA[i].Name = words[2];
            TA[i].size_X = stoi(words[3]);
            TA[i].size_Y = stoi(words[4]);
            TA[i].Pin_count = stoi(words[5]);
            TA[i].pin = new Pin[TA[i].Pin_count];
            for (int j = 0; j < TA[i].Pin_count; j++)
            {

                getline(fin, lineStr);
                words = split(lineStr, ' ');
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

                getline(fin, lineStr);
                words = split(lineStr, ' ');

                TB[i].is_Macro = (words[1] == "N") ? 0 : 1;
                TB[i].Name = words[2];
                TB[i].size_X = stoi(words[3]);
                TB[i].size_Y = stoi(words[4]);
                TB[i].Pin_count = stoi(words[5]);
                TB[i].pin = new Pin[TB[i].Pin_count];
                for (int j = 0; j < TB[i].Pin_count; j++)
                {
                    getline(fin, lineStr);
                    words = split(lineStr, ' ');
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

        getline(fin, lineStr); // 22

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        DieSize_LL_X = stoi(words[1]);
        DieSize_LL_Y = stoi(words[2]);
        DieSize_UR_X = stoi(words[3]);
        DieSize_UR_Y = stoi(words[4]);

        getline(fin, lineStr); // 24

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        TopDieMaxUtil = stoi(words[1]);

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        BottomDieMaxUtil = stoi(words[1]);

        getline(fin, lineStr); // 27

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        TopDieRows_X = stoi(words[1]);
        TopDieRows_Y = stoi(words[2]);
        TopDieRows_row_len = stoi(words[3]);
        TopDieRows_row_height = stoi(words[4]);
        TopDieRows_repeat_count = stoi(words[5]);

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        BottomDieRows_X = stoi(words[1]);
        BottomDieRows_Y = stoi(words[2]);
        BottomDieRows_row_len = stoi(words[3]);
        BottomDieRows_row_height = stoi(words[4]);
        BottomDieRows_repeat_count = stoi(words[5]);

        getline(fin, lineStr); // 30

        getline(fin, lineStr);

        words = split(lineStr, ' ');
        TopDieTech = words[1];

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        BottomDieTech = words[1];

        getline(fin, lineStr); // 33

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        TerminalSize_X = stoi(words[1]);
        TerminalSize_Y = stoi(words[2]);

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        TerminalSpacing = stoi(words[1]);

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        TerminalCost = stoi(words[1]);

        getline(fin, lineStr); // 37

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        NumInstances = stoi(words[1]);
        vector<Instance> Inst;

        for (int i = 0; i < NumInstances; i++)
        {
            getline(fin, lineStr);
            words = split(lineStr, ' ');
            Instance inst(words[1], words[2]);
            // cout<<"words1="<<words[1]<<" ,words2="<<words[2]<<endl;
            Inst.push_back(inst);
            // cout << inst.instName << endl;
        }

        for (int i = 0; i < NumInstances; i++)
        {
            // cout << Inst[i].instName << " " << Inst[i].libCellName << endl;
        }
    }
}