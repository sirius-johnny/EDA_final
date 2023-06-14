#include <string>
using namespace std;


// multilevel global placement using nonlinear programming
// return true is placement is legal
bool multilevel_nlp( CPlaceDB& placedb, string outFilePrefix, 
       int ctype,
       int clusterSize, double clusterRatio, double targetNNB, double targetDensity,
       double incFactor, double wWire, int maxLevel=INT_MAX, 
       double weightLevelDecreaingRate=2, double targetUtilization=1.0 );


