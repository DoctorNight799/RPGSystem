#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Entity {
public:
	int hp;
	int dmg;
	int initiative;
	string name;

	Entity(string _name, int _hp, int _dmg) : name(_name), hp(_hp), dmg(_dmg) {}

	void attack(Entity& c);

	void rollInitiative() {
		initiative = rand() % 6;
	}
};

void Entity::attack(Entity& c) {
	if (c.hp > 0)
	{
		c.hp -= dmg;
		cout << c.name << " took " << dmg << " dmg\n";
	}
	else
		cout << c.name << " is dead\n";
}

int main() {
	srand(time({}));
	Entity c1("1", 20, 5);
	Entity c2("2", 20, 10);

	if (c1.initiative > c2.initiative)
		c1.attack(c2);
	else
		c2.attack(c1);
}