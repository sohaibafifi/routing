//
// Created by ali on 4/3/19.
//

#ifndef HYBRID_CONFIGURECALLBACKSCALL_H
#define HYBRID_CONFIGURECALLBACKSCALL_H


class Config {
    //confi class to activate or deactivate a callback passed to cplex
    //by default all callbacks are active
    private:
        bool activeHeuristicCallback = true;
        bool activeUserCutCallback = true;
        bool activeIncumbentCallback = true;
        bool activeInformationCallback = true;
        bool activeLazyConstraintCallback = true;
    public:
        Config(bool _activeHeuristicCallback, bool _activeUserCutCallback,
                bool _activeIncumbentCallback, bool _activeInformationCallback,
                bool _activeLazyConstraintCallback):
        activeHeuristicCallback(_activeHeuristicCallback),
        activeUserCutCallback(_activeUserCutCallback),
        activeIncumbentCallback(_activeIncumbentCallback),
        activeInformationCallback(_activeInformationCallback),
        activeLazyConstraintCallback(_activeLazyConstraintCallback)
        {
        }

        Config(){ //empty para constructor, sets all vars to true by default
            this->activeHeuristicCallback = true;
            this->activeUserCutCallback = true;
            this->activeIncumbentCallback = true;
            this->activeInformationCallback = true;
            this->activeLazyConstraintCallback = true;
        }

        //getters
        bool getActiveHeuristicCallback() const {return this->activeHeuristicCallback;}
        bool getActiveUserCutCallback() const {return this->activeUserCutCallback;}
        bool getActiveIncumbentCallback() const {return this->activeIncumbentCallback;}
        bool getActiveInformationCallback() const {return this->activeInformationCallback;}
        bool getActiveLazyConstraintCallback() const { return  this->activeLazyConstraintCallback;}

        //setters
        void setActiveHeuristicCallback(bool _activeHeuristicCallback){
            this->activeHeuristicCallback = _activeHeuristicCallback;
        }

        void setActiveUserCutCallback(bool _activeUserCutCallback){
            this->activeUserCutCallback= _activeUserCutCallback;
        }

        void setActiveIncumbentCallback(bool _activeIncumbentCallback){
            this->activeIncumbentCallback= _activeIncumbentCallback;
        }

        void setActiveInformationCallback(bool _activeInformationCallback){
            this->activeInformationCallback= _activeInformationCallback;
        }

        void setActiveLazyConstraintCallback(bool _activeLazyConstraintCallback){
            this->activeLazyConstraintCallback = _activeLazyConstraintCallback;
        }



};

#endif //HYBRID_CONFIGURECALLBACKSCALL_H
