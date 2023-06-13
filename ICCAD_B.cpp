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
            if(words[0]=="DieSize"){DieSize_LL_X=stoi(words[1]);DieSize_LL_Y=stoi(words[2]);DieSize_UR_X=stoi(words[3]);DieSize_UR_Y=stoi(words[4]);}
            if(words[0]=="TopDieMaxUtil"){TopDieMaxUtil=stoi(words[1]);}
            if(words[0]=="BottomDieMaxUtil"){BottomDieMaxUtil=stoi(words[1]);}
            
            
            for (const auto& w : words) {
                cout << w << endl;
            }

            cout<<"---------------"<<endl;
        }
    }
}