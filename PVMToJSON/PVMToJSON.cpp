// (c) Class was made using outine provided by Stefan Roettger were made by Thomas Clarke under GPL 2+


//
// PVMToJSON.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include "ReadPVMvolume.h"
#include "PVMJSONOutput.h"

int main(int argc, char *argv[])
{
    std::cout << "Starting Processing!\n";

	unsigned char *volume;

	unsigned int width, height, depth,
		components;

	float scalex, scaley, scalez;

	if (argc != 2 && argc != 3)
	{
		std::cout << argc << std::endl;
		printf("usage: %s <input.pvm> [<output.raw>]\n", argv[0]);
		exit(1);
	}

	ReadPVMvolume reader = ReadPVMvolume();
	if ((volume = reader.readPVMvolume(argv[1], &width, &height, &depth, &components, &scalex, &scaley, &scalez)) == NULL) exit(1);
	if (volume == NULL) exit(1);

	// if the users have a output dir then it can be added
	if (argc > 2)
	{
		PVMJSONOutput json = PVMJSONOutput(width, height, depth, components, volume, scalex, scaley, scalez);
		std::string output = json.ToJSON(true);

		std::ofstream myfile;
		myfile.open(argv[2], std::ios::out);
		myfile << output;
		myfile.close();

		//std::cout << scalex << std::endl;
		//std::cout << scaley << std::endl;
		//std::cout << scalez << std::endl;

		//printf("writing RAW file with size=%d\n", width*height*depth*components);

		//reader.writeRAWfile(argv[2], volume,
			//width*height*depth*components,
			//true);
	}

	free(volume);

	return(0);
}
