#include "Utility.hpp"
namespace Utilities{
    std::string filename(std::string path) {
        std::string filename;
#ifdef _WIN32
        size_t pos = path.find_last_of("\\");
#else
        size_t pos = path.find_last_of("/");
#endif
        if (pos != std::string::npos)
            filename.assign(path.begin() + pos + 1, path.end());
        else
            filename = path;
        return filename.substr(0, filename.find_last_of('.'));
    }
    std::vector<std::string>  splitString(std::string line, char sep) {
        std::vector<std::string> split;
        split.clear();
        size_t pos;
        do {
            pos = line.find_first_of(sep);
            if (pos != 0)
                split.push_back(line.substr(0, pos));
            line = line.substr(pos + 1);
        } while (pos < line.npos);
        return split;
    }

    std::string itos(int i) // convert int to std::string
    {
        std::stringstream s;
        s << i;
        return s.str();
    }
    std::string itos(unsigned i) // convert int to std::string
    {
        std::stringstream s;
        s << i;
        return s.str();
    }
    void debug(std::string message) {
#ifdef _DEBUG
        std::cout << message.c_str() << std::endl;
#endif
    }

    bool less_vectors(const std::vector<unsigned int>& a,const std::vector<unsigned int>& b)
    {
        return a.size() < b.size();
    }

#ifdef _WIN32
    inline int round_(float x) { return (floor(x + 0.5)); }
    inline int round_(double x) { return (floor(x + 0.5)); }
    inline int trunc_(float x) { return (x>0) ? floor(x) : ceil(x) ; }
    inline int trunc_(double x) { return (x>0) ? floor(x) : ceil(x) ; }
#endif

    // ****** Modified Tarjan's algorithm to find strong connected components (with size>1) ******

    void convertToAdjRel(const unsigned int & n,
                         const std::vector<std::pair<unsigned int, unsigned int> > & arc,
                         std::vector<std::vector<unsigned int> > & adjrel)
    {
        adjrel.clear();
        adjrel.resize(n);
        //put the seconde element of the pair (the i eme arc) in the end of the i eme vector of adjrel
        for (unsigned  int  i = 0; i < arc.size(); i++) {
            adjrel[arc[i].first].push_back(arc[i].second);
        }
    }

    // modified Tarjan's algorithm (only strong component of size>1 is saved)

    void tarjanAlgorithm(const std::vector<std::vector<unsigned > > & adjrel,
                         std::vector<NodeLabel> & nodeLabel,
                         std::stack<unsigned  int > & nodeStack,
                                     unsigned  int  & nodeIndex, unsigned  int  v,
                         std::vector<std::vector<unsigned  int > > & component) {


        nodeLabel[v].index = nodeIndex;//The number of the vertex or the edge
        nodeLabel[v].lowlink = nodeIndex;//The calculated value of access, it helps to detect the already visited nodes
        nodeLabel[v].visited = true;

        nodeIndex++;
        nodeStack.push(v);//put this visited node on the stack
        nodeLabel[v].stacked = true;//Mark it as visited
        //We have to visite all neighboors of the current node
        for (unsigned  int  i = 0; i < adjrel[v].size(); i++) { // labeling
            unsigned  int  w = adjrel[v][i];
            //if the node haven't been visited, then recall the function recursively
            if (!nodeLabel[w].visited) {
                tarjanAlgorithm(adjrel, nodeLabel, nodeStack, nodeIndex, w, component);
                nodeLabel[v].lowlink = std::min(nodeLabel[v].lowlink, nodeLabel[w].lowlink);
            } else if (nodeLabel[w].stacked) {
                nodeLabel[v].lowlink = std::min(nodeLabel[v].lowlink, nodeLabel[w].index);
            }
        }
        if (nodeLabel[v].lowlink == nodeLabel[v].index) { // found a root of a strong component
            if (!nodeStack.empty()) {
                std::vector<unsigned  int > tmp;
                unsigned  int  w;
                do {
                    w = nodeStack.top();
                    nodeStack.pop();
                    nodeLabel[w].stacked = false;
                    tmp.push_back(w);
                } while (w != v);
                if (tmp.size() > 1 && w != 0)
                    component.push_back(tmp);
            }
        }
    }

    void applyTarjanAlgorithm(const std::vector<std::vector<unsigned int> > & adjrel,
                              std::vector<std::vector<unsigned int> > & component)
    {
        std::vector<NodeLabel> nodeLabel(adjrel.size());
        std::stack<unsigned  int > nodeStack;
        component.clear();
        unsigned  int  nodeIndex = 0;
        for (unsigned  int  i = 0; i < adjrel.size(); i++) {
            if (!nodeLabel[i].visited)
                tarjanAlgorithm(adjrel, nodeLabel, nodeStack, nodeIndex, i, component);
        }
    }



} // end Utilities
