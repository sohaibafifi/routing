//
// Created by ali on 6/26/19.
//

#ifndef HYBRID_TSPTWDESTRUCTIONPARAMETERS_HPP
#define HYBRID_TSPTWDESTRUCTIONPARAMETERS_HPP

#include "DestructionParameters.hpp"
#include "../../data/models/Client.hpp"
namespace routing {
    class TSPTWDestructionParameters : public DestructionParameters {
    private:
        unsigned long startPosition;
        unsigned long endPosition;
        int tourIndex;

    public:
        std::vector<routing::models::Client*> toRoute;

        unsigned long getStartPosition() const {
            return startPosition;
        }

        void setStartPosition(unsigned long startPosition) {
            TSPTWDestructionParameters::startPosition = startPosition;
        }

        unsigned long getEndPosition() const {
            return endPosition;
        }

        void setEndPosition(unsigned long endPosition) {
            TSPTWDestructionParameters::endPosition = endPosition;
        }

        int getTourIndex() const {
            return tourIndex;
        }

        void setTourIndex(int tourIndex) {
            TSPTWDestructionParameters::tourIndex = tourIndex;
        }

        routing::models::Client* getToRouteClient(unsigned int i ) const{ return toRoute[i]; }

        void setToRoute(const std::vector<models::Client *> &toRoute) {
            TSPTWDestructionParameters::toRoute = toRoute;
        }


        const std::vector<models::Client *> &getToRoute() const {
            return toRoute;
        }


        void addClientToRout(routing::models::Client* client) { toRoute.push_back(client);}

        TSPTWDestructionParameters(unsigned long startPosition, unsigned long endPosition, int tourIndex, std::vector<routing::models::Client*> toRoute)
                : startPosition(startPosition), endPosition(endPosition), tourIndex(tourIndex), toRoute(toRoute) {}

    };
}

#endif //HYBRID_TSPTWDESTRUCTIONPARAMETERS_HPP
