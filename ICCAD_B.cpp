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
#include <cmath>
using namespace std;

// 讀入.txt的資料們
string NumTechnologies;
long long num_LibCell;
long long DieSize_LL_X, DieSize_LL_Y, DieSize_UR_X, DieSize_UR_Y;
long long TopDieMaxUtil, BottomDieMaxUtil;
long long TopDieRows_X, TopDieRows_Y, TopDieRows_row_len, TopDieRows_row_height, TopDieRows_repeat_count;
long long BottomDieRows_X, BottomDieRows_Y, BottomDieRows_row_len, BottomDieRows_row_height, BottomDieRows_repeat_count;
string TopDieTech, BottomDieTech;
long long TerminalSize_X, TerminalSize_Y, TerminalSpacing, TerminalCost;
long long NumInstances, NumNets;
long long NumMacro = 0;
long long max_areaA, max_areaB, areaA, areaB;

// partition
vector<int> IA, IB; // vector of index of instructions (0 => C1), not sorted. IA for top / IB for bot.
long long max_size = 0;
long long max_pin = 0;
// 其他全域
int *slot_arr; // a pseudo 3d array
// Instance 面積總和
long long bot_occupied = 0;
long long top_occupied = 0;

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
typedef struct
{
    int index, gain;
} Gain;

class Instance
{
public:
    string instName, libCellName;
    LibCell libCell;
    int locationX, locationY, rotate;
    bool top;
    int *nets;
    int index;
    int gain, temp_gain;
    long long sizeA, sizeB;
    Instance *next, *previous;
    bool locked, temp_top;
    int instindex;
    long double AB_ratio;
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
        previous = nullptr;
        locked = 0;
        temp_top = 0;
        instindex = 0;
        AB_ratio = double(sizeA) / (sizeB);
    }
    void input_nets(int pin, int net)
    {
        nets[pin] = net;
    }
    void change_top(bool top) // update top, temptop, IA/IB, libCell of Instance
    {
        int instindex = this->instindex;
        this->top = top;
        this->temp_top = top;
        if (top)
        {
            libCell = TA[index];
            IB.erase(remove(IB.begin(), IB.end(), instindex), IB.end());
            IA.push_back(instindex);
        }
        else
        {
            libCell = TB[index];
            IA.erase(remove(IA.begin(), IA.end(), instindex), IA.end());
            IB.push_back(instindex);
        }
    }

private:
};
typedef struct
{
    int gain;
    Instance *c;
} Bucket;
struct Terminal
{
    string netName;
    int netNum;
    int center_x, center_y;
    int edges[4]; // left,right,bottom,left
};
struct Ratio
{
    long double AB_ratio;
    int index;
};

vector<Terminal> Terminals;
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
        IB.push_back(i);
    }
    int macroSplit = NumMacro / 2;
    int index = 0;
    while (macroSplit > 0)
    {
        // cout<<"there"<<endl;
        if (Inst[Inst.size() - 1 - index].libCell.is_Macro)
        {
            Inst[Inst.size() - 1 - index].change_top(1);
            bot_occupied -= Inst[Inst.size() - 1 - index].sizeB * TopDieMaxUtil;
            top_occupied += Inst[Inst.size() - 1 - index].sizeA * BottomDieMaxUtil;
            macroSplit--;
        }
        index++;
    }
    index = 0;
    while ((bot_occupied - top_occupied) > 0 && index < NumInstances)
    {
        // cout<<index<<endl;
        if (!Inst[index].libCell.is_Macro)
        {
            Inst[index].change_top(1);
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
            Inst[i].top = 1;
        }
    }
    return;
}
void initialize_area()
{
    areaA = 0;
    areaB = 0;
    // gain initialize
    for (int i = 0; i < NumInstances; i++)
    {
        Inst[i].gain = 0;
        if (Inst[i].top)
            areaA += Inst[i].sizeA;
        else
            areaB += Inst[i].sizeB;
    }
}
bool mycompare(Ratio r1, Ratio r2)
{
    return (r1.AB_ratio < r2.AB_ratio);
}
void ratio_split()
{
    vector<Ratio> r;
    for (int i = 0; i < NumInstances; i++)
    {
        Ratio c;
        c.index = i;
        c.AB_ratio = Inst[i].AB_ratio;
        r.push_back(c);
    }
    std::sort(r.begin(), r.end(), mycompare);
    initialize_area();
    for (int i = 0; i < NumInstances; i++)
    {
        if (areaA + Inst[r[i].index].sizeA < max_areaA)
        {
            Inst[r[i].index].change_top(1);
            areaA += Inst[r[i].index].sizeA;
        }
    }
}
void update_set()
{
    IA.clear();
    IB.clear();

    for (int i = 0; i < NumInstances; i++)
    {
        if (Inst[i].top)
        {
            IA.push_back(i);
            Inst[i].libCell = TA[Inst[i].index];
        }
        else
        {
            IB.push_back(i);
            Inst[i].libCell = TB[Inst[i].index];
        }
    }
    return;
}
void print_set()
{
    update_set();
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
}
void print_gain()
{
    for (int i = 0; i < NumInstances; i++)
    {
        cout << Inst[i].temp_gain << " ";
    }
    cout << endl;
}

