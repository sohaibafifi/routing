#pragma once
#include "../../data/problem.hpp"
#include "../../data/attributes.hpp"
#include "../../data/reader.hpp"
#include "../cvrp/cvrp.hpp"
#include <sstream>


namespace CCVRP {
    class Problem : public CVRP::Problem{
        public :
            friend class CVRP::Reader;
            Problem(CVRP::Problem & problem):CVRP::Problem(problem){ }
        protected :
            virtual void addVariables() override;
            virtual void addSequenceConstraints ()override; // override to use demands for the sequence instead of order
            virtual void addObjective() override;
            std::vector<IloNumVar> arrival;
            void addTotalArrivalsObjective();
    };
    class Reader : public  CVRP::Reader{
        public:
            virtual CCVRP::Problem *readFile(std::string filepath);
            virtual ~Reader(){}
    };

}
