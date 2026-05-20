#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Person {
public:
	int hp;
	int dmg;

	Person(int _hp, int _dmg) : hp(_hp), dmg(_dmg) {}

	void attack(Person& c);
};

void Person::attack(Person& c) {
	if (c.hp > 0)
		c.hp -= dmg;
	else
		cout << "Target is dead\n";
	cout << "ATTACK!\n";
}

int main() {
	Person c1(20, 5);
	Person c2(20, 10);

	c2.attack(c1);
	c2.attack(c1);
	c2.attack(c1);
	c2.attack(c1);
}