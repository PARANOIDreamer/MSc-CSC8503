#pragma once

class Feature
{
public:
	Feature() {
		catchGoat = 0;
		hookGoat = 0;
		cubeMove = 0;
		time = 0;
		onTime = 0;
		onMove = 0;
		playerScore = 0;
		for (int i = 0; i < 4; i++)
			lockDoor[i] = 0;
		score = 0;
		for (int i = 0; i < 4; i++)
			bonus[i] = 0;
	}
	~Feature();

	int GetCatchgoat() {
		return catchGoat;
	}
	void SetCatchgoat(int num) {
		catchGoat = num;
	}
	int GetHookgoat() {
		return hookGoat;
	}
	void SetHookgoat(int num) {
		hookGoat = num;
	}
	int GetCubemove() {
		return cubeMove;
	}
	void SetCubemove(int num) {
		cubeMove = num;
	}
	float GetTime() {
		return time;
	}
	void SetTime(float dt) {
		time += dt;
	}
	int GetOntime() {
		return onTime;
	}
	void SetOntime(int num) {
		onTime = num;
	}
	int GetOnmove() {
		return onMove;
	}
	void SetOnmove(int num) {
		onMove = num;
	}
	int GetPlayerscore() {
		return playerScore;
	}
	void SetPlayerscore(int num) {
		playerScore = num;
	}
	int GetLockdoor(int i) {
		return lockDoor[i];
	}
	void SetLockdoor(int i, int num) {
		lockDoor[i] = num;
	}
	int GetScore() {
		return score;
	}
	void AddScore(int num) {
		score += num;
	}
	void Again() {
		catchGoat = 0;
		hookGoat = 0;
		cubeMove = 0;
		onTime = 1;
		onMove = 0;
		playerScore = 0;
		for (int i = 0; i < 4; i++)
			lockDoor[i] = 0;
		score = 0;
	}
	bool bonus[4];

protected:
	int catchGoat;
	int hookGoat;
	int cubeMove;
	float time;
	int onTime;
	int onMove;
	int playerScore;
	int lockDoor[4];
	int score;
};
