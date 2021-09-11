#pragma once

#include <iostream>
#include<string>

class PVMJSONOutput
{
private:
	unsigned int width;
	unsigned int height;
	unsigned int breath;
	unsigned int component;
	unsigned int scalex;
	unsigned int scaley;
	unsigned int scalez;

	unsigned char * volume;

public:
	const std::string NEWLINE = "";

	PVMJSONOutput(unsigned int width, unsigned int height, unsigned int breath, unsigned int component, unsigned char* volume, 
		unsigned int scalex, unsigned int scaley, unsigned int scalez) {
		this->breath = breath;
		this->height = height;
		this->width = width;
		this->component = component;
		this->volume = volume;
		this->scalex = scalex;
		this->scaley = scaley;
		this->scalez = scalez;
	}

	std::string ToJSON(bool includeScale){
		// The string continating the json
		std::string output = "{";
		output.append(NEWLINE);
		
		// write the volume details
		output.append("\"width\":");
		output.append(std::to_string(width));
		output.append(",");
		output.append(NEWLINE);

		output.append("\"height\":");
		output.append(std::to_string(height));
		output.append(",");
		output.append(NEWLINE);

		output.append("\"breath\":");
		output.append(std::to_string(breath));
		output.append(",");
		output.append(NEWLINE);

		/*
		output.append("\"components\":");
		output.append(std::to_string(component));
		output.append(",");
		output.append(NEWLINE);
		*/

		if (includeScale) {
			output.append("\"scalex\":");
			output.append(std::to_string(scalex));
			output.append(",");
			output.append(NEWLINE);

			output.append("\"scaley\":");
			output.append(std::to_string(scaley));
			output.append(",");
			output.append(NEWLINE);

			output.append("\"scaley\":");
			output.append(std::to_string(scaley));
			output.append(",");
			output.append(NEWLINE);
		}

		for (int index = 0; index < 100; index++) {
			std::cout << std::to_string(volume[index]) << std::endl;
		}

		// add the volume data
		output.append("\"data\": [");
		int endofLoop = width * height * breath * component;
		for (int index = 0; index < endofLoop; index += component) {

			// add the correct amount of components to the loop
			switch (component) {
			case 1 : 
				output.append(std::to_string(volume[index]));
				break;
			case 2:
				output.append(std::to_string(((unsigned)volume[index] << 8) + (unsigned)volume[index + 1]));
				break;
			default:
				std::cout << "Not Implmented" << std::endl;
				break;
			}

			if (index < endofLoop - component) {
				output.append(",");
			}
			else {
				output.append("]");
			}
		}
		output.append(NEWLINE);
		output.append("}");
		output.append("\n");

		return(output);
	}
	
};