void initialize_gain()
{
    // area initialize
    initialize_area();
    // gain initialize
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
    // temp_gain and temp_top and locked initialize
    for (int i = 0; i < NumInstances; i++)
    {
        Inst[i].temp_gain = Inst[i].gain;
        Inst[i].temp_top = Inst[i].top;
        Inst[i].locked = 0;
    }
    // bucket initialize
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
    // bucket list
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
                Inst[i].previous = pointerA[max_pin - Inst[i].gain];
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
                Inst[i].previous = pointerB[max_pin - Inst[i].gain];
                pointerB[max_pin - Inst[i].gain] = &Inst[i];
            }
        }
    }
    // cout << "initialized" << endl;
    return;
}
void del_cell(int index, int gain)
{
    // cout << "delete C" << index + 1 << " with gain " << gain << endl;
    if (!Inst[index].previous)
    {
        if (Inst[index].temp_top)
        {
            if (!Inst[index].next)
                bucketA[max_pin - gain].c = nullptr;
            else
            {
                bucketA[max_pin - gain].c = Inst[index].next;
                Inst[index].next->previous = nullptr;
            }
        }
        else
        {
            if (!Inst[index].next)
                bucketB[max_pin - gain].c = nullptr;
            else
            {
                bucketB[max_pin - gain].c = Inst[index].next;
                Inst[index].next->previous = nullptr;
            }
        }
    }
    else
    {
        if (!Inst[index].next)
            Inst[index].previous->next = nullptr;
        else
        {
            Inst[index].previous->next = Inst[index].next;
            Inst[index].next->previous = Inst[index].previous;
        }
    }
    // 保險起見
    Inst[index].next = nullptr;
    Inst[index].previous = nullptr;
}
void move_cell(int index, int temp_gain)
{
    // cout << Inst[index].instName << " ";
    del_cell(index, temp_gain);
    if (Inst[index].top)
    {
        if (!bucketA[max_pin - Inst[index].temp_gain].c)
        {
            bucketA[max_pin - Inst[index].temp_gain].c = &Inst[index];
            Inst[index].next = nullptr;
            Inst[index].previous = nullptr;
        }

        else
        {
            bucketA[max_pin - Inst[index].temp_gain].c->previous = &Inst[index];
            Inst[index].next = bucketA[max_pin - Inst[index].temp_gain].c;
            bucketA[max_pin - Inst[index].temp_gain].c = &Inst[index];
        }
    }
    else
    {
        if (!bucketB[max_pin - Inst[index].temp_gain].c)
        {
            bucketB[max_pin - Inst[index].temp_gain].c = &Inst[index];
            Inst[index].next = nullptr;
            Inst[index].previous = nullptr;
        }
        else
        {
            bucketB[max_pin - Inst[index].temp_gain].c->previous = &Inst[index];
            Inst[index].next = bucketB[max_pin - Inst[index].temp_gain].c;
            bucketB[max_pin - Inst[index].temp_gain].c = &Inst[index];
        }
    }
}
void update_gain(int index)
{
    del_cell(index, Inst[index].temp_gain);
    // Inst[index].nets
    for (int i = 0; i < Inst[index].libCell.Pin_count; i++)
    {
        int NA = 0;
        int NB = 0;
        // empty pin
        if (Inst[index].nets[i] < 0)
        {
            continue;
        }
        // neighbor instances
        for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
        {
            if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
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
                    // Inst[index]
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    // locked
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 2;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NA == 1) && (NB > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NA == 2) && (NB == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NA > 2) && (NB == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NA == 2) && (NB > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NA == 2) && (NB == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 2;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NA > 2) && (NB == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
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
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 2;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NB == 1) && (NA > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NB == 2) && (NA == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NB > 2) && (NA == 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain -= 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NB == 2) && (NA > 1))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NB == 2) && (NA == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 2;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
            if ((NB > 2) && (NA == 0))
            {
                for (int j = 0; j < Nets[Inst[index].nets[i]].Pin_num; j++)
                {
                    if (index == Nets[Inst[index].nets[i]].Ins_Pin[j][0])
                        continue;
                    if (Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].locked)
                        continue;
                    if (!Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_top)
                    {
                        int temp_gain = Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain;
                        Inst[Nets[Inst[index].nets[i]].Ins_Pin[j][0]].temp_gain += 1;
                        move_cell(Nets[Inst[index].nets[i]].Ins_Pin[j][0], temp_gain);
                    }
                }
            }
        }
    }
    // Inst[index]

    Inst[index].temp_top = !Inst[index].temp_top;
    Inst[index].locked = 1;
    return;
}
int max_gain()
{
    for (int i = 0; i < 2 * max_pin + 1; i++)
    {
        // cout << "i:" << i << endl;
        if ((!bucketA[i].c) && (!bucketB[i].c))
        {
            continue;
        }
        Instance *currentA = bucketA[i].c;
        Instance *currentB = bucketB[i].c;
        while ((currentA) || (currentB))
        {
            if (currentA)
            {
                // cout << "A:" << i << endl;
                if (areaB + currentA->sizeB < max_areaB)
                {
                    areaB += currentA->sizeB;
                    areaA -= currentA->sizeA;
                    return currentA->instindex;
                }

                currentA = currentA->next;
            }
            if (currentB)
            {
                // cout << "B:" << i << endl;
                if (areaA + currentB->sizeA < max_areaA)
                {
                    areaA += currentB->sizeA;
                    areaB -= currentB->sizeB;
                    return currentB->instindex;
                }
                currentB = currentB->next;
            }
        }
    }
    return -1;
}
int num_terminal()
{
    int num = 0;
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
        if ((NA != 0) && (NB != 0))
            num++;
    }
    cout << "num_terminal = " << num << endl;
    return num;
}
bool F_M()
{
    cout << "enter" << endl;
    initialize_gain();
    // cout << "initialize_gain" << endl;
    //  print_gain();
    vector<Gain> G;
    G.clear();
    // cout << "Gain" << endl;

    while (1)
    {
        int index = max_gain();
        // cout << index << " " << endl;
        if (index < 0)
            break;
        update_gain(index);
        Gain gain;
        gain.index = index;
        gain.gain = Inst[index].temp_gain;
        G.push_back(gain);
    }
    // cout << "while" << endl;

    int max = 0;
    int sum = 0;
    int G_index = -1;
    for (int i = 0; i < G.size(); i++)
    {
        sum += G[i].gain;
        if (sum > max)
        {
            max = sum;
            G_index = i;
        }
    }
    cout << "max: " << max << endl;
    if (G_index < 0)
        return 1;
    for (int i = 0; i <= G_index; i++)
    {
        Inst[G[i].index].top = (!Inst[G[i].index].top);
    }
    num_terminal();
    return 0;
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

void net_edges_init()
{
    for (int i = 0; i < NumNets; i++)
    {
        int top_left = DieSize_UR_X, top_right = DieSize_LL_X, top_top = DieSize_LL_Y, top_bot = DieSize_UR_Y;
        int bot_left = DieSize_UR_X, bot_right = DieSize_LL_X, bot_top = DieSize_LL_Y, bot_bot = DieSize_UR_Y;
        int pin_x, pin_y;
        if (Nets[i].Top_degree * Nets[i].Bot_degree > 0)
        {
            Terminal term{"N" + to_string(i + 1), i + 1, 0, 0, {0, 0, 0, 0}};
            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                string inst_name = "C" + to_string(Nets[i].Ins_Pin[j][0] + 1); // index of instance 0=>C1
                string pin_name = "P" + to_string(Nets[i].Ins_Pin[j][1] + 1);  // index of instance 0=>C1
                auto it = find_if(Inst.begin(), Inst.end(), [inst_name](Instance obj)
                                  { return obj.instName == inst_name; });
                if (it == Inst.end())
                {
                    cout << "net_edges_init error" << endl;
                }
                LibCell libcell = it->libCell;
                auto it2 = find_if(libcell.pin, libcell.pin + libcell.Pin_count, [pin_name](Pin pin)
                                   { return pin.pinName == pin_name; });
                if (it2 == libcell.pin + libcell.Pin_count)
                {
                    cout << "net_edges_init error" << endl;
                }
                pin_x = it->locationX + it2->pinLocationX;
                pin_y = it->locationY + it2->pinLocationY;
                if (it->top == 1)
                {
                    top_left = (top_left > pin_x) ? pin_x : top_left;
                    top_right = (top_right < pin_x) ? pin_x : top_right;
                    top_top = (top_top < pin_y) ? pin_y : top_top;
                    top_bot = (top_bot > pin_y) ? pin_y : top_bot;
                }
                else if (it->top == 0)
                {
                    bot_left = (bot_left > pin_x) ? pin_x : bot_left;
                    bot_right = (bot_right < pin_x) ? pin_x : bot_right;
                    bot_top = (bot_top < pin_y) ? pin_y : bot_top;
                    bot_bot = (bot_bot > pin_y) ? pin_y : bot_bot;
                }
            }
            vector<int> v = {top_left, top_right, bot_left, bot_right};
            sort(v.begin(), v.end()); // default : increasing
            term.edges[0] = v[1];
            term.edges[0] = ceil((float)max((int)(term.edges[0] - TerminalSpacing - TerminalSize_X) / 2, 0) / (float)(TerminalSize_X + TerminalSpacing));
            term.edges[1] = v[2];
            term.edges[1] = floor((float)max((int)(term.edges[1] - TerminalSpacing - TerminalSize_X) / 2, 0) / (float)(TerminalSize_X + TerminalSpacing));
            v = {top_top, top_bot, bot_top, bot_bot};
            sort(v.begin(), v.end());
            term.edges[2] = v[1];
            term.edges[2] = ceil((float)max((int)(term.edges[2] - TerminalSpacing - TerminalSize_Y) / 2, 0) / (float)(TerminalSize_Y + TerminalSpacing));
            term.edges[3] = v[2];
            term.edges[3] = floor((float)max((int)(term.edges[3] - TerminalSpacing - TerminalSize_Y) / 2, 0) / (float)(TerminalSize_Y + TerminalSpacing));
            // vector<int> v = {top_left, top_right, bot_left, bot_right};
            // sort(v.begin(), v.end()); // default : increasing
            // term.edges[0] = v[1];
            // term.edges[0] = ceil((term.edges[0] - TerminalSpacing - TerminalSize_X / 2) / (TerminalSize_X + TerminalSpacing));
            // term.edges[1] = v[2];
            // term.edges[1] = floor((term.edges[1] - TerminalSpacing - TerminalSize_X / 2) / (TerminalSize_X + TerminalSpacing));
            // v = {top_top, top_bot, bot_top, bot_bot};
            // sort(v.begin(), v.end());
            // term.edges[2] = v[1];
            // term.edges[2] = ceil((term.edges[2] - TerminalSpacing - TerminalSize_Y / 2) / (TerminalSize_Y + TerminalSpacing));
            // term.edges[3] = v[2];
            // term.edges[3] = floor((term.edges[3] - TerminalSpacing - TerminalSize_Y / 2) / (TerminalSize_Y + TerminalSpacing));

            if (term.edges[0] > term.edges[1])
            {
                swap(term.edges[0], term.edges[1]);
            }
            if (term.edges[2] > term.edges[3])
            {
                swap(term.edges[2], term.edges[3]);
            }
            Terminals.push_back(term);
        }
    }
    return;
}
void net_edges_init_top()
{
    for (int i = 0; i < NumNets; i++)
    {
        int top_left = DieSize_UR_X, top_right = DieSize_LL_X, top_top = DieSize_LL_Y, top_bot = DieSize_UR_Y;
        int bot_left = DieSize_UR_X, bot_right = DieSize_LL_X, bot_top = DieSize_LL_Y, bot_bot = DieSize_UR_Y;
        int pin_x, pin_y;
        if (Nets[i].Top_degree * Nets[i].Bot_degree > 0)
        {
            Terminal term{"N" + to_string(i + 1), i + 1, 0, 0, {0, 0, 0, 0}};
            for (int j = 0; j < Nets[i].Pin_num; j++)
            {
                string inst_name = "C" + to_string(Nets[i].Ins_Pin[j][0] + 1); // index of instance 0=>C1
                string pin_name = "P" + to_string(Nets[i].Ins_Pin[j][1] + 1);  // index of instance 0=>C1
                auto it = find_if(Inst.begin(), Inst.end(), [inst_name](Instance obj)
                                  { return obj.instName == inst_name; });
                if (it == Inst.end())
                {
                    cout << "net_edges_init error" << endl;
                }
                LibCell libcell = it->libCell;
                auto it2 = find_if(libcell.pin, libcell.pin + libcell.Pin_count, [pin_name](Pin pin)
                                   { return pin.pinName == pin_name; });
                if (it2 == libcell.pin + libcell.Pin_count)
                {
                    cout << "net_edges_init error" << endl;
                }
                pin_x = it->locationX + it2->pinLocationX;
                pin_y = it->locationY + it2->pinLocationY;
                if (it->top == 1)
                {
                    top_left = (top_left > pin_x) ? pin_x : top_left;
                    top_right = (top_right < pin_x) ? pin_x : top_right;
                    top_top = (top_top < pin_y) ? pin_y : top_top;
                    top_bot = (top_bot > pin_y) ? pin_y : top_bot;
                }
                else if (it->top == 0)
                {
                    bot_left = (bot_left > pin_x) ? pin_x : bot_left;
                    bot_right = (bot_right < pin_x) ? pin_x : bot_right;
                    bot_top = (bot_top < pin_y) ? pin_y : bot_top;
                    bot_bot = (bot_bot > pin_y) ? pin_y : bot_bot;
                }
            }
            vector<int> v = {top_left, top_right, 0, int(DieSize_UR_X)};
            sort(v.begin(), v.end()); // default : increasing
            term.edges[0] = v[1];
            term.edges[0] = ceil((float)max((int)(term.edges[0] - TerminalSpacing - TerminalSize_X) / 2, 0) / (float)(TerminalSize_X + TerminalSpacing));
            term.edges[1] = v[2];
            term.edges[1] = floor((float)max((int)(term.edges[1] - TerminalSpacing - TerminalSize_X) / 2, 0) / (float)(TerminalSize_X + TerminalSpacing));
            v = {top_top, top_bot, int(DieSize_UR_Y) , 0};
            sort(v.begin(), v.end());
            term.edges[2] = v[1];
            term.edges[2] = ceil((float)max((int)(term.edges[2] - TerminalSpacing - TerminalSize_Y) / 2, 0) / (float)(TerminalSize_Y + TerminalSpacing));
            term.edges[3] = v[2];
            term.edges[3] = floor((float)max((int)(term.edges[3] - TerminalSpacing - TerminalSize_Y) / 2, 0) / (float)(TerminalSize_Y + TerminalSpacing));
            // vector<int> v = {top_left, top_right, bot_left, bot_right};
            // sort(v.begin(), v.end()); // default : increasing
            // term.edges[0] = v[1];
            // term.edges[0] = ceil((term.edges[0] - TerminalSpacing - TerminalSize_X / 2) / (TerminalSize_X + TerminalSpacing));
            // term.edges[1] = v[2];
            // term.edges[1] = floor((term.edges[1] - TerminalSpacing - TerminalSize_X / 2) / (TerminalSize_X + TerminalSpacing));
            // v = {top_top, top_bot, bot_top, bot_bot};
            // sort(v.begin(), v.end());
            // term.edges[2] = v[1];
            // term.edges[2] = ceil((term.edges[2] - TerminalSpacing - TerminalSize_Y / 2) / (TerminalSize_Y + TerminalSpacing));
            // term.edges[3] = v[2];
            // term.edges[3] = floor((term.edges[3] - TerminalSpacing - TerminalSize_Y / 2) / (TerminalSize_Y + TerminalSpacing));

            if (term.edges[0] > term.edges[1])
            {
                swap(term.edges[0], term.edges[1]);
            }
            if (term.edges[2] > term.edges[3])
            {
                swap(term.edges[2], term.edges[3]);
            }
            Terminals.push_back(term);
        }
    }
    return;
}

