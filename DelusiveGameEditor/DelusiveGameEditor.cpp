// DelusiveGameEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <Delusive/DelusiveEngine.h>
#include <SDL3/SDL.h>
#include <iostream>

int main()
{
	std::cout << "Starting Delusive Game Editor..." << std::endl;
    DelusiveEngine::DelusiveContext context;
    context.editorMode = true;
	return DelusiveEngine::Run(context);
}
