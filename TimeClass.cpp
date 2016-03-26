#include "TimeClass.h"

TimeClass::TimeClass(){
	frame = 0;
	for (int i = 0; i < MAX_TIMER_COUNT; i++){
		timer[i].timeLeft = -1;
	}
	timerIDCount = 1;
	timerCount = 0;
	clockIDCount = 1;
	clockCount = 0;
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

	for (int i = 0; i < clockCount; i++){
		clock[i].time++;
	}
}

bool TimeClass::AddTimer(int& timerID, int timeLimit){
	if (timerCount < MAX_TIMER_COUNT){
		timer[timerCount].timeLeft = timeLimit;
		timer[timerCount].ID = timerIDCount;
		timerID = timerIDCount;
		timerCount++;
		timerIDCount++;

		return true;
	}
	else{
		return false;
	}
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

bool TimeClass::AddClock(int& clockID){
	if (clockCount < MAX_CLOCK_COUNT){
		clock[clockCount].ID = clockIDCount;
		clock[clockCount].time = 0;
		clockID = clockIDCount;
		clockCount++;
		clockIDCount++;

		return true;
	}
	else{
		return false;
	}
}

void TimeClass::SetClock(int clockID, int time){
	for (int i = 0; i < clockCount; i++){
		if (clock[i].ID == clockID){
			clock[i].time = time;
			break;
		}
	}
}

void TimeClass::DeleteClock(int& clockID){
	for (int i = 0; i < clockCount; i++){
		if (clock[i].ID == clockID){
			for (int j = i; j < clockCount - 1; j++){
				clock[j].ID = clock[j + 1].ID;
				clock[j].time = clock[j + 1].time;
			}
			clockID = 0;
			clock[clockCount - 1].ID = -1;
			clockCount--;
			break;
		}
	}
}

int TimeClass::CurrentClockTime(int clockID){
	for (int i = 0; i < clockCount; i++){
		if (clock[i].ID == clockID){
			return clock[i].time;
		}
	}
}