void net_edges_init_V2()
{
    for (int i = 0; i < NumNets; i++)
    {
        if (Nets[i].Top_degree * Nets[i].Bot_degree > 0)
        {
            Terminal term{"N" + to_string(i + 1), i + 1, 0, 0, {0, 0, 0, 0}};
            Terminals.push_back(term);
        }
    }
    return;
}

void slot_init_V2(int &sizex, int &sizey)
{
    sizex = floor((float)(DieSize_UR_X - (2 * TerminalSpacing + TerminalSize_X)) / (float)(TerminalSpacing + TerminalSize_X)) + 1;
    sizey = floor((float)(DieSize_UR_Y - (2 * TerminalSpacing + TerminalSize_Y)) / (float)(TerminalSpacing + TerminalSize_Y)) + 1;
}

void terminal_in_order(int sizex, int sizey){
    for(int i=0; i < Terminals.size(); i++){
        Terminals[i].center_x = i%sizex;
        Terminals[i].center_y = i/sizex;
    }
}

bool compare_Netsize(Terminal t1, Terminal t2)
{ // to sort Terminals in increasing netsize order
    return ((t1.edges[1] - t1.edges[0] + 1) * (t1.edges[3] - t1.edges[2] + 1) < (t2.edges[1] - t2.edges[0] + 1) * (t2.edges[3] - t2.edges[2] + 1));
}
void slot_init(int &sizex, int &sizey)
{
    sizex = floor((float)(DieSize_UR_X - (2 * TerminalSpacing + TerminalSize_X)) / (float)(TerminalSpacing + TerminalSize_X)) + 1;
    sizey = floor((float)(DieSize_UR_Y - (2 * TerminalSpacing + TerminalSize_Y)) / (float)(TerminalSpacing + TerminalSize_Y)) + 1;
    slot_arr = new int[(sizex + 1) * (sizey + 1) * 2]();
    for (auto &terminal : Terminals)
    { // bounded in slot array
        terminal.edges[0] = max(0, terminal.edges[0]);
        terminal.edges[1] = min(sizex - 1, terminal.edges[1]);
        terminal.edges[2] = max(0, terminal.edges[2]);
        terminal.edges[3] = min(sizey - 1, terminal.edges[3]);
    }
    sort(Terminals.begin(), Terminals.end(), compare_Netsize); // sort Terminals in increasing netsize order
    // for (auto &t1 : Terminals){
    //     cout<<t1.netName<<endl;
    //     cout<<(t1.edges[1] - t1.edges[0] + 1) * (t1.edges[3] - t1.edges[2] + 1)<<endl;
    // }
    for (auto &terminal : Terminals)
    {
        for (int i = terminal.edges[0]; i <= terminal.edges[1]; i++)
        {
            for (int j = terminal.edges[2]; j <= terminal.edges[3]; j++)
            {
                // address = ia + z * (sizex * sizey) + y * (sizex) + x
                slot_arr[0 * sizex * sizey + j * sizex + i] += 1; // layer of nets on the grid
            }
        }
    }
    // for(int i=0;i<sizex*sizey;i++){
    //     cout<<slot_arr[i];
    // }
    // return;
}
bool check_replace(Terminal *terminal, int x, int y, int sizex, int sizey)
{
    int min_layer = 2 ^ 30;
    int temp_x = 0, temp_y = 0;
    bool found = false;
    for (int i = terminal->edges[0]; i <= terminal->edges[1]; i++)
    {
        for (int j = terminal->edges[2]; j <= terminal->edges[3]; j++)
        {
            if ((slot_arr[1 * sizex * sizey + j * sizex + i] == 0) && (slot_arr[0 * sizex * sizey + j * sizex + i] < min_layer))
            {
                min_layer = slot_arr[0 * sizex * sizey + j * sizex + i];
                temp_x = i;
                temp_y = j;
                found = true;
            }
        }
    }
    if (found)
    {
        slot_arr[1 * sizex * sizey + temp_y * sizex + temp_x] = terminal->netNum;
        slot_arr[1 * sizex * sizey + y * sizex + x] = 0;
        terminal->center_x = temp_x;
        terminal->center_y = temp_y;
    }
    return (found);
}
bool ripple(Terminal *terminal, int sizex, int sizey)
{
    for (int i = terminal->edges[0]; i <= terminal->edges[1]; i++)
    {
        for (int j = terminal->edges[2]; j <= terminal->edges[3]; j++)
        {
            int occupying_term = slot_arr[1 * sizex * sizey + j * sizex + i];
            auto it = find_if(Terminals.begin(), Terminals.end(), [occupying_term](Terminal term)
                              { return term.netNum == occupying_term; });
            if (check_replace(&(*it), i, j, sizex, sizey))
            {
                slot_arr[1 * sizex * sizey + j * sizex + i] = terminal->netNum;
                terminal->center_x = i;
                terminal->center_y = j;
                return (true);
            }
        }
    }
    return (false);
}
void search_slot(Terminal *terminal, int sizex, int sizey)
{
    bool found = false;
    int i = 0, min_layer = 2 ^ 30, temp_x, temp_y;
    int l = terminal->edges[0], r = terminal->edges[1], b = terminal->edges[2], t = terminal->edges[3];
    while (found == false)
    {
        if ((l - i) >= 0)
        {
            for (int j = max(0, b - i); j <= min(sizey, t + i); j++)
            {
                if ((slot_arr[1 * sizex * sizey + j * sizex + (l - i)] == 0) && (slot_arr[0 * sizex * sizey + j * sizex + (l - i)] < min_layer))
                {
                    min_layer = slot_arr[0 * sizex * sizey + j * sizex + (l - i)];
                    temp_x = (l - i);
                    temp_y = j;
                    found = true;
                }
            }
        }
        if ((r + i) <= sizex - 1)
        {
            for (int j = max(0, b - i); j <= min(sizey, t + i); j++)
            {
                if ((slot_arr[1 * sizex * sizey + j * sizex + (r + i)] == 0) && (slot_arr[0 * sizex * sizey + j * sizex + (r + i)] < min_layer))
                {
                    min_layer = slot_arr[0 * sizex * sizey + j * sizex + (r + i)];
                    temp_x = (r + i);
                    temp_y = j;
                    found = true;
                }
            }
        }
        if ((b - i) >= 0)
        {
            for (int j = max(0, l - i); j <= min(sizex, r + i); j++)
            {
                if ((slot_arr[1 * sizex * sizey + (b - i) * sizex + j] == 0) && (slot_arr[0 * sizex * sizey + (b - i) * sizex + j] < min_layer))
                {
                    min_layer = slot_arr[0 * sizex * sizey + (b - i) * sizex + j];
                    temp_x = j;
                    temp_y = b - i;
                    found = true;
                }
            }
        }
        if ((t + i) <= sizey - 1)
        {
            for (int j = max(0, l - i); j <= min(sizex, r + i); j++)
            {
                if ((slot_arr[1 * sizex * sizey + (t + i) * sizex + j] == 0) && (slot_arr[0 * sizex * sizey + (t + i) * sizex + j] < min_layer))
                {
                    min_layer = slot_arr[0 * sizex * sizey + (t + i) * sizex + j];
                    temp_x = j;
                    temp_y = t + i;
                    found = true;
                }
            }
        }
        i = i + 1;
    }
    slot_arr[1 * sizex * sizey + temp_y * sizex + temp_x] = terminal->netNum;
    terminal->center_x = temp_x;
    terminal->center_y = temp_y;
}
void place_terminal(const int sizex, const int sizey)
{ // placing terminals by searching 2 levels
    int temp_x = 0, temp_y = 0, min_layer = 2 ^ 30;
    bool found = false;
    for (auto &terminal : Terminals)
    {
        temp_x = 0;
        temp_y = 0;
        min_layer = 2 ^ 30;
        found = false;
        for (int i = terminal.edges[0]; i <= terminal.edges[1]; i++)
        {

            for (int j = terminal.edges[2]; j <= terminal.edges[3]; j++)
            {

                if ((slot_arr[1 * sizex * sizey + j * sizex + i] == 0) && (slot_arr[0 * sizex * sizey + j * sizex + i] < min_layer))
                {
                    temp_x = i;
                    temp_y = j;
                    min_layer = slot_arr[0 * sizex * sizey + j * sizex + i];
                    found = true;
                }
            }
        }
        if (found)
        {
            slot_arr[1 * sizex * sizey + temp_y * sizex + temp_x] = terminal.netNum;
            terminal.center_x = temp_x;
            terminal.center_y = temp_y;
        }
        else
        {
            if (!ripple(&terminal, sizex, sizey))
            {
                search_slot(&terminal, sizex, sizey);
            }
        }
        for (int i = terminal.edges[0]; i <= terminal.edges[1]; i++)
        {
            for (int j = terminal.edges[2]; j <= terminal.edges[3]; j++)
            {
                slot_arr[0 * sizex * sizey + j * sizex + i] -= 1;
            }
        }
    }
    return;
}

