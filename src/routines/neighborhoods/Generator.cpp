#include "Generator.hpp"
#include <algorithm>    // std::random_shuffle

bool routing::Generator::generate(routing::models::Solution * solution)
{
    solution->notserved.clear();
    for (unsigned long c = 0; c < solution->getProblem()->clients.size(); ++c) {
        solution->notserved.push_back( solution->getProblem()->clients[c]);
    }
    std::random_shuffle ( solution->notserved.begin(), solution->notserved.end() );

    unsigned iter = 1;
    while(iter < solution->getProblem()->clients.size()){
        if(constructor->bestInsertion(solution)){
            return true;
        }else {
            switch (Configuration::destructionPolicy){
                case Configuration::DestructionPolicy::RANDOM :{
                    routing::RandomDestructionParameters* parameters = new routing::RandomDestructionParameters(solution->notserved.size() + 1);
                    destructor->destruct(solution, parameters);
                }
            }

        }
    }
    return false;
}
