#ifndef BINPACKING_H
#define BINPACKING_H

#include <vector>
#include <functional>
#include <iostream>
using namespace std;

class CPlaceDB;


class CBinPacking
{
public:
	static bool Packing(CPlaceDB& fplan, 
		const double &left, 
		const double &bottom, 
		const double &right, 
		const double &top, 
		const std::vector<int> block_list );
	static bool CheckSolution(CPlaceDB& fplan, 
		const double &left, 
		const double &bottom, 
		const double &right, 
		const double &top, 
		const std::vector<int>& block_list );
	static bool Packing(CPlaceDB& fplan, const int& PartID);
	static bool CheckSolution(CPlaceDB& fplan, const int& PartID);

};

#endif
