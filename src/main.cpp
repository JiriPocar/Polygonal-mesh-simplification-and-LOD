/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file main.cpp
 * @brief Main entry point for the Vulkan application.
 *
 * This file contains the main function that initializes the Vulkan application.
 */

#include <vulkan/vulkan.hpp>
#include <iostream>

int main() {
	try
	{
		std::cout << "Starting Vulkan application..." << std::endl;
	}
	catch (vk::Error& e)
	{
		std::cout << "Caught a Vulkan error" << e.what() << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Caught a standard exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Caught an unknown exception" << std::endl;
	}
}

/* End of the main.cpp file */