// Reflections.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "DelusiveEngine.h"
#include <iostream>

int main()
{
	DelusiveEngine::DelusiveContext context;
	context.editorMode = false;
	return DelusiveEngine::Run(context);
}