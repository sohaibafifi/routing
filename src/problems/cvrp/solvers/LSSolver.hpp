#ifndef LSSOLVER_HPP
#define LSSOLVER_HPP

#include "../../../solvers/LSSolver.hpp"
#include "../../../routines/neighborhoods/IDCH.hpp"
#include "../models/Solution.hpp"
#include "../routines/operators/Constructor.hpp"
#include "../routines/operators/Destructor.hpp"
#include "../../../routines/operators/Constructor.hpp"
namespace CVRP {
    template <class Reader>
    class LSSolver : public routing::LSSolver<Reader>{
          public :
            LSSolver(const std::string & p_inputFile):
            routing::LSSolver<Reader>(p_inputFile,
                                      this->getGenerator(),
                                      this->getNeighbors())
            {
            }

            virtual void initSolution() override
            {
                this->solution = new Solution(static_cast<CVRP::Problem*>(this->problem));
            }

            static std::vector<routing::Neighborhood*> getNeighbors(){
                std::vector<routing::Neighborhood*> neighbors{
                    new routing::IDCH(new Constructor, new Destructor)
                };
                return neighbors;
            }
            static  routing::Generator * getGenerator(){
                return new routing::Generator(new Constructor, new Destructor);
            }


    };
}


#endif // LSSOLVER_HPP
