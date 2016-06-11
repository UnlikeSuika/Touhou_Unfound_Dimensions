#ifndef _TIME_CLASS_H_
#define _TIME_CLASS_H_

const int MAX_TIMER_COUNT = 30;
const int MAX_CLOCK_COUNT = 30;

class TimeClass{
private:
	struct TimerType{
		int timeLeft;
		int ID;
	};
	struct ClockType{
		int time;
		int ID;
	};
	
	unsigned long int frame;
	TimerType timer[MAX_TIMER_COUNT];
	ClockType clock[MAX_CLOCK_COUNT];
	int timerIDCount, timerCount;
	int clockIDCount, clockCount;
public:
	TimeClass();
	TimeClass(const TimeClass& other);
	~TimeClass();

	unsigned long int GetFrameCount();
	void SetFrameCount(int input);
	void FrameIncrement();
	bool AddTimer(int& timerID, int timeLimit);
	void SetTimer(int timerID, int timeLimit);
	void DeleteTimer(int& timerID);
	bool IsTimerRunning(int timerID);
	long int TimeLeft(int timerID);
	bool AddClock(int& clockID);
	void SetClock(int clockID, int time);
	void DeleteClock(int& clockID);
	int CurrentClockTime(int clockID);
};

#endif