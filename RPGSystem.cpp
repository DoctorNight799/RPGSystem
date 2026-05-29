#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

const int teamNum = 10;

class Entity {
public:
	int hp;
	int dmg;
	int initiative;
	int teamId; // 0 - player + allies, 1 - enemies and other
	string name;
	bool isPlayer = false;

	Entity(string _name, int _hp, int _dmg, int _teamId, bool player) : name(_name), hp(_hp), dmg(_dmg), teamId(_teamId), isPlayer(player) {}

	Entity(string _name, int _hp, int _dmg, int _teamId) : name(_name), hp(_hp), dmg(_dmg), teamId(_teamId) {}

	void takeDamage(int _dmg) {
		if (hp > 0) {
			hp -= _dmg;
		}
	}

	void setInitiative() {
		initiative = rand() % 6 + 1;
	}
};


bool initiativeComp(const Entity& a, const Entity& b) {
	if (a.initiative == b.initiative) {
		return &a > &b;
	}
	return a.initiative > b.initiative;
}

class  BattleManager {
public:
	void addEntity(Entity e) {
		entities.push_back(e);
	}

	void rollInitiatve() {
		for (auto& e : entities)
			e.setInitiative();
		sort(entities.begin(), entities.end(), initiativeComp);
	}

	void playerTurn(Entity& player) {
		int num;
		bool action{ false };
		while (action == false) {
			cout << "1.Атака\n2.Способность\n3.Магия\n4.Предметы\n5.Побег\nВыберите действие: ";
			cin >> num;
			switch (num) {
				case 1:
				{
					cout << "Список целей:\n";
					int index = 1;
					vector<Entity*> targets;
					for (auto& e : entities) {
						if (e.teamId != player.teamId && e.hp > 0) {
							cout << index++ << ". " << e.name << " (HP: " << e.hp << ")\n";
							targets.push_back(&e);
						}
					}
					if (targets.empty()) return;

					cout << "Выберите цель для атаки: ";
					int choice;
					cin >> choice;
					cin.ignore();

					if (choice >= 1 && choice <= targets.size()) {
						targets[choice - 1]->takeDamage(player.dmg);
						cout << "Player dealt " << player.dmg << " damage to " << targets[choice - 1]->name << '\n';
					}
					cout << endl;
					action = true;
					break;
				}
				default: {
					cout << "Incorrect action\n\n";
				}
			}
		}
		
	}

	void aiTurn(Entity& current) {
		for (auto& e : entities) {
			if (e.teamId != current.teamId && e.hp > 0) {
				e.takeDamage(current.dmg);
				cout << "Enemy dealt " << current.dmg << " damage to " << e.name << '\n';
				break;
			}
		}
		cout << endl;
	}

	bool isBattleOver() {
		bool teamsAlive[teamNum] = { false };
		int aliveTeams = 0;

		for (auto& e : entities) {
			if (e.hp > 0 && !teamsAlive[e.teamId]) {
				teamsAlive[e.teamId] = true;
				aliveTeams++;
				if (aliveTeams > 1) return false;
			}
		}
		return true;
	}

	void startBattle() {
		rollInitiatve();
		while (!isBattleOver()) {
			for (auto& e : entities) {
				if (e.hp <= 0) continue;

				if (e.isPlayer)
					playerTurn(e);
				else
					aiTurn(e);

				if (isBattleOver()) break;
			}
		}
		for (auto& e : entities) {
			if (e.hp > 0) {
				cout << "Бой окончен. Победитель: " << e.name << '\n';
				break;
			}
		}
	}
private:
	vector<Entity> entities;
};

int main() {
	setlocale(LC_ALL, "Rus");

	srand(time({}));

	Entity c1("Player", 20, 10, 0, true);
	Entity c2("Enemy", 20, 10, 1);

	BattleManager manager;

	manager.addEntity(c1);
	manager.addEntity(c2);

	manager.startBattle();
}