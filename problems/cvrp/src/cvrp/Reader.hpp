//
// Created by Sohaib LAFIFI on 20/11/2019.
//

#pragma once


#include <plugins/readers/ReaderCorePlugin/Reader.hpp>

namespace cvrp {
    class Reader : public routing::Reader {
    public :
        routing::Problem *readFile(const std::string &filePath) override;
    };

}

