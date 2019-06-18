#pragma once
#include "../attributes.hpp"
#include "ServiceQuery.hpp"
namespace routing {
    typedef std::pair<Duration , Duration> TW;

    namespace attributes {

        /**
 * @brief a node with a time window
 *
 */
        struct Rendezvous
        {
                Rendezvous(const TW & timewindow):tw(timewindow){}
                Rendezvous(){}
                EntityData<TW> tw;

                Duration getTwOpen()  const { return this->tw.getValue().first;}
                Duration getTwClose() const { return this->tw.getValue().second;}
                Duration getEST()  const { return this->getTwOpen();}
                Duration getLST() const { return this->getTwClose();}
        };

    }
}
