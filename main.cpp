#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#define BOT_DEBUG 0
#define INPUT_DEBUG 0
#define RECYCLER_DEBUG 0

/*=======================================================================
||                        Classes declaration                          ||
=======================================================================*/

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

string playerToType(Player &p)
{
	if (p == PLAYER_ME)
	{
		return "ME";
	}
	else if (p == PLAYER_OPPONENT)
	{
		return "OPPONENT";
	}
	else
	{
		return "NONE";
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
	int distance(Position other)
	{
		return abs(x - other.x) + abs(y - other.y);
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
		if (BOT_DEBUG)
			cerr << "Bot created at " << pos.x << " " << pos.y << " for: " << playerToType(owner) << endl;
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
	int distance(Bot &other)
	{
		return pos.distance(other.pos);
	}
};

class AAction
{
public:
	virtual string extractString() = 0;
	virtual int matterRemove() = 0;
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
	int matterRemove()
	{
		return 0;
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
	int matterRemove()
	{
		return 10;
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
	int matterRemove()
	{
		return 10;
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
	int matterRemove()
	{
		return 0;
	}
};

class Game;
class ActionManager
{
public:
	vector<AAction *> actions;
	ActionManager();
	void addAction(Game *game, AAction *action);
	void execute();
};

class Game
{
public:
	int width;
	int height;
	int my_matter;
	int opp_matter;
	vector<Case> cases;
	vector<Bot> my_bots;
	vector<Bot> opp_bots;
	ActionManager action_manager;
	Game(int width, int height);
	void read_inputs();
	void register_action(AAction *action);
	void execute_actions();
	Case &get_case(Position pos);
	Case &get_case(int x, int y);
};

ActionManager::ActionManager() {}
void ActionManager::addAction(Game *game, AAction *action)
{
	game->my_matter -= action->matterRemove();
	actions.push_back(action);
}
void ActionManager::execute()
{
	if (actions.size() > 0)
	{
		for (auto it = actions.begin(); it != actions.end(); it++)
		{
			cout << (it == actions.begin() ? "" : ";") << (*it)->extractString();
			delete *it;
		}
		cout << endl;
	}
	else
	{
		cout << "WAIT" << endl;
	}
	actions.clear();
}

Game::Game(int width, int height)
{
	this->width = width;
	this->height = height;
}

void Game::read_inputs()
{
	cases.clear();
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
			Case c = Case(Position(j, i), scrap_amount, convertToPlayer(owner), units, recycler, can_build, can_spawn, in_range_of_recycler);
			if (INPUT_DEBUG)
			{
				cerr << "Read case " << c.pos.x << " " << c.pos.y << endl;
			}
			cases.push_back(c);
			for (int i = 0; i < units; i++)
			{
				if (owner == PLAYER_ME)
				{
					my_bots.push_back(Bot(c.pos, convertToPlayer(owner)));
				}
				else if (owner == PLAYER_OPPONENT)
				{
					opp_bots.push_back(Bot(c.pos, convertToPlayer(owner)));
				}
			}
		}
	}
	if (INPUT_DEBUG)
	{
		cerr << cases.size() << " cases readed" << endl;
		cerr << "All inputs readed" << endl;
	}
}

void Game::register_action(AAction *action)
{
	action_manager.addAction(this, action);
}
void Game::execute_actions()
{
	action_manager.execute();
}

Case &Game::get_case(Position pos)
{
	return cases[pos.y * width + pos.x];
}

Case &Game::get_case(int x, int y)
{
	return get_case(Position(x, y));
}

/*=======================================================================
||                          Utils functions                            ||
=======================================================================*/

Position get_nearest(Position from, vector<Bot> targets)
{
	if (targets.size() == 0)
	{
		return from;
	}
	Position nearest = targets.front().pos;
	int nearest_distance = from.distance(nearest);
	for (auto it = targets.begin(); it != targets.end(); it++)
	{
		int distance = from.distance(it->pos);
		if (distance < nearest_distance)
		{
			nearest = it->pos;
			nearest_distance = distance;
		}
	}
	return nearest;
}

bool isCaseUsefulForRecycler(Game &game, Position pos)
{
	if (RECYCLER_DEBUG)
		cerr << "Test case " << pos.x << " " << pos.y << " for recycler: ";
	if (pos.x < 1 || pos.x > game.width - 2 || pos.y < 1 || pos.y > game.height - 2)
	{
		if (RECYCLER_DEBUG)
			cerr << "out of map" << endl;
		return false;
	}
	if (game.get_case(pos.x, pos.y - 1).scrap_amount <= 0)
	{
		if (RECYCLER_DEBUG)
			cerr << "no scrap on top" << endl;
		return false;
	}
	if (game.get_case(pos.x, pos.y + 1).scrap_amount <= 0)
	{
		if (RECYCLER_DEBUG)
			cerr << "no scrap on bottom" << endl;
		return false;
	}
	if (game.get_case(pos.x - 1, pos.y).scrap_amount <= 0)
	{
		if (RECYCLER_DEBUG)
			cerr << "no scrap on left" << endl;
		return false;
	}
	if (game.get_case(pos.x + 1, pos.y).scrap_amount <= 0)
	{
		if (RECYCLER_DEBUG)
			cerr << "no scrap on right" << endl;
		return false;
	}
	if (game.get_case(pos.x, pos.y - 1).in_range_of_recycler)
	{
		if (RECYCLER_DEBUG)
			cerr << "on top is already recycled" << endl;
		return false;
	}
	if (game.get_case(pos.x, pos.y + 1).in_range_of_recycler)
	{
		if (RECYCLER_DEBUG)
			cerr << "on bottome is already recycled" << endl;
		return false;
	}
	if (game.get_case(pos.x - 1, pos.y).in_range_of_recycler)
	{
		if (RECYCLER_DEBUG)
			cerr << "on left is already recycled" << endl;
		return false;
	}
	if (game.get_case(pos.x + 1, pos.y).in_range_of_recycler)
	{
		if (RECYCLER_DEBUG)
			cerr << "on right is already recycled" << endl;
		return false;
	}
	if (RECYCLER_DEBUG)
		cerr << "valid" << endl;
	return true;
}

/*=======================================================================
||                           Main Function                             ||
=======================================================================*/

int main()
{
	int width;
	int height;
	cin >> width >> height;
	cin.ignore();
	if (INPUT_DEBUG)
	{
		cerr << "Readed initialization inputs" << endl;
	}

	Game game(width, height);

	int turn = 0;
	while (++turn)
	{
		game.read_inputs();

		for (auto it = game.my_bots.begin(); it != game.my_bots.end(); it++)
		{
			game.register_action(new ActionMove(it->pos, get_nearest(it->pos, game.opp_bots), 1));
		}

		if (game.my_matter >= 10)
		{
			for (auto it = game.cases.begin(); it != game.cases.end(); it++)
			{
				if (it->can_build && isCaseUsefulForRecycler(game, it->pos))
				{
					game.register_action(new ActionBuildRecycler(it->pos));
					break;
				}
			}
		}

		if (game.my_matter >= 20)
		{
			for (auto it = game.cases.begin(); it != game.cases.end(); it++)
			{
				if (it->can_spawn)
				{
					game.register_action(new ActionSpawn(it->pos, 1));
					break;
				}
			}
		}

		game.execute_actions();
	}
}