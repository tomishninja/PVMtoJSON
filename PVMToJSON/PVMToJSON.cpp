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
    std::cout << "Hello World!\n";

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

	

	if (argc > 2)
	{
		PVMJSONOutput json = PVMJSONOutput(width, height, depth, components, volume);
		std::string output = json.ToJSON();

		std::ofstream myfile;
		myfile.open(argv[2], std::ios::out);
		myfile << output;
		myfile.close();

		//printf("writing RAW file with size=%d\n", width*height*depth*components);

		//reader.writeRAWfile(argv[2], volume,
			//width*height*depth*components,
			//true);
	}

	//std::cout << "width" << width << std::endl;
	//std::cout << "height" << height << std::endl;
	//std::cout << "depth" << depth << std::endl;
	//std::cout << "components" << components << std::endl;

	//std::string vol = std::string(volume);
	//for (int index = 0; index < 8388608; index++)
		//std::cout << +volume[index] << ", ";

	free(volume);

	return(0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
