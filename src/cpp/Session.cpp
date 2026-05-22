#include "Session.h"

std::vector<Feedback> Session::currentFeedbacks;
std::map<std::string, std::vector<Feedback>> Session::sessionFeedbacks;
std::map<std::string, std::string> Session::internalData;
std::map<std::string, std::string> Session::filterOptions;
