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

	unsigned char * volume;

public:
	const std::string NEWLINE = "\n";

	PVMJSONOutput(unsigned int width, unsigned int height, unsigned int breath, unsigned int component, unsigned char* volume) {
		this->breath = breath;
		this->height = height;
		this->width = width;
		this->component = component;
		this->volume = volume;
	}

	std::string ToJSON(){
		// The string continating the json
		std::string output = "{" + NEWLINE;
		
		// write the volume details
		output.append("\"width\" : ");
		output.append(std::to_string(width));
		output.append(",");
		output.append(NEWLINE);

		output.append("\"height\" : ");
		output.append(std::to_string(height));
		output.append(",");
		output.append(NEWLINE);

		output.append("\"breath\" : ");
		output.append(std::to_string(breath));
		output.append(",");
		output.append(NEWLINE);

		output.append("\"components\" : ");
		output.append(std::to_string(component));
		output.append(",");
		output.append(NEWLINE);

		// add the volume data
		output.append("\"data\": [");
		int endofLoop = width * height * breath * component;
		for (int index = 0; index < endofLoop; index++) {
			output.append(std::to_string(volume[index]));
			if (index < endofLoop - 1) {
				output.append(",");
			}
			else {
				output.append("],");
			}
		}
		output.append(NEWLINE);
		output.append("}");

		return(output);
	}
	
};

