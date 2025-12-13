#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
	// get the inputs
	

	// error if user puts the incorrect number of args
	if (argc != 3) 
	{
		std::cerr << "Usage: " << argv[0] << " <input filename 1> <input filename 2>" << std::endl;
		return 1;	
	}

	std::string filename1 = argv[1];
	std::string filename2 = argv[2];

	std::ifstream inFile1(filename1);
	std::ifstream inFile2(filename2);
	
	
	// error if program can't read one of the files.
	if (!inFile1)
	{
		std::cerr << "Error: Could not open file '" << filename1 << "'" << std::endl;
		return 1;
	}
	
	if (!inFile2)
	{
		std::cerr << "Error: Could not open file '" << filename2 << "'" << std::endl;
		return 1;
	}

	return 0;
}
