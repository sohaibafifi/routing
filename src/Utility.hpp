#ifndef UTILITY_H_
#define UTILITY_H_

#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <set>
#include <stack>
#include <algorithm>
namespace Utilities{

    std::string filename(std::string path);
    std::vector<std::string>  splitString(std::string line, char sep);
    std::string itos(int i);
    std::string itos(unsigned i);
    void debug(std::string message) ;
    bool less_vectors(const std::vector<unsigned int>& a,const std::vector<unsigned int>& b);
#ifdef _WIN32
    inline int round_(float x);
    inline int round_(double x);
    inline int trunc_(float x) ;
    inline int trunc_(double x) ;
#endif
    struct NodeLabel {
            bool visited; // visited
            bool stacked; // currently in the stack
            unsigned  int  index; // index
            unsigned  int  lowlink; // label
            NodeLabel(): visited(false), stacked(false), index(0), lowlink(0) {	}
    };
    /**
      * @n       : number of clients
      * @arc     : solution pairs, returned by Cplex
      * @adjred  : 2 dimentions vector, it is an OUT variable containing the converted format
      */
    void convertToAdjRel(const unsigned int & n,
                         const std::vector<std::pair<unsigned int, unsigned int> > & arc,
                         std::vector<std::vector<unsigned int> > & adjrel);
    /**
      *
      * @adjrel     : converted format from convertToAdjrel
      * @nodeLabel  : a vector of node of the graph
      * @nodeStack  : a stack containing nodes
      * @nodeIndex  : the root from which we start the exploration of the graph
      * @v          : the current edge or vertex
      * @component  : the final set of sets of nodes forming a set of strong components
      *
      */
    void applyTarjanAlgorithm(const std::vector<std::vector<unsigned int> > & adjrel,
                              std::vector<std::vector<unsigned int> > & component);
    void tarjanAlgorithm(const std::vector<std::vector<unsigned > > & adjrel,
                         std::vector<NodeLabel> & nodeLabel,
                         std::stack<unsigned  int > & nodeStack,
                         unsigned  int  & nodeIndex, unsigned  int  v,
                         std::vector<std::vector<unsigned  int > > & component);
} // end namespace

#endif /*UTILITY_H_*/
