#include "Client.hpp"
#include "Depot.hpp"
#include "Solution.hpp"
#include "Tour.hpp"
#include "Vehicle.hpp"
namespace routing {
    namespace models {

        Solution::Solution(Problem *p_problem):
            problem(p_problem),
            notserved(std::vector<Client *>()){
            for (unsigned i  = 0; i < problem->clients.size(); ++i) {
                notserved.push_back(problem->clients[i]);
            }
        }

        Problem *Solution::getProblem() {
            return problem;
        }

        Solution::~Solution(){}
    }

    Model::~Model(){}

}
