#ifndef _PHONONBC_H_
#define _PHONONBC_H_

#include "misc.h"
#include "FloatVarDict.h"

template<class T>
struct PhononBC : public FloatVarDict<T>
{
  PhononBC()
  {
      this->defineVar("specifiedTemperature",T(300.0));
      this->defineVar("specifiedreflection",T(0.0));
      this->defineVar("specifiedHeatflux",T(0.0));
  }
  string bcType;
};

template<class T>
struct PhononModelOptions : public FloatVarDict<T>
{
  PhononModelOptions()
  {
    this->defineVar("timeStep",T(0.1));
    this->defineVar("initialTemperature",T(300.0));
    this->defineVar("Tref",T(299.0));
    this->printNormalizedResiduals = true;
    this->transient = false;
    this->timeDiscretizationOrder=1;
    this->constantcp=true;
    this->constanttau=true;
  }
  
  bool printNormalizedResiduals;
  bool transient;
  int timeDiscretizationOrder;
  bool constantcp;
  bool constanttau;
};


#endif
