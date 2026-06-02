#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>

using namespace std;

enum SpellType {
	HEAL,
	DAMAGE,
	EFFECT
};

enum EffectType {
	PERIODIC, // heal, damage
	CONTROL // оглушение, восприимчивость и др
};

class Entity;

class Effect {
	int duration;
	int value;
	EffectType type;
	void apply(Entity& target);
};

class Spell {
public:
	string name;
	int value;
	int manaCost;
	SpellType type;
	vector<Effect> effects;
	Spell(string _name, int _manaCost, int _value, SpellType _type) : name(_name), manaCost(_manaCost), value(_value), type(_type) {}
	void cast(Entity& caster, Entity& target);
};

class Entity {
public:
	string name;
	int hp;
	int maxHp;
	int mana;
	int maxMana;
	int dmg;
	int teamId; // 0 - player + allies, 1 - enemies and so on
	bool isPlayer = false;
	int initiative;
	vector<Spell> spells;

	Entity(string _name, int _hp = 100, int _mana = 100, int _dmg = 10, int _teamId = 1, bool player = false) : name(_name), hp(_hp), maxHp(_hp), mana(_mana), maxMana(_mana), dmg(_dmg), teamId(_teamId), isPlayer(player) {}

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

	void startBattle() {
		hasEscaped = false;
		rollInitiatve();
		while (!isBattleOver() && !hasEscaped) {
			for (auto& e : entities) {
				if (e.hp <= 0) continue;

				if (e.isPlayer)
					playerTurn(e);
				else
					aiTurn(e);

				if (isBattleOver()) break;
				if (hasEscaped) {
					cout << "Бой окончен.\n";
					return;
				}
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
	bool hasEscaped;

	void rollInitiatve() {
		for (auto& e : entities)
			e.setInitiative();
		sort(entities.begin(), entities.end(), initiativeComp);
	}

	int getAliveEnemies(int teamId) {
		int count = 0;
		for (auto& e : entities) {
			if (e.teamId != teamId && e.hp > 0)
				count++;
		}
		return count;
	}

	Entity* selectTarget(int teamId, bool enemy) {
		cout << "Список целей:\n";
		int index = 1;
		vector<Entity*> targets;
		for (auto& e : entities) {
			if (enemy) {
				if (e.teamId != teamId && e.hp > 0) {
					cout << index++ << ". " << e.name << " (" << e.hp << " HP)\n";
					targets.push_back(&e);
				}
			}
			else {
				if (e.teamId == teamId && e.hp > 0) {
					cout << index++ << ". " << e.name << " (" << e.hp << " HP)\n";
					targets.push_back(&e);
				}
			}
		}
		if (targets.empty()) return nullptr;
		int ch = 0;
		while (true) {
			cout << "Выберите цель: ";
			cin >> ch;
			if (ch >= 1 && ch <= targets.size())
				return targets[ch - 1];
			cout << "Неправильная цель\n";
		}
	}

	void displaySpells(Entity& caster) {
		if (caster.spells.empty()) {
			cout << "У вас нет заклинаний\n";
			return;
		}

		int index = 1;
		for (auto& s : caster.spells)
			cout << index++ << ". " << s.name << ' ' << s.manaCost << " MP\n";
	}

	void playerTurn(Entity& player) {
		int num;
		bool action{ false };
		while (action == false) {
			cout << "1. Атака\n2. Способность\n3. Магия\n4. Предметы\n5. Побег\nВыберите действие: ";
			cin >> num;
			if (cin.fail()) {
				cin.clear();
				cin.ignore(1000, '\n');
			}
			switch (num) {
			case 1:
			{
				Entity* target = selectTarget(0, true);
				if (target) {
					target->takeDamage(player.dmg);
					cout << "Player dealt " << player.dmg << " damage to " << target->name << '\n';
				}
				else {
					cout << " Нет доступных целей\n";
				}
				cout << endl;
				action = true;
				break;
			}
			case 3:
			{
				displaySpells(player);

				cout << "Выберите заклинание для использования: ";
				int spellChoice;
				cin >> spellChoice;
				if (spellChoice < 1 || spellChoice > player.spells.size()) {
					cout << "Неправильное заклинание\n";
					break;
				}

				if (player.spells[spellChoice - 1].type == DAMAGE) {
					Entity* target = selectTarget(0, true);
					player.spells[spellChoice - 1].cast(player, *target);
				}

				if (player.spells[spellChoice - 1].type == HEAL) {
					Entity* target = selectTarget(0, false);
					player.spells[spellChoice - 1].cast(player, *target);
				}

				action = true;
				break;
			}
			case 5: {
				cout << "Вы пытаетесь сбежать..\n";
				int chance = 50 - (getAliveEnemies(0) * 10);
				if (chance < 10) chance = 10;
				if (rand() % 100 < chance) {
					cout << "Вы успешно сбежали!\n";
					action = true;
					hasEscaped = true;
				}
				else {
					cout << "У вас не получилось сбежать.\n";
					action = true;
				}
				break;
			}
			default:
				cout << "Incorrect action\n" << endl;
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
		set<int> teamsCount;
		for (auto& e : entities)
			if (e.hp > 0)
				teamsCount.insert(e.teamId);
		return teamsCount.size() <= 1;
	}
};

int main() {
	setlocale(LC_ALL, "Rus");

	srand(time({}));

	Spell fireball("Fireball", 20, 15, DAMAGE);
	Spell smallHeal("Small healing", 10, 5, HEAL);

	Entity player("Player", 20, 100, 10, 0, true);
	player.spells.push_back(fireball);
	player.spells.push_back(smallHeal);
	Entity enemy("Enemy", 20, 0, 10);

	BattleManager manager;

	manager.addEntity(player);
	manager.addEntity(enemy);

	manager.startBattle();
}