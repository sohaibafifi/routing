//
// Created by Sohaib LAFIFI on 22/11/2019.
//

// Copyright (c) 2020. Sohaib LAFIFI <sohaib.lafifi@univ-artois.fr>
// You are allowed to use this project for research purposes as a member of
// a non-commercial and academic institution.

#pragma once


#include "MASolver.hpp"
#include "Solver.hpp"
#include "../routines/operators/Generator.hpp"
#include "../routines/neighborhoods/Neighborhood.hpp"
#include <cassert>
#include <algorithm>
#include <set>
#include <map>


namespace routing {
    struct PSOKey {
        PSOKey(const double &length)
                : _cost(length) {}

        bool operator<(const PSOKey &key) const {
            return (this->_cost - key._cost < -0.01);
        }

        bool operator>(const PSOKey &key) const {
            return (this->_cost - key._cost > 0.01);
        }

        bool operator==(const PSOKey &key) {
            return (fabs(this->_cost - key._cost) <= 0.01);
        }

        double _cost;
    };

    class Particle {
    public :
        Particle(Problem *p_problem) :
                problem(p_problem),
                sequence(new Sequence(p_problem)),
                local_best_position(new Sequence(p_problem)) {
            best_cost = 0.0;
        }

        void initializePositionFromSolutions(models::Solution *solution_1, models::Solution *solution_2) {
             Sequence *sequence_1 = new Sequence(solution_1);
             Sequence *sequence_2 = new Sequence(solution_2);
             double cost_1 = sequence_1->decode()->getCost();
             double cost_2 = sequence_2->decode()->getCost();
            if(cost_1 < cost_2){
                this->local_best_position = sequence_1;
                this->best_cost = cost_1;

                this->sequence = sequence_2;
                this->cost = cost_2;
            }else{
                this->local_best_position = sequence_2;
                this->best_cost = cost_2;

                this->sequence = sequence_1;
                this->cost = cost_1;
            }

        }

        void initializeBestPositionFromSolution(models::Solution *solution) {
            Sequence *sequence = new Sequence(solution);
            this->local_best_position = sequence;
            this->best_cost = sequence->decode()->getCost();
        }



        void updateCurrentPosition(double inertia, double local, double global,
                                   Sequence *global_best);

        void returnToLocalBest(){
             *this->sequence = *this->local_best_position;
             this->cost = this->best_cost;
        }

    protected:
        routing::Duration cost;
        routing::Duration best_cost;
        Sequence *sequence;
        Sequence *local_best_position;
        Problem *problem;
    };


    template<class Reader>
    class PSOSolver : public MASolver<Reader> {
    protected:
        std::vector<Particle> swarm;
        std::map<PSOKey, unsigned int> sorted_swarm;

        virtual void initializeSwarm();

        virtual bool addParticleToSwarm(Particle &particle);

        virtual bool updateLocalBestSwarm(unsigned int i);

    public:
        PSOSolver(const std::string &p_inputFile,
                  Generator *p_generator,
                  const std::vector<Neighborhood *> &p_neighbors,
                  std::ostream &os = std::cout) : MASolver<Reader>(p_inputFile, p_generator, p_neighbors, os) {

        }

        PSOSolver(const std::string &p_inputFile,
                  std::ostream &os = std::cout) : MASolver<Reader>(p_inputFile, os) {

        }


    };
}
