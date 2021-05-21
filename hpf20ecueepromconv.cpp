// hpf20ecueepromconv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>
#include <tuple>
#include <algorithm>

using namespace std;

#define fieldsize(name, field) \
    (sizeof(((struct name *)0)->field) / 2)

string remove_ws(const string& str)
{
	string str_no_ws;
	for (char c : str) if (!isspace(c)) str_no_ws += c;
	return str_no_ws;
}

typedef struct pedalcurvestruct {
	//  uint8_t PedalCurveSize
	uint16_t PedalCurveInput[16];
	uint16_t PedalCurveOutput[16];//   65 bytes. * 5
} pedalcurve;


typedef struct eepromdatastruct {
	char VersionString[10];

	uint16_t ADCSteeringInput[10]; // HPF19 compatibility, potential future use
	uint16_t ADCSteeringOutput[10]; // min/max possible, further reaches, mid point. 40 bytes

	uint16_t ADCBrakeRPresInput[2];   // hpf19 compatibility, potential future use.
	uint16_t ADCBrakeRPresOutput[2]; // 8 bytes

	uint16_t ADCBrakeFPresInput[2];   // hpf19 compatibility, potential future use. linear scale Need more if non linear.
	uint16_t ADCBrakeFPresOutput[2]; // 8 bytes // 57 bytes.

	uint16_t ADCBrakeTravelInput[4]; // will always map to 0-100%.. min valid, zero value, 100% val, max val. // 8 bytes.

	uint16_t ADCTorqueReqLInput[4]; // min, zero reading, 100% reading[98%], max. // 8 bytes

	uint16_t ADCTorqueReqRInput[4]; // min, zero reading, 100% reading[98%], max. // 8 bytes

	pedalcurve pedalcurves[5];

	//uint8_t CoolantSize
	uint16_t CoolantInput[20];
	uint16_t CoolantOutput[20];  // 81 bytes.

	uint16_t DrivingModeInput[8]; //  16 bytes

	// config data

	uint8_t MaxTorque;
	uint8_t PedalProfile;
	bool LimpMode;
	bool TorqueVectoring;
	bool Fans;

} eepromdata;



union EEprom {
	uint8_t buffer[4096];
	struct {
		char version[32]; // block 0  32 bytes
		uint8_t active[32]; // block 1 32 bytes
		uint8_t reserved1[32*8]; // blocks 2-9 256 bytes.
		union {
			uint8_t padding1[32*50]; // force the following structure to be aligned to start of a 50 block area.
			eepromdata block1; // block 10-59
		};
		union {
			uint8_t padding2[32*50];
			eepromdata block2; // block 60-109
		};

		uint8_t reserved2[32*14]; // block 110-123  448 bytes
		uint8_t errorlogs[32*4]; // block 124-127  128 bytes
	};
};

typedef vector<tuple <string, uint16_t *, int, int > > localdatatype;

bool setvalue(localdatatype  &data, const string field, const int value)
{
	auto position = find_if(data.begin(), data.end(),
		[=](auto item)
		{
			return get< 0 >(item) == field;
		});

	if (position not_eq data.end()) {
		// found
		cout << "Found" << endl;


		int pos = get<2>(*position);
		//int pos = 0;

		/*
		pos = 0;
		for (; get <1>(*position)[pos] != 0 && pos < get <3>(*position); pos++); // find end of list
		{
		} */

		if (pos >= get <3>(*position))
			//	if (get <3>(*position) > get<1>(*position).size())
				//	get <1>(*position).push_back(value);
		{
			cout << "Field full." << endl;
			return false;
		}
		else
		{
			get <1>(*position)[pos] = value;
			get<2>(*position)++;
			return true;
		}
	}
	else { cout << "Not found" << endl; return false; }
}


bool putdata(localdatatype &datain, eepromdata &dataout) {

	strncpy_s(dataout.VersionString, "MMECUV0.1", 9); // TODO correct field size

	auto position = find_if(datain.begin(), datain.end(),
		[=](auto item)
		{
			return get< 0 >(item) == "coolantinput";
		});

	int pos = get<2>(*position);
	for (; get <1>(*position)[pos] != 0 && pos < get <3>(*position); pos++) // find end of list
	{
		dataout.CoolantInput[pos] = get<1>(*position)[pos];
		get<2>(*position)++;
	}

	if (pos >= get <3>(*position))
	{
		return false;
	}
	else
	{

	}

	// ensure vector isn't too big, or small, regardless of what happened before.
	//copy(get <1>(*position)[0].begin, get <1>(*position).end(), dataout.CoolantInput);


//	for (vector<int>::size_type i = 0; i != v.size(); i++) {
		/* std::cout << v[i]; ... */
	//}

	return false;

}

