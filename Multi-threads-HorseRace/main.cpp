/*
Rachael Savage
CSC275:C++2
Professor Tony Hinton
7/6/23
*/


#include <iostream>
#include <thread>// for working w/multi-threads/support/manage concurrent threads of execution
#include <fstream>//work w/files
#include <string>
#include <ctime>//real time
#include <vector>//container to hold collection of dynamic data/element
//for implementing mutual exclusion
//allow sharing resources among multiple threads, ensuring that only one thread can access the resource at a time
#include <mutex> //(short for mutual exclusion)
#include <condition_variable>
#include <algorithm> //for sorting the horses

//create global vars 
//const means that the variable's value cannot be changed once it is initialized
const int NUM_HORSES = 5;
const int TRACK_LENGTH = 20;

//create a tructure to group vars
struct Horse {
    //member data
    int id;//to store horse id
    int distance;
};

//create a vector to hold objects horse
// horses is the container to store multi horse objects
//initialize the global var NUM_HORSES for size
std::vector<Horse> horses(NUM_HORSES);

//declare a vector to store threads. each  thread associate to a horse 
std::vector<std::thread> horseThreads(NUM_HORSES);

//declare a mutex object to allow multiple threads safely access shared resource -horse
std::mutex mutex;

//enables threads to wait for specific conditions to be met before proceeding. use with mutex
std::condition_variable cv;
//create a vars to track the race
bool raceFinished = false;
bool restartRace = false;
int winningHorse = -1; // Global var to keep track and store the winning horse result

//funcs prototype
void HorseRace(int horseID);
void StartRace();
void StopRace();
void RestartRace();
void SaveRaceResultsToAFile(const std::string& fileName);



//program start output according to the order inside main()
int main() {
    std::cout << "\n\n" << std::endl;
    std::cout << "\t\tHORSE RACE PROGRAM" << std::endl;
    std::cout << "************************************************************************" << std::endl;
    std::cout << "\nWelcome to the Horse Race!" << std::endl;
    std::cout << "\nWe have 5 Horses that race on a track length 20" << std::endl;
    std::cout << "The first horse to reach the finish line wins." << std::endl;
    std::cout << "\nWhen you're ready to start the race, press enter\n" << std::endl;
    system("pause");
    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //std::cin.ignore();//wait for user to press the enter key and ignore any input

    //create a seed for the random # generator with the current time (must include ctime lib)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    StartRace();//call the func to start the race
    std::cout << "The race has started!\n" << std::endl;


    std::cout << " Press ENTER to stop the race" << std::endl;
    system("pause");
    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
   // std::cin.ignore();//wait for user to press the enter key


    StopRace();//call the func to stop the race

    //display the winning horse


    std::cout << "Press ENTER to Restart the race" << std::endl;
    system("pause");
    //std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    //std::cin.ignore();
    //call the funcs
    RestartRace();
    StartRace();


    std::cout << " The Race has restarted!!!\n" << std::endl;
    
    
    std::cout << "Press ENTER to stop the race" << std::endl;
    std::cin.ignore();

    StopRace();//call the func

    //create a string fileName concatenate race_result w/ current time to generate a time stamp for file name
    std::string fileName = "race_results_" + std::to_string(std::time(nullptr)) + ".txt";
    SaveRaceResultsToAFile(fileName);//call the save func


    std::cout << "\n\n" << std::endl;
    return 0;
}//---------------------END MAIN------------------------------------------------------------------------------------------



//bring funcs here and add in bodies
void HorseRace(int horseID) {
    //pause the thread for 3s
    std::this_thread::sleep_for(std::chrono::seconds(3));

    //
    while (true) {
        //use a unique lock from mutex to make sure that only one thread can hold the lock at a time
        std::unique_lock<std::mutex> lock(mutex);
        //make the thread to wait until the conditionvar cv is notified and the func return true
        cv.wait(lock, [] {return raceFinished || restartRace; });

        //if the restartRace is true, the code block inside this if will be executed
        if (restartRace) {
            horses[horseID].distance = 0;
            restartRace = false;
        }
       
        if (horses[horseID].distance >= TRACK_LENGTH) {
            {
                std::lock_guard<std::mutex> winnerLock(mutex);
                if (winningHorse == -1 || horses[horseID].distance > horses[winningHorse].distance) {
                    winningHorse = horseID;
                }
            } // winnerLock releases the mutex here
            raceFinished = true;
            cv.notify_all();
            lock.unlock();
            
            return;
        }
       
        // incre the distance cover by horse ID with a random value 3+1 to simulate a horse race
  
        horses[horseID].distance += std::rand() % 3 + 1;

        cv.notify_all();
        lock.unlock(); // Release the lock after notifying other threads
        //lock.unlock();//release the mutex lock from unique_lock
       // cv.notify_all(); //notify all threads
        return;
        
       // std::this_thread::sleep_for(std::chrono::milliseconds(500));//sleep 0.5s


    }
}


void StartRace() {
    //creat and initialize horse objects that corresponding to threads
    for (int i = 0; i < NUM_HORSES; ++i) {
        horses[i].id = i + 1; //count start from 1
        horses[i].distance = 0;//distance start from 0
        //create a new thread and pass in the Horserace func for the thread entry
        horseThreads[i] = std::thread(HorseRace, i);//i is used to identify the horse within the HorseRace func

        //show thread created and show thread ID for learning purpose
        std::cout << "Horse #" << horses[i].id << "thread created. Thread ID: " << horseThreads[i].get_id() << std::endl; 
    }

}



void StopRace() {
    //flip the global var to show that the race is finished as this global var was shared among multi-threads
    raceFinished = true;
    cv.notify_all();//notify all waiting threads that the race is finished.Now activate all the waiting threads
    //loop through elements that stored in the vector horsethreads
    for (auto& thread : horseThreads) {
        if (thread.joinable()) {//check the thread whether it's safe to join
            thread.join();//calling thread to wait for the specified thread to finish its execution be4 the StopRace func return
        }
    }

}


void RestartRace() {
    raceFinished = false;//race is not finished/reset the race
    restartRace = true; //request to restart the race
    cv.notify_all();//notify all the threads that the race has restarted and waiting thread to be awaken to check for update
    //start the loop to go through elements inside the vector horses
    for (auto& horse : horses) {
        horse.distance = 0;//reset the distance
    }

}

//save the result into a file
void SaveRaceResultsToAFile(const std::string& fileName) {
    //create a file object allow writing data to a file
    //if the file name hasn't existed, it'll created one. If the filename existed, it'll over wrriten
    std::ofstream outputFile(fileName);

    //check if the file open sucessfully. if successfully opened, execute the code block
    if (outputFile.is_open()) {
        outputFile << "\n\t---------- - HORSE RACE RESULTS----------------\n";//header
        

        //sort horses based on the distance it covered
        std::sort(horses.begin(), horses.end(), [](const Horse& h1, const Horse& h2){

            return h1.distance > h2.distance;
           
         });

        //display race result and save the data into a file
        
        for (const auto& horse : horses) {//go through elements inside the vector horses
            outputFile << "Horse #" << horse.id << " finished at distance: " << horse.distance <<"\n";

        }
        if (winningHorse != -1) {
            outputFile << "\nWinning horse: Horse #" << horses[0].id << "\n";
        }
        else {
            outputFile << "\nNo horse finished the race.\n";
        }
        outputFile << "\nEND OF HORSE RACE\n";
        outputFile.close();

        std::cout << "Race results saved to file: " << fileName << std::endl;

    }
    else {//display error message if the file unable to open
        std::cerr << "Unable to open file: " << fileName << std::endl;
    }
}