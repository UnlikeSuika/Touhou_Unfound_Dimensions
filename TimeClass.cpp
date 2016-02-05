#include "TimeClass.h"

TimeClass::TimeClass(){
	frame = 0;
	for (int i = 0; i < MAX_TIMER_COUNT; i++){
		timer[i].timeLeft = -1;
	}
	timerIDCount = 1;
	timerCount = 0;
}

TimeClass::TimeClass(const TimeClass& other){}

TimeClass::~TimeClass(){}

unsigned long int TimeClass::GetFrameCount(){
	return frame;
}

void TimeClass::SetFrameCount(int input){
	frame = (unsigned long int)input;
}

void TimeClass::FrameIncrement(){
	frame++;
	
	for (int i = 0; i < timerCount; i++){
		if (timer[i].timeLeft >= 0){
			timer[i].timeLeft--;
		}
	}
}

void TimeClass::AddTimer(int& timerID, int timeLimit){
	timer[timerCount].timeLeft = timeLimit;
	timer[timerCount].ID = timerIDCount;
	timerID = timerIDCount;

	timerCount++;
	timerIDCount++;
}

void TimeClass::SetTimer(int timerID, int timeLimit){
	for (int i = 0; i < timerCount; i++){
		if (timer[i].ID == timerID){
			timer[i].timeLeft = timeLimit;
			break;
		}
	}
}

void TimeClass::DeleteTimer(int& timerID){
	for (int i = 0; i < timerCount; i++){
		if (timer[i].ID == timerID){
			for (int j = i; j < timerCount - 1; j++){
				timer[j].ID = timer[j + 1].ID;
				timer[j].timeLeft = timer[j + 1].timeLeft;
			}
			timerID = 0;
			timer[timerCount-1].ID = -1;
			timerCount--;
			break;
		}
	}
}

bool TimeClass::IsTimerRunning(int timerID){
	for (int i = 0; i < timerCount; i++){
		if (timer[i].ID == timerID && timer[i].timeLeft >= 0){
			return true;
		}
	}
	return false;
}

long int TimeClass::TimeLeft(int timerID){
	for (int i = 0; i < timerCount; i++){
		if (timer[i].ID == timerID){
			return timer[i].timeLeft;
		}
	}
	return -1;
}