//////////////////////////////// NTUplace Associated Functions ////////////////////////////////
void NTUplace_TOP(string filename);
void NTUplace_BOT(string filename);
void NTUplace_Get_Placement_Result(string filename, bool top_or_not);
void NTUplace_BOT_PinProjection(string filename);
void NTUplace_BOT_PinProjection_SINGLE(string filename);
void NTUplace_BOT_TerminalProjection(string filename);
//////////////////////////////// Write the Output File ////////////////////////////////
void Output_Format(string filename);
void TOP_ter_BOT_Output(string TOP_filename, string BOT_filename, string filename);

int main(int argc, char *argv[])
{
    fstream fin;
    fin.open("ProblemB_case4.txt", ios::in);
    // fstream fout;
    // fout.open("o.txt", ios::out);
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

        max_areaA = DieSize_UR_X * DieSize_UR_Y * TopDieMaxUtil / 100;
        max_areaB = DieSize_UR_X * DieSize_UR_Y * BottomDieMaxUtil / 100;

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
            inst.instindex = i;
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
    // 讀檔結束決定ICCAD_B.cpp要進行(請修改mode為partition或terminal)：
    // [partition] 1. 進行partition，並生出NTUplace檔；
    // [terminal]  2. 讀進.ntup.pl檔們，並擺放terminals，生出output_result.txt
    string mode;
    cout << "MODE[partition]: Do partition, and generate NTUplace files." << endl;
    cout << "MODE[pinprojection]: Do pin projecting, and generate BOTTOM NTUplace files." << endl;
    cout << "MODE[terminal_afterplaceTOPBOT]: Do terminal placing after placing TOP&BOT, and generate output.txt." << endl;
    cout << "Enter MODE for ICCAD_B.cpp: ";
    cin >> mode;
    // mode = "partition";
    // NTUplace 檔案相關
    string Top_NTUplace_filename, Bot_NTUplace_filename;
    Top_NTUplace_filename = "TOP_PLACE_case4";
    Bot_NTUplace_filename = "BOT_PLACE_case4";

    if (mode == "partition")
    {
        if (NumTechnologies == "1")
        {
            partition_init();
        }
        else
        {
            ratio_split();
        }

        initialize_area();
        cout << "max_areaA=    " << max_areaA << ", maxareaB=     " << max_areaB << endl;
        cout << "current_areaA=" << areaA << ", current_areaB=" << areaB << endl;
        // print_set();

        num_terminal();
        F_M();
        while (0)
        {
            bool end = F_M();
            if (end)
            {
                update_set(); // 一定要
                break;
            }
        }
        update_set();
        num_terminal();
        initialize_area();
        cout << "max_areaA=    " << max_areaA << ", maxareaB=     " << max_areaB << endl;
        cout << "current_areaA=" << areaA << ", current_areaB=" << areaB << endl;
        Net_degree_counter(); // 一定要記得先call這個function才能用NTUplace
        NTUplace_TOP(Top_NTUplace_filename);
        NTUplace_BOT(Bot_NTUplace_filename);
    }

    else if (mode == "pinprojection")
    {
        print_set();
        NTUplace_Get_Placement_Result(Top_NTUplace_filename, true);
        update_set();
        Net_degree_counter();
        NTUplace_BOT_PinProjection_SINGLE(Bot_NTUplace_filename);
    }

    else if (mode == "terminal_afterplaceTOPBOT")
    {
        NTUplace_Get_Placement_Result(Top_NTUplace_filename, true);
        NTUplace_Get_Placement_Result(Bot_NTUplace_filename, false);
        update_set();
        // NTUplace_Get_Placement_Result(Bot_NTUplace_filename + "_PROJECTION");
        Net_degree_counter(); // 一定要記得先call這個function才能用NTUplace
        net_edges_init_V2();
        int sizex = 0, sizey = 0;
        slot_init_V2(sizex, sizey);
        terminal_in_order(sizex, sizey);
        // place_terminal(sizex, sizey);
        for (auto &terminal : Terminals)
        {
            terminal.center_x = terminal.center_x * (TerminalSize_X + TerminalSpacing) + TerminalSpacing + TerminalSize_X / 2;
            terminal.center_y = terminal.center_y * (TerminalSize_Y + TerminalSpacing) + TerminalSpacing + TerminalSize_Y / 2;
        }
        /// terminal end ///
        Output_Format("case4");
    }

    else if(mode == "terminal_afterplaceTOP"){
        
        NTUplace_Get_Placement_Result(Top_NTUplace_filename, true);
        update_set();
        Net_degree_counter();
        net_edges_init_top();
        int sizex = 0, sizey = 0;
        slot_init(sizex, sizey);
        place_terminal(sizex, sizey);
        for (auto &terminal : Terminals)
        {
            terminal.center_x = terminal.center_x * (TerminalSize_X + TerminalSpacing) + TerminalSpacing + TerminalSize_X / 2;
            terminal.center_y = terminal.center_y * (TerminalSize_Y + TerminalSpacing) + TerminalSpacing + TerminalSize_Y / 2;
        }
        //TODO:印出terminal寫入BOT的NTUplace檔案們
        NTUplace_BOT_TerminalProjection(Bot_NTUplace_filename);
    }

    else if(mode == "terminal_afterplaceTOP_RESULT"){
        string out_filename = "case2_ter_v2";
        TOP_ter_BOT_Output(Top_NTUplace_filename, Bot_NTUplace_filename, out_filename);
    }

    else if(mode == "terminal_lookup2times"){
        //先參考TOP後擺出termminals，再用結果擺出BOT，最後根據TOP和BOT再擺一次terminals
        NTUplace_Get_Placement_Result(Top_NTUplace_filename, true);
        NTUplace_Get_Placement_Result(Bot_NTUplace_filename, false);
        update_set();
        Net_degree_counter(); // 一定要記得先call這個function才能用NTUplace
        net_edges_init();
        int sizex = 0, sizey = 0;
        slot_init(sizex, sizey);
        place_terminal(sizex, sizey);
        for (auto &terminal : Terminals)
        {
            terminal.center_x = terminal.center_x * (TerminalSize_X + TerminalSpacing) + TerminalSpacing + TerminalSize_X / 2;
            terminal.center_y = terminal.center_y * (TerminalSize_Y + TerminalSpacing) + TerminalSpacing + TerminalSize_Y / 2;
        }
        Output_Format("case2_terminal_lookup2times_V2");
    }
}

