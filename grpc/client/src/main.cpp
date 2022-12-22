#include <iostream>

#include <iostream>

#include "client.h"


int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "[-] wrong args count" << std::endl;
		exit(1);
	}
	std::string database = argv[1];
	std::string command = argv[2];
	Client cli(database, "127.0.0.1", "50051");
	
	auto res = cli.Execute(command);
	for (auto &[key, vals] : res.items()) {
			if (key == "_error_") {
					std::cout << vals << std::endl;					
					break;
			} else {
					for (auto &val : vals.items())
							std::cout << val.value() << " ";
					std::cout << std::endl;
			}
			
	}

    return 0;
}
