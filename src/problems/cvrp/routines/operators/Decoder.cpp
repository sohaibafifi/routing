#include "Decoder.hpp"


bool CVRP::Decoder::decode(const std::vector<routing::models::Client *> & p_sequence, routing::models::Solution * solution)
{
    std::vector<routing::models::Client *> sequence = p_sequence;
    while(solution->getNbTour() < solution->getProblem()->vehicles.size()) {
        solution->pushTour(new Tour(static_cast<Problem*>(solution->getProblem()), solution->getNbTour() ));
    }
    bool insertion_found = false;
    while (!sequence.empty() && insertion_found) {
        for (unsigned cc = 0; cc < sequence.size(); ++cc) {
            unsigned best_t = 0, best_p = 0, best_client_i = 0;
            insertion_found = false;
            InsertionCost* bestCost = new InsertionCost(IloInfinity,true);
            InsertionCost* cost = new InsertionCost(0,true);

            bool insertion_found = true;
            Client * client = static_cast<Client*>(sequence[cc]);

            for (unsigned r = 0; r < static_cast<Solution*>(solution)->getNbTour(); ++r) {
                for (unsigned i = 0; i <= int(static_cast<Tour*>(static_cast<Solution*>(solution)->getTour(r))->getNbClient()); ++i) {
                    cost = static_cast<CVRP::InsertionCost*>(static_cast<Tour*>(static_cast<Solution*>(solution)->getTour(r))->evaluateInsertion(client, i));
                    if(!cost->isPossible()) continue;
                    if (bestCost > cost) {
                        insertion_found = true;
                        best_t = r;
                        best_p = i;
                        best_client_i = cc;
                        *bestCost = *cost;
                    }
                }
            }

            if (insertion_found) {

                static_cast<Solution*>(solution)->getTour(best_t)->addClient(sequence[best_client_i], best_p );
                static_cast<Solution*>(solution)->traveltime += bestCost->getDelta();
                static_cast<Solution*>(solution)->notserved.erase(sequence.begin() + best_client_i);

            }
        }
    }
    return insertion_found;
}