//////////////////////////////// NTUplace Associated Functions ////////////////////////////////
void NTUplace_TOP(string filename)
{
    // .aux
    fstream faux;
    faux.open(filename + ".aux", ios::out);
    faux << "RowBasedPlacement : " << filename << ".nodes " << filename << ".nets " << filename << ".wts " << filename << ".pl " << filename << ".scl";
    faux.close();
    //.nodes
    fstream fnodes;
    fnodes.open(filename + ".nodes", ios::out);
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
    fnets.open(filename + ".nets", ios::out);
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
    fwts.open(filename + ".wts", ios::out);
    fwts << "UCLA wts 1.0" << endl
         << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        fwts << "\t" << Inst[IA[i]].instName << "\t" << 0 << endl;
    }
    //.pl
    fstream fpl;
    fpl.open(filename + ".pl", ios::out);
    fpl << "UCLA pl 1.0" << endl
        << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        if (!Inst[IA[i]].libCell.is_Macro)
        {
            fpl << Inst[IA[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "N" << endl;
        }
        else
        {
            fpl << Inst[IA[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "E" << endl;
        }
    }

    //.scl
    fstream fscl;
    fscl.open(filename + ".scl", ios::out);
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
             << "SubrowOrigin :\t" << 0 << " Numsites :\t" << DieSize_UR_X << endl;
        fscl << "End" << endl;
    }
}

void NTUplace_BOT(string filename)
{
    // .aux
    fstream faux;
    faux.open(filename + ".aux", ios::out);
    faux << "RowBasedPlacement : " << filename << ".nodes " << filename << ".nets " << filename << ".wts " << filename << ".pl " << filename << ".scl";
    faux.close();
    //.nodes
    fstream fnodes;
    fnodes.open(filename + ".nodes", ios::out);
    fnodes << "UCLA nodes 1.0" << endl
           << endl;
    fnodes << "NumNodes : " << IB.size() << endl;
    fnodes << "NumTerminals : " << 0 << endl;
    // Terminal還沒存進去喔
    for (int i = 0; i < IB.size(); i++)
    {
        fnodes << "\t" << Inst[IB[i]].instName << "\t" << Inst[IB[i]].libCell.size_X << "\t" << Inst[IB[i]].libCell.size_Y << endl;
    }

    fstream fnets;
    fnets.open(filename + ".nets", ios::out);
    fnets << "UCLA nets 1.0" << endl
          << endl;
    fnets << "NumNets : " << NumNets << endl;
    fnets << "NumPins : " << Bot_NumPins << endl;
    double pin_x_offset = 0;
    double pin_y_offset = 0;
    for (int i = 0; i < NumNets; i++)
    {
        fnets << "NetDegree : " << Nets[i].Bot_degree << "\t"
              << "N" << i + 1 << endl;
        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            if (!Inst[Nets[i].Ins_Pin[j][0]].top)
            {
                pin_x_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationX - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_X / 2;
                pin_y_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationY - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_Y / 2;
                fnets << "\t" << Inst[Nets[i].Ins_Pin[j][0]].instName << " I : " << pin_x_offset << " " << pin_y_offset << endl;
            }
        }
    }
    //.wts
    fstream fwts;
    fwts.open(filename + ".wts", ios::out);
    fwts << "UCLA wts 1.0" << endl
         << endl;
    for (int i = 0; i < IB.size(); i++)
    {
        fwts << "\t" << Inst[IB[i]].instName << "\t" << 0 << endl;
    }
    //.pl
    fstream fpl;
    fpl.open(filename + ".pl", ios::out);
    fpl << "UCLA pl 1.0" << endl
        << endl;
    for (int i = 0; i < IB.size(); i++)
    {
        if (!Inst[IB[i]].libCell.is_Macro)
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "N" << endl;
        }
        else
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "E" << endl;
        }
    }

    //.scl
    fstream fscl;
    fscl.open(filename + ".scl", ios::out);
    fscl << "UCLA scl 1.0" << endl
         << endl;
    fscl << "Numrows : " << BottomDieRows_repeat_count << endl
         << endl;
    for (int i = 0; i < BottomDieRows_repeat_count; i++)
    {
        fscl << "CoreRow Horizontal" << endl;
        fscl << "\t"
             << "Coordinate :\t" << i * BottomDieRows_row_height << endl;
        fscl << "\t"
             << "Height :\t" << BottomDieRows_row_height << endl;
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
             << "SubrowOrigin :\t" << 0 << " Numsites :\t" << DieSize_UR_X << endl;
        fscl << "End" << endl;
    }
}

