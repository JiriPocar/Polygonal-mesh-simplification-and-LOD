/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file main.cpp
 * @brief Main entry point for the Simplificator application.
 *
 * This file contains the main function that initializes the Simplificator application.
 * 
 * ==========================================
 * Features:
 * - Split-screen renderer
 * - Wireframe and solid rendering modes
 * - UI toggle
 * - Model loading, rendering and simplification
 * - Simplification result statistics display via UI
 * ==========================================
 */

#include "SimplificatorApp.hpp"
#include <iostream>

void help()
{
	std::cout << "===============================================================" << std::endl;
	std::cout << "Welcome to the Simplificator app!" << std::endl;
	std::cout << "@author Jiri Pocarovsky" << std::endl;
	std::cout << "===============================================================" << std::endl;
	std::cout << "Controls:" << std::endl;
	std::cout << "  M - Unlock camera" << std::endl;
	std::cout << "  N - Lock camera" << std::endl;
	std::cout << "      Movement: WASD" << std::endl;
	std::cout << "  U - Toggle UI" << std::endl;
	std::cout << "	Up arrow - Increase camera speed" << std::endl;
	std::cout << "	Down arrow - Decrease camera speed" << std::endl;
	std::cout << "  ESC - Exit application" << std::endl;
	std::cout << "===============================================================" << std::endl;
}

int main()
{
	help();

	try {
		SimplificatorApp app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Application error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/* End of the main.cpp file */