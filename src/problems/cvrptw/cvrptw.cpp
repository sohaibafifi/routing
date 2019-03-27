#include "cvrptw.hpp"
#include "../../Utility.hpp"
CVRPTW::Problem *CVRPTW::Reader::readFile(std::string filepath)
{
	CVRPTW::Problem *problem = new CVRPTW::Problem();
	std::fstream fh(filepath.c_str(), std::ios_base::in);
	try {

		if (!fh.good()) throw std::string("file open error");
		unsigned nbClients = 100;
		std::string line;
		getline(fh, line);
		problem->name = line;
		getline(fh, line);
		getline(fh, line);
		getline(fh, line);
		getline(fh, line);
		unsigned k;
		routing::Capacity Q;
		std::stringstream(line) >> k >> Q;
		for (unsigned i = 0; i < k; ++i) {
			problem->vehicles.push_back(new Vehicle(i, Q));
		}
		getline(fh, line);
		getline(fh, line);
		getline(fh, line);
		getline(fh, line);
		std::vector<routing::Duration> x(nbClients), y(nbClients);
		while (getline(fh, line)) {
			unsigned i;
			routing::Duration _x, _y, service;
			double twopen, twclose;
			routing::Demand demand;
			std::stringstream(line) >> i >> _x >> _y >> demand >> twopen >> twclose >> service;
			routing::TW tw;
			tw.first = twopen;
			tw.second = twclose;

			if (i == 0)
                problem->setDepot(new Depot(1, _x, _y, tw));
			else {
				problem->clients.push_back(new Client(i, demand, service, tw, _x, _y));
				x[i - 1] = _x;
				y[i - 1] = _y;
			}
		}
		problem->distances = std::vector<std::vector<routing::Duration> >(nbClients);
        problem->distances_to_depots = std::vector<std::vector<routing::Duration> >(1);
        problem->distances_to_depots[0] = std::vector<routing::Duration>(nbClients);
		for (unsigned i = 0; i < nbClients; ++i) {
			problem->distances[i] = std::vector<routing::Duration>(nbClients);
            problem->distances_to_depots[0][i] = floor(std::hypot(x[i] - problem->getDepot()->getX(), y[i] - problem->getDepot()->getY())*100)/100.0;
			for (unsigned j = 0; j < nbClients; ++j)
				problem->distances[i][j] = floor(std::hypot(x[i] - x[j], y[i] - y[j])*100)/100.0;
		}
		return problem;

	}
	catch (std::string emsg) {
		throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " : " + emsg);
	}

}

void CVRPTW::Problem::addVariables()
{
	CVRP::Problem::addVariables();
	for (unsigned i = 0; i <= clients.size(); ++i) {
		if (i == 0)
			start.push_back(IloNumVar(model.getEnv(),
                                      static_cast<Depot*>(getDepot())->getTwOpen(),
                                      static_cast<Depot*>(getDepot())->getTwClose(),
									  std::string("t_" + Utilities::itos(i)).c_str()));
		else
			start.push_back(IloNumVar(model.getEnv(),
									  static_cast<Client *>(clients[i - 1])->getTwOpen(),
									  static_cast<Client *>(clients[i - 1])->getTwClose(),
									  std::string("t_" + Utilities::itos(i)).c_str()));

		model.add(start.back());
	}
}

void CVRPTW::Problem::addSequenceConstraints()
{
	CVRP::Problem::addSequenceConstraints();
	for (unsigned i = 1; i <= clients.size(); ++i) {
        model.add(start[i] >= CVRPTW::Problem::getDistance(*clients[i - 1], *getDepot()));
		for (unsigned j = 0; j <= clients.size(); ++j) {
			if (i == j) continue;
			if (j == 0)
				model.add(start[i]
							  +
								  (
									  static_cast<Client *>(clients[i - 1])->getService()
                                          + CVRPTW::Problem::getDistance(*clients[i - 1], *getDepot())
								  ) * arcs[i][0]
							  <=
								  start[j]
									  +
										  (
											  static_cast<Client *>(clients[i - 1])->getTwClose()
                                                  - static_cast<Depot *>(getDepot())->getTwOpen()
										  ) * (1 - arcs[i][0]));
			else
				model.add(start[i]
							  +
								  (
									  static_cast<Client *>(clients[i - 1])->getService()
										  + routing::Problem::getDistance(*clients[i - 1], *clients[j - 1])
								  ) * arcs[i][j]
							  <=
								  start[j]
									  +
										  (
											  static_cast<Client *>(clients[i - 1])->getTwClose()
												  - static_cast<Client *>(clients[j - 1])->getTwOpen()
										  ) * (1 - arcs[i][j]));
		}

	}
}

routing::callback::HeuristicCallback *CVRPTW::Problem::setHeuristicCallback(IloEnv &env)
{
    std::vector<routing::Neighborhood*> neighborhoods;
    return new HeuristicCallback(env, this,
                                 new routing::Generator(new CVRP::Constructor, new CVRP::Destructor),
                                 neighborhoods);
}



CVRPTW::Solution::Solution(CVRPTW::Problem *problem):
    CVRP::Solution(problem),
    tours(std::vector<Tour *>()){
    visits = std::vector<Visit*>(problem->clients.size() + 1);
    for (unsigned long i = 0; i < problem->clients.size(); ++i) {
        Client * client = static_cast<Client *>(problem->clients[i]);
        visits[client->getID()] =
                    new Visit(client,
                              client->getEST(),
                              client->getLST() - client->getEST(),
                              0.0);
    }
}

void CVRPTW::Solution::updateMaxShifts()
{
    std::cout << "updating maxshift " << std::endl;
}

void CVRPTW::Solution::print(std::ostream &out)
{
    out << "solution cost " << getCost() << std::endl;
    for (unsigned t = 0;t < getNbTour();t++) {
        out << "tour " << t << " : ";
        for (unsigned i = 0; i < tours[t]->getNbClient(); ++i) {
            out << tours[t]->getClient(i)->getID() << "(" << static_cast<Client*>( tours[t]->getClient(i))->getService() << ") ";
        }
        out << std::endl;
    }
}

void CVRPTW::Solution::update()
{

}

void CVRPTW::Solution::addTour(routing::models::Tour *tour, unsigned long position)
{
    tours.insert(tours.begin() + position, static_cast<Tour*>(tour));
    traveltime += static_cast<Tour*>( tour )->traveltime;
}

routing::models::Solution *CVRPTW::Solution::clone() const
{
    return new Solution(*this);
}

routing::models::Tour *CVRPTW::Solution::getTour(unsigned t)
{
    return tours.at(t);
}

void CVRPTW::Tour::addClient(routing::models::Client *client, unsigned long position)
{
    CVRP::Tour::addClient(client, position);
    // update the visit and the tour

}
