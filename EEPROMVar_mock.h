/*
*/
#include <Arduino.h>

class EEPROMClassEx
{
	  
  public:
	EEPROMClassEx()  {;};
    void 	 setMemPool(int base, int memSize)  {;};
	void  	 setMaxAllowedWrites(int allowedWrites)  {;};
};


template<typename T> class EEPROMVar 
{
	public:
	  EEPROMVar(T init) {
		var = init;
	  }
	  operator T () { 
		return var; 
	  }
	  EEPROMVar &operator=(T val) {
		var = val;
		return *this;
	  }
	  
	  void operator+=(T val) {
		var += T(val); 
	  }
	  void operator-=(T val) {
		var -= T(val); 
	  }	 	  
	  void operator++(int) {
		var += T(1); 
	  }
	  void operator--(int) {
		var -= T(1); 
	  }
	  void operator++() {
		var += T(1); 
	  }
	  void operator--() {
		var -= T(1); 
	  }
	  template<typename V>
		void operator /= (V divisor) {
		var = var / divisor;
	  }
	  template<typename V>
		void operator *= (V multiplicator) {
		var = var * multiplicator;
	  }
	  void save(){	   	   
	    ;
	  }
	  
	  void update(){	   	   
	    ;
	  }
	  
	  int getAddress(){	   	   
	    return 0;
	  }
	  
	  void restore(){
	  	;
	  }
	protected:	
	  T var;
};

extern EEPROMClassEx EEPROM;