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
int NumInstances, NumNets;
int NumMacro = 0;

// partition
vector<int>
    IA, IB;
int max_size = 0;
int max_pin = 0;
// 其他全域

// Instance 面積總和
long bot_occupied = 0;
long top_occupied = 0;

typedef struct
{
    int Pin_num;
    int Top_degree;
    int Bot_degree;
    int **Ins_Pin;
} Net;

typedef struct
{
    string pinName;
    int pinLocationX, pinLocationY;
} Pin;

struct LibCell
{
    bool is_Macro;
    string Name;
    double size_X;
    double size_Y;
    int Pin_count;
    Pin *pin;
};
LibCell *TA, *TB;
class Instance
{
public:
    string instName, libCellName;
    LibCell libCell;
    int locationX, locationY, rotate;
    bool top;
    int *nets;
    int index;
    int gain, temp_gain, sizeA, sizeB;
    Instance *next;
    bool fixed, temp_top;
    Instance(string instName, string libCellName)
    {
        this->instName = instName;
        this->libCellName = libCellName;
        top = 0;
        index = stoi(libCellName.erase(0, 2)) - 1;
        libCell = TB[index];
        // cout << &libCell << "  " << &TA[index] << " ";
        locationX = 0;
        locationY = 0;
        rotate = 0;
        nets = new int[libCell.Pin_count];
        for (int i = 0; i < libCell.Pin_count; i++)
        {
            nets[i] = -1;
        }
        gain = 0;
        temp_gain = 0;
        sizeA = TA[index].size_X * TA[index].size_Y;
        sizeB = TB[index].size_X * TB[index].size_Y;
        next = nullptr;
        fixed = 0;
        temp_top = 0;
    }
    void input_nets(int pin, int net)
    {
        nets[pin] = net;
    }
    void change_top(bool top)
    {
        this->top = top;
        this->temp_top = top;
        if (top)
            libCell = TA[index];
        else
            libCell = TB[index];
    }

private:
};
typedef struct
{
    int gain;
    Instance *c;
} Bucket;
vector<Instance> Inst;
Net *Nets;
vector<Bucket> bucketA, bucketB;
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
void partition_init() // top_occupied : bot_occupied = TopDieMaxUtil : BottomDieMaxUtil
{

    for (int i = 0; i < Inst.size(); i++)
    {
        bot_occupied = bot_occupied + Inst[i].sizeB * TopDieMaxUtil;
    }
    int macroSplit = NumMacro / 2;
    int index = 0;
    while (macroSplit > 0)
    {
        if (Inst[Inst.size() - 1 - index].libCell.is_Macro)
        {
            Inst[Inst.size() - 1 - index].top = 1;
            Inst[Inst.size() - 1 - index].temp_top = 1;
            bot_occupied -= Inst[Inst.size() - 1 - index].sizeB * TopDieMaxUtil;
            top_occupied += Inst[Inst.size() - 1 - index].sizeA * BottomDieMaxUtil;
            macroSplit--;
        }
        index++;
    }
    index = 0;
    while ((bot_occupied - top_occupied) > 0)
    {
        if (!Inst[index].libCell.is_Macro)
        {
            Inst[index].top = 1;
            Inst[index].temp_top = 1;
            bot_occupied -= Inst[index].sizeB * TopDieMaxUtil;
            top_occupied += Inst[index].sizeA * BottomDieMaxUtil;
        }
        index++;
    }
    bot_occupied /= TopDieMaxUtil;
    top_occupied /= BottomDieMaxUtil;
}
void split_half()
{
    for (int i = 0; i < NumInstances; i++)
    {
        if (i % 2 == 0)
        {
            Inst[i].change_top(1);
            IA.push_back(i);
        }
        else
        {

            IB.push_back(i);
        }
    }
    return;
}

