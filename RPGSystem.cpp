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
	DOT, // damage over time
	HOT // heal over time
};

class Entity;

class Effect {
public:
	string name;
	int duration;
	int value;
	EffectType type;
	bool hasExpired = false;

	Effect(string _name, int _duration, int _value, EffectType _type) : name(_name), duration(_duration), value(_value), type(_type) {}

	void apply(Entity& target);
};

class Spell {
public:
	string name;
	int value;
	int manaCost;
	SpellType type;
	bool isAoe;
	int size;
	vector<Effect> effects;
	Spell(string _name, int _manaCost, int _value, SpellType _type) : name(_name), manaCost(_manaCost), value(_value), type(_type), isAoe(false) {}
	Spell(string _name, int _manaCost, int _value, SpellType _type, bool aoe, int _size) : Spell(_name, _manaCost, _value, _type) {
		isAoe = aoe;
		size = _size;
	}
	void cast(Entity& caster, Entity& target);
	void cast(Entity& caster, vector<Entity*>& targets);

	void addEffect(Effect effect) {
		effects.push_back(effect);
	}
};

class Item{};

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
	vector<Effect> activeEffects;
	vector<Item> inventory;

	Entity(string _name, int _hp = 100, int _mana = 100, int _dmg = 10, int _teamId = 1, bool player = false) : name(_name), hp(_hp), maxHp(_hp), mana(_mana), maxMana(_mana), dmg(_dmg), teamId(_teamId), isPlayer(player) {}

	void takeDamage(int _dmg) {
		if (hp > 0) {
			hp -= _dmg;
		}
	}

	void setInitiative() {
		initiative = rand() % 6 + 1;
	}
	
	void applyEffects() {
		vector<Effect> remain;
		for (auto& e : activeEffects) {
			e.apply(*this);
			if (e.hasExpired)
				continue;
			remain.push_back(e);
		}
		activeEffects = remain;
	}
};

void Effect::apply(Entity& target) {
	switch (type) {
	case DOT: {
		if (target.hp > 0) {
			target.takeDamage(value);
			cout << target.name << " suffers from " << name << " in amount of " << value << " dmg\n";
		}
		duration--;
		if (duration == 0)
			hasExpired = true;
		break;
	}
	case HOT: {
		if (target.hp > 0 && target.hp < target.maxHp) {
			target.hp += value;
			if (target.hp > target.maxHp) target.hp = target.maxHp;
			cout << target.name << " heales from " << name << " in amount of " << value << " HP\n";
		}
		duration--;
		if (duration == 0)
			hasExpired = true;
		break;
	}
	}
}

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
		break;
	case EFFECT:
		if (caster.mana >= manaCost && target.hp > 0) {
			caster.mana -= manaCost;
			for (auto& e : this->effects) {
				target.activeEffects.push_back(e);
				cout << caster.name << " uses " << e.name << " on " << target.name << '\n';
			}
		}
		break;
	}
}

void Spell::cast(Entity& caster, vector<Entity*>& targets) {
	switch (type) {
	case HEAL:
		for(auto& e : targets)
			if (caster.mana >= manaCost && e->hp < e->maxHp) {
				caster.mana -= manaCost;
				e->hp += value;
				if (e->hp > e->maxHp) e->hp = e->maxHp;
				cout << caster.name << " healed " << e->name << " on " << value << " hp.\n";
		}
		break;
	case DAMAGE:
		for(auto& e : targets)
			if (caster.mana >= manaCost && e->hp > 0) {
				caster.mana -= manaCost;
				e->takeDamage(value);
				cout << caster.name << " damaged " << e->name << " on " << value << " hp.\n";
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
	bool hasEscaped = false;

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

	vector<Entity*> selectTargets(int size, int teamId, bool enemy) {
		vector<Entity*> targets;
		vector<Entity*> available;
		int index = 1;
		for (auto& e : entities) {
			if (enemy) {
				if (e.teamId != teamId && e.hp > 0) {
					cout << index++ << ". " << e.name << " (" << e.hp << "HP)\n";
					available.push_back(&e);
				}
			}
			else {
				if (e.teamId == teamId && e.hp > 0) {
					cout << index++ << ". " << e.name << " (" << e.hp << "HP)\n";
					available.push_back(&e);
				}
			}
		}
		if (available.size() < size)
			size = available.size();
		int ch = 0;
		while (targets.size() < size) {
			cout << "Введите цель: ";
			cin >> ch;
			if (ch < 1 || ch > available.size()) {
				cout << "Некорректный ввод";
				continue;
			}

			Entity* selected = available[ch - 1];

			if (find(targets.begin(), targets.end(), selected) != targets.end()) {
				cout << "Цель уже выбрана\n";
				continue;
			}

			targets.push_back(selected);
		}
		return targets;
	}

	void displaySpells(Entity& caster) {
		if (caster.spells.empty()) {
			cout << "У вас нет заклинаний\n";
			return;
		}

		int index = 1;
		for (auto& s : caster.spells)
			if(s.isAoe)
				cout << index++ << ". " << s.name << ' ' << s.manaCost << " MP. Количество целей: " << s.size << '\n';
			else
				cout << index++ << ". " << s.name << ' ' << s.manaCost << " MP\n";
	}

	void playerTurn(Entity& player) {
		player.applyEffects();
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

				if (player.spells[spellChoice-1].isAoe) {
					if (player.spells[spellChoice - 1].type == DAMAGE) {
						vector<Entity*> targets = selectTargets(player.spells[spellChoice - 1].size, 0, true);
						player.spells[spellChoice - 1].cast(player, targets);
					}

					if (player.spells[spellChoice - 1].type == HEAL) {
						vector<Entity*> targets = selectTargets(player.spells[spellChoice-1].size, 0, false);
						player.spells[spellChoice - 1].cast(player, targets);
					}
				}
				else {
					if (player.spells[spellChoice - 1].type == DAMAGE || player.spells[spellChoice - 1].type == EFFECT) {
						Entity* target = selectTarget(0, true);
						player.spells[spellChoice - 1].cast(player, *target);
					}

					if (player.spells[spellChoice - 1].type == HEAL) {
						Entity* target = selectTarget(0, false);
						player.spells[spellChoice - 1].cast(player, *target);
					}
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
		current.applyEffects();
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
			if(e.hp > 0)
				teamsCount.insert(e.teamId);
		return teamsCount.size() <= 1;
	}
};

int main() {
	setlocale(LC_ALL, "Rus");

	srand(time({}));

	Effect posion_effect("Posion", 2, 5, DOT);
	Effect burn("Burn", 2, 7, DOT);

	Spell fireball("Fireball", 20, 15, DAMAGE);
	fireball.addEffect(burn);
	Spell smallHeal("Small healing", 10, 5, HEAL);

	Spell big_fireball("Big Fireball", 50, 30, DAMAGE, true, 3);

	Spell poison("Posion", 20, 0, EFFECT);
	poison.addEffect(posion_effect);

	Entity player("Player", 50, 100, 10, 0, true);
	player.spells.push_back(fireball);
	player.spells.push_back(smallHeal);
	player.spells.push_back(big_fireball);
	player.spells.push_back(poison);
	Entity enemy("Enemy", 20, 0, 10);

	BattleManager manager;

	manager.addEntity(player);
	manager.addEntity(enemy);

	manager.startBattle();
}