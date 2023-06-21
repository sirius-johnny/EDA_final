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

int main(){
    // READ TOP_PLACEMENT_RESULT_FILE
    string in_TOP_place_filename = "TOP_PLACE";
    fstream TOP_Result_File;
    TOP_Result_File.open(in_TOP_place_filename+".ntup.pl");
    vector<string> every_line_in_TOPFILE;
    string single_line_in_TOPFILE;
    if(TOP_Result_File){
        cout<<in_TOP_place_filename+".ntup.pl opens !"<<endl;
        getline(TOP_Result_File, single_line_in_TOPFILE);
        getline(TOP_Result_File, single_line_in_TOPFILE);
        while (getline(TOP_Result_File, single_line_in_TOPFILE))
        {
            if(single_line_in_TOPFILE=="") break;
            else every_line_in_TOPFILE.push_back(single_line_in_TOPFILE);
        }
    }
    else{
        cout<<in_TOP_place_filename+".ntup.pl doesn't exist !"<<endl;
    }
    TOP_Result_File.close();

    // READ BOT_PLACEMENT_RESULT_FILE
    string in_BOT_place_filename = "BOT_PLACE";
    fstream BOT_Result_File;
    BOT_Result_File.open(in_BOT_place_filename+".ntup.pl");
    vector<string> every_line_in_BOTFILE;
    string single_line_in_BOTFILE;
    if(BOT_Result_File){
        cout<<in_BOT_place_filename+".ntup.pl opens !"<<endl;
        getline(BOT_Result_File, single_line_in_BOTFILE);
        getline(BOT_Result_File, single_line_in_BOTFILE);
        while (getline(BOT_Result_File, single_line_in_BOTFILE))
        {
            if(single_line_in_BOTFILE=="") break;
            else every_line_in_BOTFILE.push_back(single_line_in_BOTFILE);
        }
    }
    else{
        cout<<in_BOT_place_filename+".ntup.pl doesn't exist !"<<endl;
    }
    BOT_Result_File.close();
    
    // READ TERMINAL_PLACEMENT_RESULT_FILE

    // WRITE casex_result.txt
    vector<string> many_word;
    vector<string> word;
    int rotate = 0;

    string filename = "case2";
    fstream fout;
    fout.open(filename + "_result.txt", ios::out);
    fout << "TopDiePlacement " << every_line_in_TOPFILE.size() << endl;
    for (int i = 0; i < every_line_in_TOPFILE.size(); i++)
    {
        many_word = split(every_line_in_TOPFILE[i], ' ');
        word = split(many_word[0], '\t');
        if(many_word[2]=="N") rotate = 0;
        else if(many_word[2]=="W") rotate = 90;
        else if(many_word[2]=="S") rotate = 180;
        else if(many_word[2]=="E") rotate = 270;
        fout << "Inst " << word[0] << " " << word[1] << " " << word[2] << " "
             << "R" << rotate << endl;
    }
    fout << "BottomDiePlacement " << every_line_in_BOTFILE.size() << endl;
    for (int i = 0; i < every_line_in_BOTFILE.size(); i++)
    {
        many_word = split(every_line_in_BOTFILE[i], ' ');
        word = split(many_word[0], '\t');
        if(many_word[2]=="N") rotate = 0;
        else if(many_word[2]=="W") rotate = 90;
        else if(many_word[2]=="S") rotate = 180;
        else if(many_word[2]=="E") rotate = 270;
        fout << "Inst " << word[0] << " " << word[1] << " " << word[2] << " "
             << "R" << rotate << endl;
    }
    // fout << "NumTerminals " << Terminals.size() << endl;
    // for (int i = 0; i < Terminals.size(); i++)
    // {
    //     fout << "Terminal " << Terminals[i].netName << " " << Terminals[i].center_x << " " << Terminals[i].center_y << endl;
    // }
    fout.close();
}