void print_set()
{
    cout << "IA: ";
    for (int i = 0; i < IA.size(); i++)
    {
        cout << IA[i] << " ";
    }
    cout << endl
         << "IB: ";
    for (int i = 0; i < IB.size(); i++)
    {
        cout << IB[i] << " ";
    }
    cout << endl;
    return;
}
void print_gain()
{
    for (int i = 0; i < NumInstances; i++)
    {
        cout << Inst[i].temp_gain << " ";
    }
}
void initialize_gain()
{
    cout << "enter" << endl;
    for (int i = 0; i < NumNets; i++)
    {
        int NA = 0;
        int NB = 0;
        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            if (Inst[Nets[i].Ins_Pin[j][0]].top)
                NA++;
            else
                NB++;
        }
        if ((NA == 0) || (NB == 0))
        {
            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                Inst[Nets[i].Ins_Pin[j][0]].gain -= 1;
            }
        }
        if (NA == 1)
        {
            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                if (Inst[Nets[i].Ins_Pin[j][0]].top)
                    Inst[Nets[i].Ins_Pin[j][0]].gain += 1;
            }
        }
        if (NB == 1)
        {
            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                if (!Inst[Nets[i].Ins_Pin[j][0]].top)
                    Inst[Nets[i].Ins_Pin[j][0]].gain += 1;
            }
        }
    }
    for (int i = 0; i < NumInstances; i++)
    {
        Inst[i].temp_gain = Inst[i].gain;
    }
    bucketA.clear();
    for (int i = 0; i < 2 * max_pin + 1; i++)
    {
        Bucket b;
        b.gain = max_pin - i;
        b.c = nullptr;
        bucketA.push_back(b);
    }

    bucketB.clear();
    for (int i = 0; i < 2 * max_pin + 1; i++)
    {
        Bucket b;
        b.gain = max_pin - i;
        b.c = nullptr;
        bucketB.push_back(b);
    }

    vector<Instance *> pointerA, pointerB;
    for (int i = 0; i < 2 * max_pin + 1; i++)
    {
        pointerA.push_back(nullptr);
        pointerB.push_back(nullptr);
    }
    for (int i = 0; i < NumInstances; i++)
    {
        // cout << i << " ";
        if (Inst[i].top)
        {
            if (bucketA[max_pin - Inst[i].gain].c == nullptr)
            {
                bucketA[max_pin - Inst[i].gain].c = &Inst[i];
                pointerA[max_pin - Inst[i].gain] = &Inst[i];
            }
            else
            {

                pointerA[max_pin - Inst[i].gain]->next = &Inst[i];
                pointerA[max_pin - Inst[i].gain] = &Inst[i];
            }
        }
        else
        {
            if (bucketB[max_pin - Inst[i].gain].c == nullptr)
            {
                bucketB[max_pin - Inst[i].gain].c = &Inst[i];
                pointerB[max_pin - Inst[i].gain] = &Inst[i];
            }
            else
            {

                pointerB[max_pin - Inst[i].gain]->next = &Inst[i];
                pointerB[max_pin - Inst[i].gain] = &Inst[i];
            }
        }
    }

    return;
}
void bucket_move(int index, int temp_gain, int temp_temp_gain)
{
}
void update_gain(int index)
{
    for (int i = 0; i < Inst[index].libCell.Pin_count; i++)
    {
        int NA = 0;
        int NB = 0;
        // cout << Inst[index].top << "a";
        if (Inst[index].nets[i] < 0)
        {
            // cout << i << endl;
            continue;
        }

        // cout << Inst[index].nets[i] << endl;
        for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
        {
            if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                NA++;
            else
                NB++;
        }

        if (Inst[index].top)
        {
            if ((NA == 1) && (NB == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 2;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NA == 1) && (NB > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NA == 2) && (NB == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NA == 2) && (NB > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NA == 2) && (NB == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 2;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NA > 2) && (NB == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
        }
        else
        {

            if ((NA == 1) && (NB == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 2;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NB == 1) && (NA > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NB == 2) && (NA == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;

                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NB == 2) && (NA > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NB == 2) && (NA == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 2;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
            if ((NB > 2) && (NA == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        bucket_move(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain, Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain);
                    }
                }
            }
        }
    }
    return;
}
void partition()
{
    return;
}

int Top_NumPins, Bot_NumPins;
void Net_degree_counter()
{
    for (int i = 0; i < NumNets; i++)
    {
        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            if (Inst[Nets[i].Ins_Pin[j][0]].top)
            {
                Nets[i].Top_degree += 1;
            }
            else
            {
                Nets[i].Bot_degree += 1;
            }
        }
    }
    Top_NumPins = 0;
    Bot_NumPins = 0;
    for (int i = 0; i < NumNets; i++)
    {
        Top_NumPins += Nets[i].Top_degree;
        Bot_NumPins += Nets[i].Bot_degree;
    }
    cout << "Top_NumPins=" << Top_NumPins << endl;
    cout << "Bot_NumPins=" << Bot_NumPins << endl;
}

int main(int argc, char *argv[])
{
    fstream fin;
    fin.open("ProblemB_case1.txt", ios::in);
    // fstream fout;
    // fout.open("o.txt", ios::out);
    cout << "1" << endl;
    if (!fin)
    {
        cout << "input error";
        return 1;
    }
    else
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
            if (TA[i].size_X * TA[i].size_Y > max_size)
                max_size = TA[i].size_X * TA[i].size_Y;
            if (TA[i].Pin_count > max_pin)
            {
                max_pin = TA[i].Pin_count;
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

        for (int i = 0; i < NumInstances; i++)
        {
            getline(fin, lineStr);
            words = split(lineStr, ' ');
            Instance inst(words[1], words[2]);
            Inst.push_back(inst);
            if (inst.libCell.is_Macro)
            {
                NumMacro++;
            }
        }

        getline(fin, lineStr); // 47

        getline(fin, lineStr);
        words = split(lineStr, ' ');
        NumNets = stoi(words[1]);
        Nets = new Net[NumNets];
        for (int i = 0; i < NumNets; i++)
        {
            getline(fin, lineStr);
            words = split(lineStr, ' ');
            Nets[i].Pin_num = stoi(words[2]);
            Nets[i].Top_degree = 0;
            Nets[i].Bot_degree = 0;
            // 初始化Ins_Pin
            Nets[i].Ins_Pin = new int *[Nets[i].Pin_num];
            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                Nets[i].Ins_Pin[j] = new int[2];
            }

            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                getline(fin, lineStr);
                words = split(lineStr, ' ');
                vector<string> cut = split(words[1], '/');
                Nets[i].Ins_Pin[j][0] = stoi(cut[0].erase(0, 1)) - 1;
                Nets[i].Ins_Pin[j][1] = stoi(cut[1].erase(0, 1)) - 1;
                // cout << Nets[i].Ins_Pin[j][0] << " " << Nets[i].Ins_Pin[j][1] << endl;
                Inst[Nets[i].Ins_Pin[j][0]].input_nets(Nets[i].Ins_Pin[j][1], i);
            }
        }
    }
    split_half();

    initialize_gain();
    partition();
    print_set();
    update_gain(7);
    print_gain();
    /*for (int i = 0; i < NumInstances; i++)
    {
        cout << Inst[i].gain << " ";
    }
    cout << 2 * max_pin + 1 << endl;
    for (int i = 0; i < 2 * max_pin + 1; i++)
    {
        cout << bucketB[i].c << endl;
    }*/

    // Net_degree_counter();

    // for(int i=0; i<NumNets; i++){
    //     cout<<"Net"<<i+1<<": "<<"top="<<Nets[i].Top_degree<<", bot="<<Nets[i].Bot_degree<<endl;
    // }

    // -------------- NTUplace -------------- //
    // .aux
    fstream faux;
    faux.open("topdie.aux", ios::out);
    faux << "RowBasedPlacement : topdie.nodes topdie.nets topdie.wts topdie.pl topdie.scl";
    faux.close();
    //.nodes
    fstream fnodes;
    fnodes.open("topdie.nodes", ios::out);
    fnodes << "UCLA nodes 1.0" << endl
           << endl;
    fnodes << "NumNodes : " << IA.size() << endl;
    fnodes << "NumTerminals : " << 0 << endl;
    // Terminal還沒存進去喔
    for (int i = 0; i < IA.size(); i++)
    {
        fnodes << "\t" << Inst[IA[i]].instName << "\t" << Inst[IA[i]].libCell.size_X << "\t" << Inst[IA[i]].libCell.size_Y << endl;
    }
    //.nets
    // int Top_net_degree[NumNets] = {0};
    // int Top_num_pin = 0;
    // for(int i=0; i<NumNets; i++){

    // }

    fstream fnets;
    fnets.open("topdie.nets", ios::out);
    fnets << "UCLA nets 1.0" << endl
          << endl;
    fnets << "NumNets : " << NumNets << endl;
    fnets << "NumPins : " << Top_NumPins << endl;
    double pin_x_offset = 0;
    double pin_y_offset = 0;
    for (int i = 0; i < NumNets; i++)
    {
        fnets << "NetDegree : " << Nets[i].Top_degree << "\t"
              << "N" << i + 1 << endl;
        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            if (Inst[Nets[i].Ins_Pin[j][0]].top)
            {
                pin_x_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationX - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_X / 2;
                pin_y_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationY - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_Y / 2;
                fnets << "\t" << Inst[Nets[i].Ins_Pin[j][0]].instName << " I : " << pin_x_offset << " " << pin_y_offset << endl;
            }
        }
    }
    //.wts
    fstream fwts;
    fwts.open("topdie.wts", ios::out);
    fwts << "UCLA wts 1.0" << endl
         << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        fwts << "\t" << Inst[IA[i]].instName << "\t" << 0 << endl;
    }
    //.pl
    fstream fpl;
    fpl.open("topdie.pl", ios::out);
    fpl << "UCLA pl 1.0" << endl
        << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        if (!Inst[IA[i]].libCell.is_Macro)
        {
            fpl << Inst[IA[i]].instName << "\t" << 0 << "\t" << 0 << " : "
                << "N" << endl;
        }
        else
        {
            fpl << Inst[IA[i]].instName << "\t" << 0 << "\t" << 0 << " : "
                << "FS" << endl;
        }
    }

    //.scl
    fstream fscl;
    fscl.open("topdie.scl", ios::out);
    fscl << "UCLA scl 1.0" << endl
         << endl;
    fscl << "Numrows : " << TopDieRows_repeat_count << endl
         << endl;
    for (int i = 0; i < TopDieRows_repeat_count; i++)
    {
        fscl << "CoreRow Horizontal" << endl;
        fscl << "\t"
             << "Coordinate :\t" << i * TopDieRows_row_height << endl;
        fscl << "\t"
             << "Height :\t" << TopDieRows_row_height << endl;
        fscl << "\t"
             << "Sitewidth :\t" << 1 << endl;
        fscl << "\t"
             << "Sitespacing :\t" << 1 << endl;
        fscl << "\t"
             << "Siteorient :\t" << 1 << endl;
        fscl << "\t"
             << "Sitesymmetry :\t"
             << "Y" << endl;
        fscl << "\t"
             << "SubrowOrigin :\t" << 0 << " Numsites :\t" << DieSize_UR_Y << endl;
        fscl << "End" << endl;
    }
}