int main()
{
    cout << "EEprom text to data blob converter.\n";

	localdatatype localdata;

	//setup data types that can be stored.

	EEprom data = { 0 }; // create data, initialise to zero.
	
	eepromdatastruct& block1 = data.block1;
	eepromdatastruct& block2 = data.block1;
	eepromdatastruct* activeblock = &block1;

#define FIELD(a) activeblock->##a, 0, fieldsize(eepromdatastruct, ##a) // pointer to field, and how big field is.

	strncpy_s(data.version, "MMECUV0.1", 9); // TODO correct field size
	data.active[0] = 1; // block 1 32 bytes

	strncpy_s(activeblock->VersionString, "MMECUV0.1", 9); // TODO correct field size

	localdata.push_back(make_tuple("steeringinput", FIELD(ADCSteeringInput)));

	localdata.push_back(make_tuple("steeringoutput", FIELD(ADCSteeringOutput)));
//	localdata.push_back(make_tuple("steeringoutput", activeblock->ADCSteeringOutput,fieldsize(eepromdatastruct, ADCSteeringOutput)));
	localdata.push_back(make_tuple("brakerpresinput", FIELD(ADCBrakeRPresInput)));
	localdata.push_back(make_tuple("brakerpresoutput", FIELD(ADCBrakeRPresOutput)));
	localdata.push_back(make_tuple("brakefpresinput", FIELD(ADCBrakeFPresInput)));
	localdata.push_back(make_tuple("brakefpresoutput", FIELD(ADCBrakeFPresOutput)));
	localdata.push_back(make_tuple("braketravinput", FIELD(ADCBrakeTravelInput)));
	localdata.push_back(make_tuple("torquereqlinput", FIELD(ADCTorqueReqLInput)));
	localdata.push_back(make_tuple("torquereqrinput", FIELD(ADCTorqueReqRInput)));

#define FIELDPED(a) activeblock->pedalcurves[i].##a, 0, fieldsize(pedalcurvestruct, ##a)

	for (int i = 0; i < 5; i++)
	{
		localdata.push_back(make_tuple("pedalcurveinput" + to_string(i+1), FIELDPED(PedalCurveInput)));
		localdata.push_back(make_tuple("pedalcurveoutput" + to_string(i+1), FIELDPED(PedalCurveOutput)));
	}

	localdata.push_back(make_tuple("coolantinput", FIELD(CoolantInput)));
	localdata.push_back(make_tuple("coolantoutput", FIELD(CoolantOutput)));
	localdata.push_back(make_tuple("drivingmodeinput", FIELD(DrivingModeInput)));

	/*

	localdata.push_back(make_tuple("steeringinput", vector<int>(), activeblock->ADCSteeringInput, fieldsize(eepromdatastruct, ADCSteeringInput)));
	localdata.push_back(make_tuple("steeringoutput", vector<int>(), fieldsize(eepromdatastruct, ADCSteeringOutput)));
	localdata.push_back(make_tuple("brakerpresinput", vector<int>(), fieldsize(eepromdatastruct, ADCBrakeRPresInput)));
	localdata.push_back(make_tuple("brakerpresoutput", vector<int>(), fieldsize(eepromdatastruct, ADCBrakeRPresOutput)));
	localdata.push_back(make_tuple("brakelpresinput", vector<int>(), fieldsize(eepromdatastruct, ADCBrakeLPresInput)));
	localdata.push_back(make_tuple("brakelpresoutput", vector<int>(), fieldsize(eepromdatastruct, ADCBrakeLPresOutput)));
	localdata.push_back(make_tuple("braketravinput", vector<int>(), fieldsize(eepromdatastruct, ADCBrakeTravelInput)));
	localdata.push_back(make_tuple("torquereqlinput", vector<int>(), fieldsize(eepromdatastruct, ADCTorqueReqLInput)));
	localdata.push_back(make_tuple("torquereqrinput", vector<int>(), fieldsize(eepromdatastruct, ADCTorqueReqRInput)));
	
	for (int i = 1; i <= 5; i++)
	{
		localdata.push_back(make_tuple("pedalcurveinput" + to_string(i), vector<int>(), fieldsize(pedalcurvestruct, PedalCurveInput)));
		localdata.push_back(make_tuple("pedalcurveoutput" + to_string(i), vector<int>(), fieldsize(pedalcurvestruct, PedalCurveOutput)));
	}
	
	localdata.push_back(make_tuple("coolantinput", vector<int>(), fieldsize(eepromdatastruct, CoolantInput)));
	localdata.push_back(make_tuple("coolantoutput", vector<int>(), fieldsize(eepromdatastruct, CoolantOutput)));
	localdata.push_back(make_tuple("drivingmodeinput", vector<int>(), fieldsize(eepromdatastruct, DrivingModeInput)));

	*/


	/*
	ofstream myfile("example.txt");
	if (myfile.is_open())
	{
		myfile << "This is a line.\n";
		myfile << "This is another line.\n";
		myfile.close();
	}
	else cout << "Unable to open file\n"; */

	ifstream in("eeprom.txt");

	if (in.is_open())
	{
		string str;
		while (getline(in, str)) {
			string fieldstr = "";
			str = remove_ws(str);
			cout << str << endl;
			char cstr[255];
			strcpy_s(cstr, str.c_str());

			char* ptr = cstr;
			char field[80] = "";
			int value;
			unsigned int n;
			int count = 0;
			sscanf_s(ptr, "%31[^=]%n", field, 80, &n);
			fieldstr.assign(field);
		//a	fieldstr = fieldstr.

			cout << "Field = " << fieldstr << "\n";
			ptr += n;
			++ptr;

			while (sscanf_s(ptr, "%31[^,]%n", field, 80, &n) == 1 && sscanf_s(field, "%d", &value) == 1) // crashes if first line blank
			{
				++count;
				printf("value %d = \"%s\"\n", count, field);

				setvalue(localdata, fieldstr, value); // read the value into memory.

				ptr += n; /* advance the pointer by the number of characters read */
				if (*ptr != ',')
				{
					break; /* didn't find an expected delimiter, done? */
				}
				++ptr; /* skip the delimiter */
			}

			cout << "\n";
		}


		memcpy_s(&block2, 1600, &block1, 1600);

		ofstream output_file("EEPROMblock.dat", ios::binary);

		//output_file << block1;
		output_file.write((char*)&block1, sizeof(block1)); // TODO set correct size programmatically.		
		//output_file.write((char*)&data, 4096); // TODO set correct size programmatically.			
		output_file.close();


	}
	else cout << "Can't open file.\n";

	cout << "Done." << endl;
}
