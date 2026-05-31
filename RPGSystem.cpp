#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

const int teamNum = 10;

enum SpellType {
	HEAL,
	DAMAGE,
	EFFECT
};

class Entity;

class Spell {
public:
	string name;
	int value;
	int manaCost;
	SpellType type;
	Spell(string _name, int _manaCost, int _value, SpellType _type) : name(_name), manaCost(_manaCost), value(_value), type(_type) {}
	void cast(Entity& caster, Entity& target);
};

class Entity {
public:
	int hp;
	int maxHp;
	int mana;
	int maxMana;
	int dmg;
	int initiative;
	int teamId; // 0 - player + allies, 1 - enemies and other
	string name;
	bool isPlayer = false;
	vector<Spell> spells;

	// Конструктор игрока
	Entity(string _name, int _hp, int _mana, int _dmg, int _teamId, bool player) : name(_name), hp(_hp), mana(_mana), dmg(_dmg), teamId(_teamId), isPlayer(player) { maxHp = _hp; maxMana = _mana; }

	// AI конструктор (maxHp и maxMana в отдельных конструкторах потом настроить.)
	Entity(string _name, int _hp, int _dmg, int _teamId) : name(_name), hp(_hp), dmg(_dmg), teamId(_teamId) { maxHp = _hp; }

	void takeDamage(int _dmg) {
		if (hp > 0) {
			hp -= _dmg;
		}
	}

	void setInitiative() {
		initiative = rand() % 6 + 1;
	}
};

void Spell::cast(Entity& caster, Entity& target) {
	switch (type) {
	case HEAL:
		if (caster.mana >= manaCost && target.hp < target.maxHp) {
			caster.mana -= manaCost;
			target.hp += value;
			if (target.hp > target.maxHp) target.hp = target.maxHp;
			cout << caster.name << " healed " << target.name << " on " << value << " hp.\n";
		}
		break;
	case DAMAGE:
		if (caster.mana >= manaCost && target.hp > 0) {
			caster.mana -= manaCost;
			target.takeDamage(value);
			cout << caster.name << " damaged " << target.name << " on " << value << " hp.\n";
		}
	}
}

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
			cout << "1. Атака\n2. Способность\n3. Магия\n4. Предметы\n5. Побег\nВыберите действие: ";
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

					if (choice >= 1 && choice <= targets.size()) {
						targets[choice - 1]->takeDamage(player.dmg);
						cout << "Player dealt " << player.dmg << " damage to " << targets[choice - 1]->name << '\n';
					}
					cout << endl;
					action = true;
					break;
				}
				case 3:
				{
					if (player.spells.empty()) {
						cout << "У вас нет заклинаний.\n";
						break;
					}
					cout << "Список заклинаний:\n";
					int index = 1;
					for (auto s : player.spells)
						cout << index++ << ". " << s.name << ' ' << s.manaCost << " MP\n";

					cout << "Выберите заклинание для использования: ";
					int spellChoice;
					cin >> spellChoice;
					if (spellChoice < 1 || spellChoice > player.spells.size()) {
						cout << "Неправильное заклинание\n";
						break;
					}

					cout << "Список целей:\n";
					index = 1;
					vector<Entity*> targets;
					for (auto& e : entities) {
						if(player.spells[spellChoice-1].type == DAMAGE)
							if (e.teamId != player.teamId && e.hp > 0) {
								cout << index++ << ". " << e.name << " (HP: " << e.hp << ")\n";
								targets.push_back(&e);
							}
						if (player.spells[spellChoice - 1].type == HEAL)
							if (e.teamId == player.teamId && e.hp > 0) {
								cout << index++ << ". " << e.name << " (HP: " << e.hp << ")\n";
								targets.push_back(&e);
							}
					}
					if (targets.empty()) return;

					cout << "Выберите цель для атаки: ";
					int choice;
					cin >> choice;
					if (choice >= 1 && choice <= targets.size())
						player.spells[spellChoice - 1].cast(player, *targets[choice-1]);
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

	Spell fireball("Fireball", 20, 15, DAMAGE);
	Spell smallHeal("Small healing", 10, 5, HEAL);

	Entity player("Player", 20, 100, 10, 0, true);
	player.spells.push_back(fireball);
	player.spells.push_back(smallHeal);
	Entity enemy("Enemy", 20, 10, 1);

	BattleManager manager;

	manager.addEntity(player);
	manager.addEntity(enemy);

	manager.startBattle();
}