void NTUplace_Get_Placement_Result(string filename, bool top_or_not)
{
    string a_line;            // File read var.
    vector<string> many_word; // File read var.
    vector<string> word;
    fstream Result_File;
    Result_File.open(filename + ".ntup.pl");
    if (Result_File)
    {
        cout << "File open!" << endl;
        getline(Result_File, a_line);
        getline(Result_File, a_line);
        while (getline(Result_File, a_line))
        {
            many_word = split(a_line, ' ');
            if (many_word.size() == 0)
            {
                break;
            } // Read all instances
            if (many_word[many_word.size() - 1] != "/FIXED")
            {
                word = split(many_word[0], '\t');
                word[0] = word[0].erase(0, 1);
                Inst[stoi(word[0]) - 1].top = top_or_not;
                Inst[stoi(word[0]) - 1].locationX = stoi(word[1]); // Update LL_X in instances
                Inst[stoi(word[0]) - 1].locationY = stoi(word[2]); // Update LL_Y in instances
                // Rotate angle
                if (many_word[2] == "N" || many_word[2] == "FN")
                {
                    Inst[stoi(word[0]) - 1].rotate = 0;
                }
                else if (many_word[2] == "W" || many_word[2] == "FW")
                {
                    Inst[stoi(word[0]) - 1].rotate = 90;
                }
                else if (many_word[2] == "S" || many_word[2] == "FS")
                {
                    Inst[stoi(word[0]) - 1].rotate = 180;
                }
                else if (many_word[2] == "E" || many_word[2] == "FE")
                {
                    Inst[stoi(word[0]) - 1].rotate = 270;
                }
            }
            else
            {
                continue;
            }
        }
    }
    else
    {
        cout << filename + ".ntup.pl doesn't exist..." << endl;
    }
}

void NTUplace_BOT_PinProjection(string filename)
{
    // .aux
    fstream faux;
    faux.open(filename + ".aux", ios::out);
    faux << "RowBasedPlacement : " << filename << ".nodes " << filename << ".nets " << filename << ".wts " << filename << ".pl " << filename << ".scl";
    faux.close();
    //.nodes
    fstream fnodes;
    fnodes.open(filename + ".nodes", ios::out);
    fnodes << "UCLA nodes 1.0" << endl
           << endl;
    fnodes << "NumNodes : " << IA.size() + IB.size() << endl;
    fnodes << "NumTerminals : " << IA.size() << endl; // 上層的Inst全部投影下來當參考
                                                      // Terminal還沒存進去喔
    for (int i = 0; i < IA.size(); i++)
    {
        fnodes << "\t" << Inst[IA[i]].instName << "\t" << 0 << "\t" << 0 << "\tterminal" << endl;
    }
    for (int i = 0; i < IB.size(); i++)
    {
        fnodes << "\t" << Inst[IB[i]].instName << "\t" << Inst[IB[i]].libCell.size_X << "\t" << Inst[IB[i]].libCell.size_Y << endl;
    }
    //.nets
    // int Top_net_degree[NumNets] = {0};
    // int Top_num_pin = 0;
    // for(int i=0; i<NumNets; i++){

    // }

    fstream fnets;
    fnets.open(filename + ".nets", ios::out);
    fnets << "UCLA nets 1.0" << endl
          << endl;
    fnets << "NumNets : " << NumNets << endl;
    fnets << "NumPins : " << Top_NumPins + Bot_NumPins << endl;
    double pin_x_offset = 0;
    double pin_y_offset = 0;
    for (int i = 0; i < NumNets; i++)
    {
        fnets << "NetDegree : " << Nets[i].Top_degree + Nets[i].Bot_degree << "\t"
              << "N" << i + 1 << endl;
        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            pin_x_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationX - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_X / 2;
            pin_y_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationY - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_Y / 2;
            fnets << "\t" << Inst[Nets[i].Ins_Pin[j][0]].instName << " I : " << pin_x_offset << " " << pin_y_offset << endl;
        }
    }
    //.wts
    fstream fwts;
    fwts.open(filename + ".wts", ios::out);
    fwts << "UCLA wts 1.0" << endl
         << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        fwts << "\t" << Inst[IA[i]].instName << "\t" << 0 << endl;
    }
    for (int i = 0; i < IB.size(); i++)
    {
        fwts << "\t" << Inst[IB[i]].instName << "\t" << 0 << endl;
    }
    //.pl
    fstream fpl;
    fpl.open(filename + ".pl", ios::out);
    fpl << "UCLA pl 1.0" << endl
        << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        fpl << Inst[IA[i]].instName << "\t" << Inst[IA[i]].locationX + Inst[IA[i]].libCell.size_X / 2 << "\t" << Inst[IA[i]].locationY + Inst[IA[i]].libCell.size_Y / 2 << " : "
            << "/FIXED" << endl;
    }
    for (int i = 0; i < IB.size(); i++)
    {
        if (!Inst[IB[i]].libCell.is_Macro)
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "N" << endl;
        }
        else
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "E" << endl;
        }
    }

    //.scl
    fstream fscl;
    fscl.open(filename + ".scl", ios::out);
    fscl << "UCLA scl 1.0" << endl
         << endl;
    fscl << "Numrows : " << BottomDieRows_repeat_count << endl
         << endl;
    for (int i = 0; i < BottomDieRows_repeat_count; i++)
    {
        fscl << "CoreRow Horizontal" << endl;
        fscl << "\t"
             << "Coordinate :\t" << i * BottomDieRows_row_height << endl;
        fscl << "\t"
             << "Height :\t" << BottomDieRows_row_height << endl;
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
             << "SubrowOrigin :\t" << 0 << " Numsites :\t" << DieSize_UR_X << endl;
        fscl << "End" << endl;
    }
}

