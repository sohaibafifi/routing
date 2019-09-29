//
// Created by ali on 4/11/19.
//


#pragma once



#include "../models/Solution.hpp"
#include "../routines/operators/Constructor.hpp"
#include "../routines/operators/RandomDestructor.hpp"
#include "../routines/operators/SequentialDestructor.hpp"
#include "../../../solvers/LSSolver.hpp"
#include "../../../routines/neighborhoods/IDCH.hpp"
#include "Checker.hpp"
#include "../../../Configurations.hpp"
namespace CVRPTW{
    template <class Reader>
    class LSSolver: public routing::LSSolver<Reader> {
        public:
            LSSolver(const std::string & p_inputFile, std::ostream& os  = std::cout) :
                    routing::LSSolver<Reader>(p_inputFile,
                              this->getGenerator(),
                              this->getNeighbors(),
                              os)
            {

            }

            virtual void initSolution() override
            {
                this->solution = new Solution(static_cast<CVRPTW::Problem*>(this->problem));
            }

            virtual std::vector<routing::Neighborhood*> getNeighbors() {
                std::vector<routing::Neighborhood*> neighbors;
                switch (Configuration::destructionPolicy)
                {
                    case Configuration::DestructionPolicy::RANDOM:{
                        neighbors.push_back(new routing::IDCH(new Constructor, new RandomDestructor));
                        break;
                    }
                    case Configuration::DestructionPolicy::SEQUENTIAL:{
                        neighbors.push_back(new routing::IDCH(new Constructor, new SequentialDestructor));
                        break;
                    }
                    case Configuration::DestructionPolicy::NODST:{
                        neighbors.push_back(new routing::IDCH(new Constructor, new routing::dummyDestructor()));
                        break;
                    }

                };
                return neighbors;
            }

            virtual routing::Generator * getGenerator() {
                switch (Configuration::destructionPolicy)
                {
                    case Configuration::DestructionPolicy::RANDOM:{
                        return new routing::Generator(new Constructor, new RandomDestructor);
                    }
                    case Configuration::DestructionPolicy::SEQUENTIAL:{
                        return new routing::Generator(new Constructor, new SequentialDestructor);
                    }
                    case Configuration::DestructionPolicy::NODST:{
                        return new routing::Generator(new Constructor, new routing::dummyDestructor());
                    }

                };


            }


            virtual CVRPTW::Checker* getChecker() const {

                return new CVRPTW::Checker(static_cast<CVRPTW::Solution*>(this->solution),this->os);
            }
    };
}