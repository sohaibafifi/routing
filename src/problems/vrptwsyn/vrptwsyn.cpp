#include "vrptwsyn.hpp"
#include "../../Utility.hpp"
VRPTWSyn::Problem *VRPTWSyn::Reader::readFile(std::string filepath)
{
	VRPTWSyn::Problem *problem = new VRPTWSyn::Problem();
	std::fstream fh(filepath.c_str(), std::ios_base::in);
	try {
		problem->name = Utilities::filename(filepath);
		routing::Demand _maxCapacity = 1000;
		double timeFactor = 1.0;
		if (problem->name.find("case_3_50_10") != std::string::npos)
			timeFactor = 1.4;
		if (problem->name.find("case_1_80_16") != std::string::npos)
			timeFactor = 1.4;
		std::fstream fh(filepath.c_str(), std::ios_base::in);

		if (!fh.good()) {
			std::cerr << "file open error" << std::endl;
			exit(EXIT_FAILURE);
		}
		std::vector<std::string> namefield;
		namefield = Utilities::splitString(problem->name, '_');

		std::string line;
		std::vector<std::string> field;
		unsigned nbClients, nbVehicles;
		routing::Duration maxLength;
		std::vector<unsigned> demands;
		std::vector<double> preferences;
		std::vector<routing::Duration> twOpens, twCloses, services;
		std::vector<std::vector<routing::Duration> > distances;
		while (getline(fh, line)) {
			if (line[0] == '\0' || line[0] == '\n' || line[0] == '#')
				continue;
			if (line.find("NO_Staff") != std::string::npos) {
				field = Utilities::splitString(line, ' ');
				std::stringstream(field[3]) >> nbVehicles;
				for (unsigned i = 0; i < nbVehicles; ++i) {
					problem->vehicles.push_back(new Vehicle(i));
				}
			}
			else if (line.find("param NO_Visits") != std::string::npos) {
				field = Utilities::splitString(line, ' ');
				std::stringstream(field[3]) >> nbClients;
			}
			else if (line.find("param T_MAX") != std::string::npos) {
				field = Utilities::splitString(line, ' ');
				std::stringstream(field[3]) >> maxLength;
			}
			else if (line.find("param nModeOfTravel") != std::string::npos) {
				// Utilities::splitString(line, field, ' ');
				// stringstream(field[3]) >> this->_nModeOfTravel;
			}
			else if (line.find("param extra_staff_penalty") != std::string::npos) {
				//Utilities::splitString(line, field, ' ');
				//stringstream(field[3]) >> this->_extra_staff_penalty;
			}
			else if (line.find("Demands") != std::string::npos) {
				unsigned int nbdemands = nbClients;
				demands = std::vector<unsigned>(nbClients + 1);
				for (unsigned int v = 1; v <= nbdemands; ++v) {
					std::vector<std::string> field1, field2, field3, field4;
					unsigned demand = 0;
					field1 = Utilities::splitString(line, '[');
					field2 = Utilities::splitString(field1[1], ']');
					unsigned int visit;
					std::stringstream(field2[0]) >> visit;
					if (line.find("empty") == std::string::npos) {
						field3 = Utilities::splitString(field2[1], ' ');
						field4 = Utilities::splitString(field3[1], ';');
						std::stringstream(field4[0]) >> demand;
					}
					demands[visit] = demand;
					if (!getline(fh, line)) {
						std::cerr << "visit demands data missing" << std::endl;
						exit(EXIT_FAILURE);
					}
				}

			}
			else if (line.find(" a ") != std::string::npos) {
				twOpens = std::vector<routing::Duration>(nbClients + 1);
				twCloses = std::vector<routing::Duration>(nbClients + 1);

				for (int v = 0; v <= nbClients; ++v) {
					if (!getline(fh, line)) {
						std::cerr << "visit tw data missing" << std::endl;
						exit(EXIT_FAILURE);
					}
					field = Utilities::splitString(line, ' ');
					routing::Duration _twOpen, _twClose;
					unsigned client = 0;
					std::stringstream(field[0]) >> client;
					std::stringstream(field[1]) >> _twOpen;
					std::stringstream(field[2]) >> _twClose;
					twOpens[client] = _twOpen;
					twCloses[client] = _twClose;
				}
				getline(fh, line);
			}
			else if (line.find("Duration") != std::string::npos) {
				services = std::vector<routing::Duration>(nbClients + 1);
				while (true) {
					if (!getline(fh, line)) {
						std::cerr << "visit duration data missing" << std::endl;
						exit(EXIT_FAILURE);
					}
					if (line.find(";") != std::string::npos)
						break;
					field = Utilities::splitString(line, ' ');
					for (unsigned int i = 0; i < field.size() / 2; i += 1) {
						unsigned int v;
						std::stringstream(field[i * 2]) >> v;
						routing::Duration serviceDuration;
						std::stringstream(field[(i * 2) + 1]) >> serviceDuration;
						services[v] = floor(((double) serviceDuration) * timeFactor);
					}
				}
			}
			else if (line.find("TimeMatrix") != std::string::npos) {
				distances.resize(nbClients + 2);
				int arrival;
				do {
					std::string headerline;
					std::vector<std::string> header, elems;
					getline(fh, headerline);
					header = Utilities::splitString(headerline, ' ');
					for (int v = 0; v <= nbClients + 1; ++v) {
						distances[v].resize(nbClients + 2);
						if (!getline(fh, line)) {
							std::cerr << "visit time window data missing" << std::endl;
							exit(EXIT_FAILURE);
						}

						elems = Utilities::splitString(line, ' ');
						for (unsigned int f = 1; f < header.size() - 1; ++f) {
							std::stringstream(header[f]) >> arrival;
							std::stringstream(elems[f]) >> distances[v][arrival];
						}
					}
					getline(fh, headerline); // blank line
				}
				while (arrival < nbClients + 1);

			}
			else if (line.find("BonusMatrix") != std::string::npos) {
				preferences = std::vector<double>(nbClients + 1);

				int vehicle = 0;
				do {
					std::string headerline;
					std::vector<std::string> header, elems;
					getline(fh, headerline);
					header = Utilities::splitString(headerline, ' ');
					for (int v = 1; v < nbClients + 1; ++v) {
						//_clients[v]._preferences.resize(_numberVehicles);
						if (!getline(fh, line)) {
							std::cerr << "visit preference data missing" << std::endl;
							exit(EXIT_FAILURE);
						}
						elems = Utilities::splitString(line, ' ');
						for (unsigned int f = 1; f < header.size() - 1; ++f) {
							std::stringstream(header[f]) >> vehicle;
							double pref;
							std::stringstream(elems[f]) >> pref;
							//_clients[v]._preferences[vehicle - 1] =  (pref);
						}
					}
					getline(fh, headerline); // blank line

				}
				while (vehicle < nbVehicles);

				//getline(fh, line); //blank line

			}
			else {
				if (line != ";") std::cerr << "unkown line : [" << line << "]" << std::endl;
			}
		}
		fh.close();
		problem->clients.clear();
		for (int i = 0; i < nbClients + 1; ++i) {
			routing::TW tw;
			tw.first = twOpens[i];
			tw.second = twCloses[i];
			if (i == 0)
                problem->setDepot(new Depot(1, 0, 0, tw));
			else
				problem->clients.push_back(new Client(i, 1, services[i], tw, 0, 0));
		}
		problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
        problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
        problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
        for (unsigned i = 0; i < nbClients; ++i) {
			problem->distances[i] = std::vector<routing::Duration>(nbClients);
            problem->distances_to_depots[0][i] = distances[0][i + 1];
			for (unsigned j = 0; j < nbClients; ++j) {
				problem->distances[i][j] = distances[j + 1][i + 1];

				if (i != j &&
					fabs(10000 - problem->distances[i][j]) < std::numeric_limits<double>::epsilon()) {
					static_cast<Client *>(problem->clients[i])
						->addSynchronization(static_cast<Client *>(problem->clients[j]));
				}
			}

		}
		return problem;

	}
	catch (std::string emsg) {
		throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " : " + emsg);
	}

}


