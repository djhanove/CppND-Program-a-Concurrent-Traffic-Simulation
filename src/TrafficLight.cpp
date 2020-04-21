#include <iostream>
#include <random>
#include "TrafficLight.h"

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(_mutex);
    _condition.wait(lck, [this] {return !_queue.empty(); }); // pass lock to condition variable
    T msg = std::move(_queue.back()); 
    //_queue.pop_back(); // delete oldest element in queue (FIFO)
    _queue.clear(); // per suggestions from https://knowledge.udacity.com/questions/98313 since vehicles are passing through red lights
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex); // lock it down so I can add a message to the queue
    _queue.push_back(std::move(msg)); // move traffic light message into queue
    _condition.notify_one(); // notify the conditional lock that I'm done modifying
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        TrafficLightPhase currPhase = _queue.receive();
        if (currPhase == TrafficLightPhase::green)
        {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
    srand(time(NULL));
    int currentCycleTime = 0; // millisecond intervals
    int cycleDuration = rand() % 6000 + 4000; // generate random number from 4-6 using C STL
    
    while(true) {
        if(currentCycleTime  >= cycleDuration) {
            currentCycleTime = 0; // reset counter
            cycleDuration = rand() % 6000 + 4000; // get new cycle duration for light cycle
            changePhase(); // update _currentPhase 
            auto newPhase = _currentPhase;
            _queue.send(std::move(newPhase));
        }
        else
        {
            currentCycleTime++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void TrafficLight::changePhase() 
{
    if(_currentPhase == TrafficLightPhase::red)
        _currentPhase = TrafficLightPhase::green;
    else
        _currentPhase = TrafficLightPhase::red;    
}
