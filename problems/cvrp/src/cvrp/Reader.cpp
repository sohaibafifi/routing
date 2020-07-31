//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#include "Reader.hpp"

#include "models/Client.hpp"
#include "models/Vehicle.hpp"
#include "Problem.hpp"
#include <cmath>
#include <queue>
#include <utilities/Utilities.hpp>
#include <vrp/models/Depot.hpp>
#include <fstream>

routing::Problem *cvrp::Reader::readFile(std::string filePath) {
    cvrp::Problem *problem = new cvrp::Problem();
    std::fstream fh(filePath.c_str(), std::ios_base::in);
    try {
        if (!fh.good()) throw std::string("file open error");
        unsigned short edge_weight_type;
        const unsigned short
                EXPLICIT = 0,
                EUC_2D = 2,
                EUC_3D = 3,
                MAX_2D = 4,
                MAX_3D = 5,
                MAN_2D = 6,
                MAN_3D = 7,
                CEIL_2D = 8,
                GEO = 9,
                ATT = 10,
                XRAY1 = 11,
                XRAY2 = 12,
                SPECIAL = 13;

        unsigned short edge_weight_format;
        const unsigned short
                FUNCTION = 0,
                FULL_MATRIX = 1,
                UPPER_ROW = 2,
                LOWER_ROW = 3,
                UPPER_DIAG_ROW = 4,
                LOWER_DIAG_ROW = 5,
                UPPER_COL = 6,
                LOWER_COL = 7,
                UPPER_DIAG_COL = 8,
                lOWER_DIAG_COL = 9;

        unsigned short edge_data_format;
        const unsigned short EDGE_LIST = 0, ADJ_LIST = 1;

        unsigned short node_coord_type = 0;
        const unsigned short TWOD_COORDS = 0, THREED_COORDS = 1, NO_COORDS = 2;

        std::string line, dummy;
        unsigned nbClients, nbVehicles;
        std::vector<routing::Demand> demand;
        std::vector<routing::Duration> x, y;
        routing::Duration x_depot = 0, y_depot = 0;

        while (getline(fh, line)) {
            if (line.find("NAME") != std::string::npos) {
                std::stringstream(line) >> dummy >> dummy >> problem->name;
                std::cout << "problem name : " << problem->name << std::endl;
                std::vector<std::string> parts = Utilities::splitString(problem->name, '-');
                parts[2][0] = '0';
                std::stringstream(parts[2]) >> nbVehicles;
            }
            if (line.find("COMMENT") != std::string::npos) {
                std::cout << line << std::endl;
            }
            if (line.find("TYPE") != std::string::npos) {

            }
            if (line.find("DIMENSION") != std::string::npos) {
                std::stringstream(line) >> dummy >> dummy >> nbClients;
                nbClients--;
            }
            if (line.find("CAPACITY") != std::string::npos) {
                routing::Capacity Q;
                std::stringstream(line) >> dummy >> dummy >> Q;
                for (int i = 0; i < nbVehicles; ++i) {
                    problem->vehicles.push_back(new cvrp::models::Vehicle(i, Q));
                }
            }
            if (line.find("EDGE_WEIGHT_TYPE") != std::string::npos) {
                if (line.find("EXPLICIT") != std::string::npos) edge_weight_type = EXPLICIT;
                if (line.find("EUC_2D") != std::string::npos) edge_weight_type = EUC_2D;
                if (line.find("EUC_3D") != std::string::npos) edge_weight_type = EUC_3D;
                if (line.find("MAX_2D") != std::string::npos) edge_weight_type = MAX_2D;
                if (line.find("MAX_3D") != std::string::npos) edge_weight_type = MAX_3D;
                if (line.find("MAN_2D") != std::string::npos) edge_weight_type = MAN_2D;
                if (line.find("MAN_3D") != std::string::npos) edge_weight_type = MAN_3D;
                if (line.find("CEIL_2D") != std::string::npos) edge_weight_type = CEIL_2D;
                if (line.find("GEO") != std::string::npos) edge_weight_type = GEO;
                if (line.find("ATT") != std::string::npos) edge_weight_type = ATT;
                if (line.find("XRAY1") != std::string::npos) edge_weight_type = XRAY1;
                if (line.find("XRAY2") != std::string::npos) edge_weight_type = XRAY2;
                if (line.find("SPECIAL") != std::string::npos) edge_weight_type = SPECIAL;

            }
            if (line.find("EDGE_WEIGHT_FORMAT") != std::string::npos) {
                if (line.find("FUNCTION") != std::string::npos) edge_weight_format = FUNCTION;
                if (line.find("FULL_MATRIX") != std::string::npos) edge_weight_format = FULL_MATRIX;
                if (line.find("UPPER_ROW") != std::string::npos) edge_weight_format = UPPER_ROW;
                if (line.find("LOWER_ROW") != std::string::npos) edge_weight_format = LOWER_ROW;
                if (line.find("UPPER_DIAG_ROW") != std::string::npos) edge_weight_format = UPPER_DIAG_ROW;
                if (line.find("LOWER_DIAG_ROW") != std::string::npos) edge_weight_format = LOWER_DIAG_ROW;
                if (line.find("UPPER_COL") != std::string::npos) edge_weight_format = UPPER_COL;
                if (line.find("LOWER_COL") != std::string::npos) edge_weight_format = LOWER_COL;
                if (line.find("UPPER_DIAG_COL") != std::string::npos) edge_weight_format = UPPER_DIAG_COL;
                if (line.find("lOWER_DIAG_COL") != std::string::npos) edge_weight_format = lOWER_DIAG_COL;
            }
            if (line.find("EDGE_DATA_FORMAT") != std::string::npos) {
                if (line.find("ADJ_LIST") != std::string::npos) edge_data_format = ADJ_LIST;
                if (line.find("EDGE_LIST") != std::string::npos) edge_data_format = EDGE_LIST;

            }
            if (line.find("NODE_COORD_TYPE") != std::string::npos) {
                if (line.find("TWOD_COORDS") != std::string::npos) node_coord_type = TWOD_COORDS;
                if (line.find("THREED_COORDS") != std::string::npos) node_coord_type = THREED_COORDS;
                if (line.find("NO_COORDS") != std::string::npos) node_coord_type = NO_COORDS;
            }
            if (line.find("DISPLAY_DATA_TYPE") != std::string::npos) {

            }
            if (line.find("EOF") != std::string::npos) break;
            if (line.find("NODE_COORD_SECTION") != std::string::npos) {
                if (node_coord_type == TWOD_COORDS) {
                    x = std::vector<routing::Duration>(nbClients);
                    y = std::vector<routing::Duration>(nbClients);
                    for (unsigned i = 0; i <= nbClients && getline(fh, line); ++i)
                        if (i == 0) std::stringstream(line) >> dummy >> x_depot >> y_depot;
                        else std::stringstream(line) >> dummy >> x[i - 1] >> y[i - 1];

                    problem->setDepot(new vrp::models::Depot(1, x_depot, y_depot));
                } else throw std::string("format not supported");
            }
            if (line.find("DEPOT_SECTION") != std::string::npos) {

            }
            if (line.find("DEMAND_SECTION") != std::string::npos) {
                demand = std::vector<routing::Demand>(nbClients);
                for (unsigned i = 0; i <= nbClients && getline(fh, line); ++i)
                    if (i == 0) continue;
                    else std::stringstream(line) >> dummy >> demand[i - 1];
            }
            if (line.find("EDGE_DATA_SECTION") != std::string::npos) {

            }
            if (line.find("DISPLAY_DATA_SECTION") != std::string::npos) {

            }
            if (line.find("FIXED_EDGES_SECTION") != std::string::npos) {

            }
            if (line.find("TOUR_SECTION") != std::string::npos) {

            }
            if (line.find("EDGE_WEIGHTmodels::DepotION") != std::string::npos) {
                problem->setDepot(new vrp::models::Depot(1, 0, 0));

                if (edge_weight_format == LOWER_ROW) {
                    x = std::vector<routing::Duration>(nbClients, 0); // not used
                    y = std::vector<routing::Duration>(nbClients, 0);

                    int nbValues = 0;//(nbClients - 1) * nbClients / 2;
                    for (int i = 0; i < nbClients + 1; ++i)
                        for (int j = 0; j < i; ++j) if (i != j) nbValues++;
                    bool stop = false;
                    std::queue<routing::Duration> distances;
                    while (nbValues != 0 && !stop) {
                        getline(fh, line);
                        std::vector<std::string> values = Utilities::splitString(line, ' ');

                        for (int i = 0; i < values.size(); ++i) {
                            if (values[i].empty()) stop = true;
                            routing::Duration _val;
                            std::stringstream(values[i]) >> _val;
                            distances.push(_val);
                        }
                        nbValues -= values.size();
                    }
                    problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
                    problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
                    problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
                    for (int i = 1; i < nbClients + 1; ++i) {
                        problem->distances[i - 1] = std::vector<routing::Duration>(nbClients);
                        for (int j = 0; j < i; ++j) {
                            if (j == 0) {
                                problem->distances_to_depots[0][i - 1] = distances.front();
                                distances.pop();
                            } else {
                                problem->distances[i - 1][j - 1] =
                                problem->distances[j - 1][i - 1] = distances.front();
                                distances.pop();
                            }
                        }
                    }
                }
            }

        }
        for (unsigned i = 1; i <= nbClients; ++i) {
            if (i == 0) continue;
            problem->clients.push_back(new models::Client(i, x[i - 1], y[i - 1], demand[i - 1]));
        }
        if (edge_weight_type == EUC_2D) {
            problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
            problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
            problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
            for (unsigned i = 0; i < nbClients; ++i) {
                problem->distances[i] = std::vector<routing::Duration>(nbClients);
                problem->distances_to_depots[0][i] =
                        round(std::sqrt((x[i] - x_depot) * (x[i] - x_depot) + (y[i] - y_depot) * (y[i] - y_depot)));
                for (unsigned j = 0; j < nbClients; ++j)
                    problem->distances[i][j] =
                            round(std::sqrt((x[i] - x[j]) * (x[i] - x[j]) + (y[i] - y[j]) * (y[i] - y[j])));
            }
        }
        return problem;

    }
    catch (std::string emsg) {
        throw std::string(std::string("cvrp::Reader::readFile : ")  + emsg);
    }

}