IloModel VRPTWSyn::Problem::generateModel(IloEnv &env)
{
	model = CVRPTW::Problem::generateModel(env);
	addSynchronizationConstraints();
	addnvConstraint();
	return model;
}

void VRPTWSyn::Problem::addSynchronizationConstraints()
{
	for (unsigned i = 1; i < clients.size(); ++i) {
		for (int s = 0; s < static_cast<Client *>(clients[i])->getSynchronizations().size(); ++s) {
			unsigned service1 = clients[i]->getID(), service2 = static_cast<routing::Model *>(
				static_cast<Client *>(clients[i])->getSynchronizations()[s])->getID();

			model.add(arcs[service1][service2] == 0);
			model.add(start[service1] == start[service2]);
		}
	}
}

void VRPTWSyn::Problem::addnvConstraint()
{
	nv = IloExpr (model.getEnv());
	for (unsigned i = 1; i < clients.size(); ++i) {
		nv = arcs[0][clients[i]->getID()];
	}
	model.add(nv <= IloNum(vehicles.size()));
 }


void VRPTWSyn::Problem::addTotalDistanceObjective()
{
	CVRP::Problem::addTotalDistanceObjective();
	obj.removeFromAll();
    model.add(IloMinimize(model.getEnv(), obj * 9.0 / static_cast<Depot*>(getDepot())->getTwClose()));
}

void VRPTWSyn::Problem::addVariables()
{
	CVRPTW::Problem::addVariables();
}

void VRPTWSyn::Problem::addSequenceConstraints()
{
	CVRPTW::Problem::addSequenceConstraints();
}

routing::callback::HeuristicCallback *VRPTWSyn::Problem::setHeuristicCallback(IloEnv &env)
{
    return new HeuristicCallback(env, this, new routing::dummyConstructor(), new routing::dummyDestructor());
}
routing::callback::IncumbentCallback *VRPTWSyn::Problem::setIncumbentCallback(IloEnv &env)
{
	return new IncumbentCallback(env, this);
}