void NTUplace_BOT_PinProjection_SINGLE(string filename)
{
    // .aux
    fstream faux;
    faux.open(filename + ".aux", ios::out);
    faux << "RowBasedPlacement : " << filename << ".nodes " << filename << ".nets " << filename << ".wts " << filename << ".pl " << filename << ".scl";
    faux.close();
    //.nodes
    fstream fnodes;
    fnodes.open(filename + ".nodes", ios::out);
    fnodes << "UCLA nodes 1.0" << endl
           << endl;
    fnodes << "NumNodes : " << 1 + IB.size() << endl;
    fnodes << "NumTerminals : " << 1 << endl; // 上層的Inst全部投影下來當參考

    fnodes << "\t"
           << "CC"
           << "\t" << 0 << "\t" << 0 << "\tterminal" << endl;
    for (int i = 0; i < IB.size(); i++)
    {
        fnodes << "\t" << Inst[IB[i]].instName << "\t" << Inst[IB[i]].libCell.size_X << "\t" << Inst[IB[i]].libCell.size_Y << endl;
    }
    //.nets
    fstream fnets;
    fnets.open(filename + ".nets", ios::out);
    fnets << "UCLA nets 1.0" << endl
          << endl;
    fnets << "NumNets : " << NumNets << endl;

    int total_pin = 0;
    for (int i = 0; i < NumNets; i++)
    {
        if (Nets[i].Top_degree == 0)
            total_pin += Nets[i].Bot_degree;
        else
            total_pin += 1 + Nets[i].Bot_degree;
    }
    fnets << "NumPins : " << total_pin << endl;

    double pin_x_offset = 0;
    double pin_y_offset = 0;
    double x_sum = 0;
    double y_sum = 0;
    for (int i = 0; i < NumNets; i++)
    {
        if (Nets[i].Top_degree == 0)
        {
            fnets << "NetDegree : " << Nets[i].Bot_degree << "\t"
                  << "N" << i + 1 << endl;
        }
        else
            fnets << "NetDegree : " << 1 + Nets[i].Bot_degree << "\t"
                  << "N" << i + 1 << endl;

        if (Nets[i].Top_degree != 0)
        {
            x_sum = 0;
            y_sum = 0;
            for (int k = 0; k < Nets[i].Top_degree; k++)
            {
                x_sum += Inst[Nets[i].Ins_Pin[k][0]].libCell.pin[Nets[i].Ins_Pin[k][1]].pinLocationX + Inst[Nets[i].Ins_Pin[k][0]].locationX;
                y_sum += Inst[Nets[i].Ins_Pin[k][0]].libCell.pin[Nets[i].Ins_Pin[k][1]].pinLocationY + Inst[Nets[i].Ins_Pin[k][0]].locationY;
            }
            x_sum = x_sum / Nets[i].Top_degree;
            y_sum = y_sum / Nets[i].Top_degree;
            x_sum -= DieSize_UR_X / 2;
            y_sum -= DieSize_UR_Y / 2;
            fnets << "\t"
                  << "CC"
                  << " I : " << x_sum << " " << y_sum << endl;
        }

        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            if (!Inst[Nets[i].Ins_Pin[j][0]].top)
            {
                pin_x_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationX - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_X / 2;
                pin_y_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationY - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_Y / 2;
                fnets << "\t" << Inst[Nets[i].Ins_Pin[j][0]].instName << " I : " << pin_x_offset << " " << pin_y_offset << endl;
            }
        }
    }
    //.wts
    fstream fwts;
    fwts.open(filename + ".wts", ios::out);
    fwts << "UCLA wts 1.0" << endl
         << endl;
    fwts << "\t"
         << "CC"
         << "\t" << 0 << endl;
    for (int i = 0; i < IB.size(); i++)
    {
        fwts << "\t" << Inst[IB[i]].instName << "\t" << 0 << endl;
    }
    //.pl
    fstream fpl;
    fpl.open(filename + ".pl", ios::out);
    fpl << "UCLA pl 1.0" << endl
        << endl;
    fpl << "CC"
        << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
        << "/FIXED" << endl;
    for (int i = 0; i < IB.size(); i++)
    {
        if (!Inst[IB[i]].libCell.is_Macro)
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "N" << endl;
        }
        else
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "E" << endl;
        }
    }

    //.scl
    fstream fscl;
    fscl.open(filename + ".scl", ios::out);
    fscl << "UCLA scl 1.0" << endl
         << endl;
    fscl << "Numrows : " << BottomDieRows_repeat_count << endl
         << endl;
    for (int i = 0; i < BottomDieRows_repeat_count; i++)
    {
        fscl << "CoreRow Horizontal" << endl;
        fscl << "\t"
             << "Coordinate :\t" << i * BottomDieRows_row_height << endl;
        fscl << "\t"
             << "Height :\t" << BottomDieRows_row_height << endl;
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
             << "SubrowOrigin :\t" << 0 << " Numsites :\t" << DieSize_UR_X << endl;
        fscl << "End" << endl;
    }
}

