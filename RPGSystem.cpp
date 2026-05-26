#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

class Entity {
public:
	int hp;
	int dmg;
	int initiative;
	string name;

	Entity(string _name, int _hp, int _dmg) : name(_name), hp(_hp), dmg(_dmg) {}

	void attack(Entity& c);

	void setInitiative() {
		initiative = rand() % 6 + 1;
	}
};


bool initiativeComp(const Entity& a, const Entity& b) {
	if (a.initiative == b.initiative) {
		int rerollA = rand() % 6 + 1;
		int rerollB = rand() % 6 + 1;
		return rerollA > rerollB;
	}
	return a.initiative > b.initiative;
}

class  BattleManager {
public:
	void rollInitiatve() {
		for (auto& e : entities)
			e.setInitiative();
		sort(entities.begin(), entities.end(), initiativeComp);
	}

	void addEntity(Entity e) {
		entities.push_back(e);
	}
private:
	vector<Entity> entities;
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
	BattleManager manager;
	manager.addEntity(c1);
	manager.addEntity(c2);
	manager.rollInitiatve();
	
}