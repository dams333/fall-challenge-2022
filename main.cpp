#include <iostream>
#include <string>
#include <list>
#include <algorithm>

using namespace std;

enum Player
{
	PLAYER_NONE = -1,
	PLAYER_ME = 1,
	PLAYER_OPPONENT = 0
};

Player convertToPlayer(int p)
{
	if (p == 0)
	{
		return PLAYER_OPPONENT;
	}
	else if (p == 1)
	{
		return PLAYER_ME;
	}
	else
	{
		return PLAYER_NONE;
	}
}

class Position
{
public:
	int x;
	int y;
	Position()
	{
		x = -1;
		y = -1;
	}
	Position(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
	Position(const Position &other)
	{
		*this = other;
	}
	Position &operator=(const Position &other)
	{
		x = other.x;
		y = other.y;
		return *this;
	}
};

class Case
{
public:
	Position pos;
	int scrap_amount;
	Player owner;
	int units;
	int recycler;
	int can_build;
	int can_spawn;
	int in_range_of_recycler;
	Case(Position pos, int scrap_amount, Player owner, int units, int recycler, int can_build, int can_spawn, int in_range_of_recycler)
	{
		this->pos = pos;
		this->scrap_amount = scrap_amount;
		this->owner = owner;
		this->units = units;
		this->recycler = recycler;
		this->can_build = can_build;
		this->can_spawn = can_spawn;
		this->in_range_of_recycler = in_range_of_recycler;
	}
	Case(const Case &other)
	{
		*this = other;
	}
	Case &operator=(const Case &other)
	{
		pos = other.pos;
		scrap_amount = other.scrap_amount;
		owner = other.owner;
		units = other.units;
		recycler = other.recycler;
		can_build = other.can_build;
		can_spawn = other.can_spawn;
		in_range_of_recycler = other.in_range_of_recycler;
		return *this;
	}
};

class Bot
{
public:
	Position pos;
	Player owner;
	Bot(Position pos, Player owner)
	{
		this->pos = pos;
		this->owner = owner;
	}
	Bot(const Bot &other)
	{
		*this = other;
	}
	Bot &operator=(const Bot &other)
	{
		pos = other.pos;
		owner = other.owner;
		return *this;
	}
};

class AAction
{
public:
	virtual ~AAction() = 0;
	virtual string extractString() = 0;
};

class ActionMove : public AAction
{
public:
	Position from;
	Position to;
	int amount_of_units;
	ActionMove(Position from, Position to, int amount_of_units)
	{
		this->from = from;
		this->to = to;
		this->amount_of_units = amount_of_units;
	}
	string extractString()
	{
		return "MOVE " + to_string(amount_of_units) + " " + to_string(from.x) + " " + to_string(from.y) + " " + to_string(to.x) + " " + to_string(to.y);
	}
};

class ActionBuildRecycler : public AAction
{
public:
	Position pos;
	ActionBuildRecycler(Position pos)
	{
		this->pos = pos;
	}
	string extractString()
	{
		return "BUILD " + to_string(pos.x) + " " + to_string(pos.y);
	}
};

class ActionSpawn : public AAction
{
public:
	Position pos;
	int amount_of_units;
	ActionSpawn(Position pos, int amount_of_units)
	{
		this->pos = pos;
		this->amount_of_units = amount_of_units;
	}
	string extractString()
	{
		return "SPAWN " + to_string(amount_of_units) + " " + to_string(pos.x) + " " + to_string(pos.y);
	}
};

class ActionMessage : public AAction
{
public:
	string message;
	ActionMessage(string message)
	{
		this->message = message;
	}
	string extractString()
	{
		return "MSG " + message;
	}
};

class Game;

class ActionManager
{
public:
	list<AAction *> actions;
	ActionManager() {}
	void addAction(AAction *action)
	{
		actions.push_back(action);
	}
	void execute(Game &map)
	{
		if (actions.size() > 0)
		{
			for (auto it = actions.begin(); it != actions.end(); it++)
			{
				cout << (it == actions.begin() ? "" : ";") << (*it)->extractString() << endl;
				delete *it;
			}
		}
		else
		{
			cout << "WAIT" << endl;
		}
		actions.clear();
	}
};

class Game
{
public:
	int width;
	int height;
	int my_matter;
	int opp_matter;
	list<list<Case>> map;
	list<Bot> my_bots;
	list<Bot> opp_bots;
	ActionManager action_manager;

	Game(int width, int height)
	{
		this->width = width;
		this->height = height;
	}

	void read_inputs()
	{
		map.clear();
		my_bots.clear();
		opp_bots.clear();
		cin >> my_matter >> opp_matter;
		cin.ignore();
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				int scrap_amount;
				int owner;
				int units;
				int recycler;
				int can_build;
				int can_spawn;
				int in_range_of_recycler;
				cin >> scrap_amount >> owner >> units >> recycler >> can_build >> can_spawn >> in_range_of_recycler;
				cin.ignore();
				if (i == 0)
				{
					map.push_back(list<Case>());
				}
				map.back().push_back(Case(Position(i, j), scrap_amount, convertToPlayer(owner), units, recycler, can_build, can_spawn, in_range_of_recycler));
				for (int i = 0; i < units; i++)
				{
					if (owner == 0)
					{
						my_bots.push_back(Bot(Position(i, j), convertToPlayer(owner)));
					}
					else
					{
						opp_bots.push_back(Bot(Position(i, j), convertToPlayer(owner)));
					}
				}
			}
		}
	}

	void register_action(AAction *action)
	{
		action_manager.addAction(action);
	}
	void execute_actions()
	{
		action_manager.execute(*this);
	}
};

int main()
{
	int width;
	int height;
	cin >> width >> height;
	cin.ignore();

	Game game(width, height);

	while (1)
	{
		game.read_inputs();

		game.execute_actions();
	}
}