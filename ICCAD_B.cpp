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
int DieSize_LL_X, DieSize_LL_Y, DieSize_UR_X, DieSize_UR_Y; 
int TopDieMaxUtil, BottomDieMaxUtil;
int TopDieRows_X, TopDieRows_Y, TopDieRows_row_len, TopDieRows_row_height, TopDieRows_repeat_count;
int BottomDieRows_X, BottomDieRows_Y, BottomDieRows_row_len, BottomDieRows_row_height, BottomDieRows_repeat_count;
string TopDieTech, BottomDieTech;
int TerminalSize_X, TerminalSize_Y, TerminalSpacing, TerminalCost;
int NumInstances;
int NumNets;


int main(int argc, char* argv[]){
    fstream fin;
    fin.open("ProblemB_case1.txt", ios::in);
    fstream fout;
    fout.open("o.txt", ios::out);

    if(fin){
        string lineStr;
        while (getline(fin, lineStr)) {
            istringstream iss(lineStr);
            vector<string> words;
            string word;

            while(iss>>word){
                words.push_back(word);
            }
            
            // 一些規格的參數讀入
            if(words[0]=="NumTechnologies"){NumTechnologies=stoi(words[1]);}
            //Tech
            //LibCell
            //Pin

            if(words[0]=="DieSize"){DieSize_LL_X=stoi(words[1]);DieSize_LL_Y=stoi(words[2]);DieSize_UR_X=stoi(words[3]);DieSize_UR_Y=stoi(words[4]);}
            
            if(words[0]=="TopDieMaxUtil"){TopDieMaxUtil=stoi(words[1]);}
            if(words[0]=="BottomDieMaxUtil"){BottomDieMaxUtil=stoi(words[1]);}
            
            if(words[0]=="TopDieRows"){TopDieRows_X=stoi(words[1]);TopDieRows_Y=stoi(words[2]);TopDieRows_row_len=stoi(words[3]);TopDieRows_row_height=stoi(words[4]);TopDieRows_repeat_count=stoi(words[5]);}
            if(words[0]=="BottomDieRows"){BottomDieRows_X=stoi(words[1]);BottomDieRows_Y=stoi(words[2]);BottomDieRows_row_len=stoi(words[3]);BottomDieRows_row_height=stoi(words[4]);BottomDieRows_repeat_count=stoi(words[5]);}

            if(words[0]=="TopDieTech"){TopDieTech=words[1];}
            if(words[0]=="BottomDieTech"){BottomDieTech=words[1];}

            if(words[0]=="TerminalSize"){TerminalSize_X=stoi(words[1]);TerminalSize_Y=stoi(words[2]);}
            if(words[0]=="TerminalSpacing"){TerminalSpacing=stoi(words[1]);}
            if(words[0]=="TerminalCost"){TerminalCost=stoi(words[1]);}

            if(words[0]=="NumInstances"){NumInstances=stoi(words[1]);}
            //Inst

            if(words[0]=="NumNets"){NumNets=stoi(words[1]);}
            //Net
            //Pin

            
            // for (const auto& w : words) {
            //     cout << w << endl;
            // }
            // cout<<"---------------"<<endl;
        }
    }
}