void NTUplace_BOT_TerminalProjection(string filename)
{
    // .aux
    fstream faux;
    faux.open(filename + ".aux", ios::out);
    faux << "RowBasedPlacement : " << filename << ".nodes " << filename << ".nets " << filename << ".wts " << filename << ".pl " << filename << ".scl";
    faux.close();
    //.nodes
    fstream fnodes;
    fnodes.open(filename + ".nodes", ios::out);
    fnodes << "UCLA nodes 1.0" << endl
           << endl;
    fnodes << "NumNodes : " << IB.size()+Terminals.size() << endl;
    fnodes << "NumTerminals : " << Terminals.size() << endl;
    // Terminal還沒存進去喔
    for(int i=0; i<Terminals.size(); i++){
        fnodes <<"\t"<< "T"+Terminals[i].netName<<"\t"<<0<<"\t"<<0<<"\t"<<"terminal"<<endl;
    }
    for (int i = 0; i < IB.size(); i++)
    {
        fnodes << "\t" << Inst[IB[i]].instName << "\t" << Inst[IB[i]].libCell.size_X << "\t" << Inst[IB[i]].libCell.size_Y << endl;
    }

    fstream fnets;
    fnets.open(filename + ".nets", ios::out);
    fnets << "UCLA nets 1.0" << endl
          << endl;
    fnets << "NumNets : " << NumNets << endl;
    fnets << "NumPins : " << Bot_NumPins+Terminals.size() << endl;
    double pin_x_offset = 0;
    double pin_y_offset = 0;
    for (int i = 0; i < NumNets; i++)
    {
        bool Ter_place = false;
        for(int k=0; k<Terminals.size(); k++){
            if(Terminals[k].netNum==i+1){
                fnets << "NetDegree : " << Nets[i].Bot_degree+1 << "\t" << "N" << i + 1 << endl;
                fnets << "\t" << "T"+Terminals[k].netName << " I "<<endl;
                Ter_place = true;
                break;
            }
        }

        if(!Ter_place){
            fnets << "NetDegree : " << Nets[i].Bot_degree << "\t" << "N" << i + 1 << endl;
        }
        
        for (int j = 0; j < Nets[i].Pin_num; j++)
        {
            if (!Inst[Nets[i].Ins_Pin[j][0]].top)
            {
                pin_x_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationX - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_X / 2;
                pin_y_offset = Inst[Nets[i].Ins_Pin[j][0]].libCell.pin[Nets[i].Ins_Pin[j][1]].pinLocationY - Inst[Nets[i].Ins_Pin[j][0]].libCell.size_Y / 2;
                fnets << "\t" << Inst[Nets[i].Ins_Pin[j][0]].instName << " I : " << pin_x_offset << " " << pin_y_offset << endl;
            }
        }
    }
    //.wts
    fstream fwts;
    fwts.open(filename + ".wts", ios::out);
    fwts << "UCLA wts 1.0" << endl
         << endl;
    for(int i=0; i<Terminals.size(); i++){
        fwts<<"\t"<<"T"+Terminals[i].netName<<"\t"<<0<<endl;
    }
    for (int i = 0; i < IB.size(); i++)
    {
        fwts << "\t" << Inst[IB[i]].instName << "\t" << 0 << endl;
    }
    //.pl
    fstream fpl;
    fpl.open(filename + ".pl", ios::out);
    fpl << "UCLA pl 1.0" << endl
        << endl;

    for(int i=0; i<Terminals.size(); i++){
        fpl<<"T"+Terminals[i].netName<<"\t"<<Terminals[i].center_x<<"\t"<<Terminals[i].center_y<<" : "<<"/FIXED"<<endl;
    }
    for (int i = 0; i < IB.size(); i++)
    {
        if (!Inst[IB[i]].libCell.is_Macro)
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "N" << endl;
        }
        else
        {
            fpl << Inst[IB[i]].instName << "\t" << DieSize_UR_X / 2 << "\t" << DieSize_UR_Y / 2 << " : "
                << "E" << endl;
        }
    }

    //.scl
    fstream fscl;
    fscl.open(filename + ".scl", ios::out);
    fscl << "UCLA scl 1.0" << endl
         << endl;
    fscl << "Numrows : " << BottomDieRows_repeat_count << endl
         << endl;
    for (int i = 0; i < BottomDieRows_repeat_count; i++)
    {
        fscl << "CoreRow Horizontal" << endl;
        fscl << "\t"
             << "Coordinate :\t" << i * BottomDieRows_row_height << endl;
        fscl << "\t"
             << "Height :\t" << BottomDieRows_row_height << endl;
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
             << "SubrowOrigin :\t" << 0 << " Numsites :\t" << DieSize_UR_X << endl;
        fscl << "End" << endl;
    }
}

void Output_Format(string filename)
{
    fstream fout;
    fout.open(filename + "_result.txt", ios::out);
    fout << "TopDiePlacement " << IA.size() << endl;
    for (int i = 0; i < IA.size(); i++)
    {
        fout << "Inst " << Inst[IA[i]].instName << " " << Inst[IA[i]].locationX << " " << Inst[IA[i]].locationY << " "
             << "R" << Inst[IA[i]].rotate << endl;
    }
    fout << "BottomDiePlacement " << IB.size() << endl;
    for (int i = 0; i < IB.size(); i++)
    {
        fout << "Inst " << Inst[IB[i]].instName << " " << Inst[IB[i]].locationX << " " << Inst[IB[i]].locationY << " "
             << "R" << Inst[IB[i]].rotate << endl;
    }
    fout << "NumTerminals " << Terminals.size() << endl;
    for (int i = 0; i < Terminals.size(); i++)
    {
        fout << "Terminal " << Terminals[i].netName << " " << Terminals[i].center_x << " " << Terminals[i].center_y << endl;
    }
    fout.close();
}

void TOP_ter_BOT_Output(string TOP_filename, string BOT_filename, string filename){
    // READ TOP_PLACEMENT_RESULT_FILE
    fstream TOP_Result_File;
    TOP_Result_File.open(TOP_filename+".ntup.pl");
    vector<string> every_line_in_TOPFILE;
    string single_line_in_TOPFILE;
    if(TOP_Result_File){
        cout<<TOP_filename+".ntup.pl opens !"<<endl;
        getline(TOP_Result_File, single_line_in_TOPFILE);
        getline(TOP_Result_File, single_line_in_TOPFILE);
        while (getline(TOP_Result_File, single_line_in_TOPFILE))
        {
            if(single_line_in_TOPFILE=="") break;
            else every_line_in_TOPFILE.push_back(single_line_in_TOPFILE);
        }
    }
    else{
        cout<<TOP_filename+".ntup.pl doesn't exist !"<<endl;
    }
    TOP_Result_File.close();

    // READ BOT_PLACEMENT_RESULT_FILE
    fstream BOT_Result_File;
    BOT_Result_File.open(BOT_filename+".ntup.pl");
    vector<string> every_line_in_BOTFILE;
    vector<string> terminal_in_BOTFILE;
    string single_line_in_BOTFILE;
    if(BOT_Result_File){
        cout<<BOT_filename+".ntup.pl opens !"<<endl;
        getline(BOT_Result_File, single_line_in_BOTFILE);
        getline(BOT_Result_File, single_line_in_BOTFILE);
        while (getline(BOT_Result_File, single_line_in_BOTFILE))
        {
            if(single_line_in_BOTFILE=="") break;
            else {
                vector<string> check;
                check = split(single_line_in_BOTFILE, ' ');
                if(check[check.size()-1] != "/FIXED"){
                    every_line_in_BOTFILE.push_back(single_line_in_BOTFILE);
                }
                else{
                    terminal_in_BOTFILE.push_back(single_line_in_BOTFILE);
                }
            }
        }
    }
    else{
        cout<<BOT_filename+".ntup.pl doesn't exist !"<<endl;
    }
    BOT_Result_File.close();
    
    // READ TERMINAL_PLACEMENT_RESULT_FILE

    // WRITE casex_result.txt
    vector<string> many_word;
    vector<string> word;
    int rotate = 0;

    fstream fout;
    fout.open(filename + "_result.txt", ios::out);
    fout << "TopDiePlacement " << every_line_in_TOPFILE.size() << endl;
    for (int i = 0; i < every_line_in_TOPFILE.size(); i++)
    {
        many_word = split(every_line_in_TOPFILE[i], ' ');
        word = split(many_word[0], '\t');
        if(many_word[2]=="N" || many_word[2]=="FN") rotate = 0;
        else if(many_word[2]=="W" || many_word[2]=="FW") rotate = 90;
        else if(many_word[2]=="S" || many_word[2]=="FS") rotate = 180;
        else if(many_word[2]=="E" || many_word[2]=="FE") rotate = 270;
        fout << "Inst " << word[0] << " " << word[1] << " " << word[2] << " "
             << "R" << rotate << endl;
    }
    fout << "BottomDiePlacement " << every_line_in_BOTFILE.size() << endl;
    for (int i = 0; i < every_line_in_BOTFILE.size(); i++)
    {
        many_word = split(every_line_in_BOTFILE[i], ' ');
        word = split(many_word[0], '\t');
        if(many_word[2]=="N" || many_word[2]=="FN") rotate = 0;
        else if(many_word[2]=="W" || many_word[2]=="FW") rotate = 90;
        else if(many_word[2]=="S" || many_word[2]=="FS") rotate = 180;
        else if(many_word[2]=="E" || many_word[2]=="FE") rotate = 270;
        fout << "Inst " << word[0] << " " << word[1] << " " << word[2] << " "
             << "R" << rotate << endl;
    }
    fout<< "NumTerminals "<<terminal_in_BOTFILE.size()<<endl;
    for(int i=0; i<terminal_in_BOTFILE.size(); i++){
        many_word = split(terminal_in_BOTFILE[i], ' ');
        word = split(many_word[0], '\t');
        fout<<"Terminal "<<word[0].erase(0,1)<<" "<<stoi(word[1])<<" "<<stoi(word[2])<<endl;
    }
}