#pragma once
class Feature
{
public:
	Feature() {
		catchGoat = 0;
		hookGoat = 0;
		cubeMove = 0;
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

protected:
	int catchGoat;
	int hookGoat;
	int cubeMove;
};

