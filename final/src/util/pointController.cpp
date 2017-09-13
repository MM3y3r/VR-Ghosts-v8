// my second program in C++
#include <iostream>
#include <string> 

using namespace std;

int playerScore = 0;

void addToScore(int added){
	std::string myValue = std::to_string(added);
	std::string string = "Added points: ";
	std::string message = string + myValue + "\n";
	//strcpy(str, myString);
	std::cout << message; 
	playerScore = playerScore + added;
	std::string myValue2 = std::to_string(playerScore);
	std::string string2 = "PlayerScore: ";
	std::string message2 = string2 + myValue2 + "\n";
	std::cout << message2; 
};

void removeFromScore(int removed){
	std::string myValue = std::to_string(removed);
	std::string string = "Removed points: ";
	std::string message = string + myValue + "\n";
	//strcpy(str, myString);
	std::cout << message; 
	playerScore = playerScore - removed;
	std::string myValue2 = std::to_string(playerScore);
	std::string string2 = "PlayerScore: ";
	std::string message2 = string2 + myValue2 + "\n";
	std::cout << message2; 
};