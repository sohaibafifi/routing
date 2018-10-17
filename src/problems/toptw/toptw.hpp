#pragma once
#include "../cvrptw/cvrptw.hpp"
#include "../../data/attributes/Profiter.hpp"

namespace TOPTW {
    class Client : public CVRPTW::Client,
                   public routing::attributes::Profiter{
        public :
            Client(const CVRPTW::Client & cvrptwClient):
                CVRPTW::Client(cvrptwClient),
                routing::attributes::Profiter(cvrptwClient.getDemand()){}
    };
    class Reader;
    class Problem : public CVRPTW::Problem{
        friend class Reader;
        public :
            Problem(CVRPTW::Problem & problem): CVRPTW::Problem(problem){}
        protected:
            // change the obj function
            virtual void addObjective() override { this->addProfitsObjective(); }
            virtual void addProfitsObjective();
            virtual void addRoutingConstraints () override; // relax the coverage constraints


    };
    class Reader : public routing::Reader{
        public:
            virtual TOPTW::Problem *readFile(std::string filepath);
            virtual ~Reader(){}
    };
}
