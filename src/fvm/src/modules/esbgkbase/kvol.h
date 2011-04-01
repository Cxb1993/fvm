#ifndef _KVOL_H_
#define _KVOL_H_

#include <vector>
#include "Vector.h"
#include "pmode.h"

template<class T>
class kvol
{
  
 public:

  typedef Vector<T,3> Tvec;
  typedef pmode<T> Tmode;
  typedef vector<Tmode*> Modes;
  /*
  kvol(int n)
    {
      _modenum=n;
      _modes=*(new Modes);
      for (int i=0;i<n;i++)
      	{
	  _modes.push_back(new Tmode);
	}
	}*/

  kvol(Tmode* mode,T dk3)     //used in gray approx.
    {
      _modenum=1;
      _modes=*(new Modes(_modenum,NULL));
      _modes[0]=mode;
      _dk3=dk3;
    }

  Tvec getkvec() {return _Kvector;}
  void setkvec(Tvec K) {_Kvector=K;}
  void setdk3(T dk3) {_dk3=dk3;}
  T getdk3() {return _dk3;}
  int getmodenum() {return _modenum;}
  /* void setmode(int n,Tmode mode)
  {
    _modes[n]=mode;
    }*/
  Tmode getmode(int n) const {return *_modes[n];}
  Tmode* getmodeptr(int n) {return _modes[n];}

 private:

  //weight factor (1/m3)
  T _dk3;

  //K vector
  Tvec _Kvector;

  //number of modes
  int _modenum;

  //array of modes
  Modes _modes;
  
};

#endif
