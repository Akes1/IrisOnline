#pragma once

struct sVehicleList;

class cVehicleScript
{
public:
	static cVehicleScript*		mpVehicleScript;

public:
	cVehicleScript();
	~cVehicleScript();

	///
	bool Init();

	///
	sVehicleList* GetVehicleInfo( unsigned long vehicleIdx );

protected:
	bool LoadVehicleList();

protected:
	typedef tPointerHashMap<unsigned long, void*> cPointHashMap;
	cPointHashMap				mVehicleMap;
};

#define VEHICLESCRIPT cVehicleScript::mpVehicleScript
