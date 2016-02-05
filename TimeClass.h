#ifndef TIMECLASSH
#define TIMECLASSH

const int MAX_TIMER_COUNT = 10;

class TimeClass{
private:
	struct TimerType{
		int timeLeft;
		int ID;
	};
	
	unsigned long int frame;
	TimerType timer[MAX_TIMER_COUNT];
	int timerIDCount, timerCount;
public:
	TimeClass();
	TimeClass(const TimeClass& other);
	~TimeClass();

	unsigned long int GetFrameCount();
	void SetFrameCount(int input);
	void FrameIncrement();
	void AddTimer(int& timerID, int timeLimit);
	void SetTimer(int timerID, int timeLimit);
	void DeleteTimer(int& timerID);
	bool IsTimerRunning(int timerID);
	long int TimeLeft(int timerID);
};

#endif