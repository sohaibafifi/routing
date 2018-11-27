//
// Created by afifi on 01/02/18.
//

#ifndef HYBRID_LSSolver_HPP
#define HYBRID_LSSolver_HPP
#include <string>
#include "../data/problem.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include "../routines/neighborhoods/Generator.hpp"
#include "Solver.hpp"
#include "../mtrand.hpp"
#include "../Utility.hpp"
namespace routing{
template <class Reader>
class LSSolver : public Solver<Reader>
{
    std::vector<routing::Neighborhood*> neighbors;
    routing::Generator * generator;
  public:
    LSSolver(const std::string & p_inputFile,
             routing::Generator * p_generator,
             const std::vector<Neighborhood *> &p_neighbors);
    virtual bool solve(double timeout = 3600) override;
    virtual ~LSSolver() ;
    virtual void initSolution() = 0;

};
}
template<class Reader>
routing::LSSolver<Reader>::LSSolver(const std::string & p_inputFile,
                           routing::Generator * p_generator,
                           const std::vector<routing::Neighborhood*> & p_neighbors)
    :Solver<Reader> (p_inputFile),
      generator(p_generator),
      neighbors(p_neighbors)
{

}

template<class Reader>
bool routing::LSSolver<Reader>::solve(double timeout)
{
    initSolution();
    generator->generate(this->solution);
    if(neighbors.empty()) return false;
    Utilities::MTRand_int32 irand(std::time(nullptr));
    bool improved = false;
    std::vector<bool> run(neighbors.size(), false);
    while(std::find(run.begin(), run.end(), false) != run.end()){
      unsigned i = 0;
      do{i = irand() % run.size();} while(run[i]);

         if(neighbors[i]->look(this->solution)){
             improved = true;
             run = std::vector<bool>(neighbors.size(), false);
         }else {
             run[i] = true;
         }
    }
    std::cout << this->problem->getName()
              << "\t" << this->solution->getCost()
              << std::endl;
}
template<class Reader>
routing::LSSolver<Reader>::~LSSolver()
{

}

#endif //HYBRID_LSSolver_HPP
