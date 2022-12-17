#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

#define BOT_DEBUG 0
#define INPUT_DEBUG 0
#define RECYCLER_DEBUG 0
#define SPAWN_DEBUG 0
#define EXPAND_DEBUG 0
#define TERRITORY_DEBUG 0

/*=======================================================================
||                                                                     ||
||                               Classes                               ||
||                                                                     ||
=======================================================================*/

/*=======================================================================
||                               Player                                ||
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

/*=======================================================================
||                                Mode                                ||
=======================================================================*/
enum Mode
{
	EXPAND,
	SPLATOON
};

/*=======================================================================
||                              Position                               ||
=======================================================================*/
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
	bool operator==(const Position &other) const
	{
		return x == other.x && y == other.y;
	}
};

Position operator-(const Position &a, const Position &b)
{
	return Position(a.x - b.x, a.y - b.y);
}

Position operator+(const Position &a, const Position &b)
{
	return Position(a.x + b.x, a.y + b.y);
}

/*=======================================================================
||                                Case                                 ||
=======================================================================*/
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
	bool operator==(const Case &other) const
	{
		return pos == other.pos;
	}
};

/*=======================================================================
||                                 Bot                                 ||
=======================================================================*/
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

/*=======================================================================
||                               Actions                               ||
=======================================================================*/
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
		return "MESSAGE " + message;
	}
	int matterRemove()
	{
		return 0;
	}
};

/*=======================================================================
||                       ActionManager prototype                       ||
=======================================================================*/
class Game;
class ActionManager
{
public:
	vector<AAction *> actions;
	ActionManager();
	void addAction(Game *game, AAction *action);
	void execute();
};

/*=======================================================================
||                           Game prototype                            ||
=======================================================================*/
class Teritory;
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
	vector<Teritory> teritories;
	ActionManager action_manager;
	Game(int width, int height);
	void read_inputs();
	void register_action(AAction *action);
	void register_spawn_remove_move(ActionSpawn *action);
	void execute_actions();
	Case &get_case(Position pos);
	Case &get_case(int x, int y);
	vector<Teritory> get_teritories();
	Bot &getBot(Position pos);
};

/*=======================================================================
||                         Territory prototype                         ||
=======================================================================*/
class Teritory
{
public:
	vector<Case> cases;
	vector<Bot> my_bots;
	vector<Bot> opp_bots;
	Mode mode;
	Player owner;
	Teritory();
	Teritory(const Teritory &t);
	void operator=(const Teritory &t);
	void add_case(Case c);
	bool is_in(Position pos);
	bool is_in(Case c);
	void addCaseAndNeighbours(Game &game, Case &c);
	void addCaseAndNeighbours(Game &game, Case &c, int xDir, int yDir);
	void buildFrom(Game &game, Case &c);
	void buildFrom(Game &game, Case &c, int xDir, int yDir);
	bool isIsolate();
	bool isIsolateWithCase();
	vector<Bot> getBots(Game &game);
};

/*=======================================================================
||                      ActionManager declaration                      ||
=======================================================================*/
ActionManager::ActionManager() {}
void ActionManager::addAction(Game *game, AAction *action)
{
	if (dynamic_cast<ActionSpawn *>(action) != NULL)
	{
		if (dynamic_cast<ActionSpawn *>(action)->amount_of_units <= 0)
			return;
	}
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

/*=======================================================================
||                          Game declaration                           ||
=======================================================================*/
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
	teritories.clear();

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

void Game::register_spawn_remove_move(ActionSpawn *action)
{
	for (auto it = action_manager.actions.begin(); it != action_manager.actions.end(); it++)
	{
		if (dynamic_cast<ActionMove *>(*it) != NULL)
		{
			ActionMove *move = dynamic_cast<ActionMove *>(*it);
			if (move->from == action->pos)
			{
				if (SPAWN_DEBUG)
					cerr << "Remove move action at " << move->from.x << " " << move->from.y << endl;
				action_manager.actions.erase(it);
				break;
			}
		}
	}
	action_manager.addAction(this, action);
}

vector<Teritory> Game::get_teritories()
{
	if (teritories.size() == 0)
	{
		for (auto it = cases.begin(); it != cases.end(); it++)
		{
			if (it->scrap_amount > 0 && it->recycler <= 0)
			{
				bool found = false;
				for (auto it2 = teritories.begin(); it2 != teritories.end(); it2++)
				{
					if (it2->is_in(*it))
					{
						found = true;
						break;
					}
				}
				if (found)
					continue;
				Teritory t = Teritory();
				t.buildFrom(*this, *it);
				teritories.push_back(t);
			}
		}
		if (TERRITORY_DEBUG)
		{
			for (auto it = teritories.begin(); it != teritories.end(); it++)
			{
				cerr << "Territory at " << it->cases[0].pos.x << " " << it->cases[0].pos.y << " with " << it->cases.size() << " cases" << endl;
			}
		}
	}
	return teritories;
}

Bot &Game::getBot(Position pos)
{
	for (auto it = my_bots.begin(); it != my_bots.end(); it++)
	{
		if (it->pos == pos)
		{
			return *it;
		}
	}
	for (auto it = opp_bots.begin(); it != opp_bots.end(); it++)
	{
		if (it->pos == pos)
		{
			return *it;
		}
	}
	return my_bots[0];
}

/*=======================================================================
||                         Territory declaration                       ||
=======================================================================*/

Teritory::Teritory() : mode(EXPAND) {}

Teritory::Teritory(const Teritory &t) { *this = t; }

void Teritory::operator=(const Teritory &t)
{
	cases = t.cases;
	my_bots = t.my_bots;
	opp_bots = t.opp_bots;
}

void Teritory::add_case(Case c)
{
	cases.push_back(c);
	if (c.units > 0)
	{
		for (int i = 0; i < c.units; i++)
		{
			if (c.owner == PLAYER_ME)
			{
				my_bots.push_back(Bot(c.pos, PLAYER_ME));
			}
			else if (c.owner == PLAYER_OPPONENT)
			{
				opp_bots.push_back(Bot(c.pos, PLAYER_OPPONENT));
			}
		}
	}
}

bool Teritory::is_in(Position pos)
{
	for (auto it = cases.begin(); it != cases.end(); it++)
	{
		if (it->pos == pos)
		{
			return true;
		}
	}
	return false;
}

bool Teritory::is_in(Case c)
{
	return is_in(c.pos);
}

void Teritory::addCaseAndNeighbours(Game &game, Case &c)
{
	add_case(c);
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->pos.distance(c.pos) == 1 && it->scrap_amount > 0 && it->recycler <= 0 && !is_in(*it))
		{
			addCaseAndNeighbours(game, *it);
		}
	}
}

void Teritory::addCaseAndNeighbours(Game &game, Case &c, int xDir, int yDir)
{
	add_case(c);
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->pos.distance(c.pos) == 1 && it->scrap_amount > 0 && it->recycler <= 0 && !is_in(*it))
		{
			if ((xDir == 1 && it->pos.x >= c.pos.x) || (xDir == -1 && it->pos.x <= c.pos.x) || (xDir == 0))
				if ((yDir == 1 && it->pos.y >= c.pos.y) || (yDir == -1 && it->pos.y <= c.pos.y) || (yDir == 0))
					addCaseAndNeighbours(game, *it);
		}
	}
}

void Teritory::buildFrom(Game &game, Case &c)
{
	if (TERRITORY_DEBUG)
		cerr << "Building territory from " << c.pos.x << " " << c.pos.y << endl;
	addCaseAndNeighbours(game, c);
	if (isIsolate())
	{
		mode = SPLATOON;
	}
}

void Teritory::buildFrom(Game &game, Case &c, int xDir, int yDir)
{
	if (TERRITORY_DEBUG)
		cerr << "Building territory from " << c.pos.x << " " << c.pos.y << endl;
	addCaseAndNeighbours(game, c, xDir, yDir);
	if (isIsolate())
	{
		mode = SPLATOON;
	}
}

bool Teritory::isIsolate()
{
	bool isolate = true;
	owner = PLAYER_NONE;
	for (auto it = cases.begin(); it != cases.end(); it++)
	{
		if (owner == PLAYER_NONE)
		{
			owner = it->owner;
		}
		else if (owner != it->owner && it->owner != PLAYER_NONE)
		{
			isolate = false;
		}
	}
	return isolate;
}

bool Teritory::isIsolateWithCase()
{
	bool isolate = true;
	owner = PLAYER_NONE;
	for (auto it = cases.begin(); it != cases.end(); it++)
	{
		if (owner == PLAYER_NONE && it->units > 0)
		{
			owner = it->owner;
		}
		else if (owner != it->owner && it->owner != PLAYER_NONE && it->units > 0)
		{
			isolate = false;
		}
	}
	return isolate;
}

vector<Bot> Teritory::getBots(Game &game)
{
	vector<Bot> bots;
	for (auto it = cases.begin(); it != cases.end(); it++)
	{
		if (it->units > 0)
		{
			bots.push_back(game.getBot(it->pos));
		}
	}
	return bots;
}

/*=======================================================================
||                                                                     ||
||                          Utils functions                            ||
||                                                                     ||
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

Position get_nearest(Position from, vector<Case> targets)
{
	if (targets.size() == 0)
	{
		return from;
	}
	Position nearest = targets.front().pos;
	int nearest_distance = from.distance(nearest);
	while (targets.size() > 0)
	{
		Case c = targets[rand() % targets.size()];
		int distance = from.distance(c.pos);
		if (distance < nearest_distance)
		{
			nearest = c.pos;
			nearest_distance = distance;
		}
		targets.erase(find(targets.begin(), targets.end(), c));
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
	if (game.get_case(pos.x + 1, pos.y).owner == PLAYER_ME && game.get_case(pos.x - 1, pos.y).owner == PLAYER_ME && game.get_case(pos.x, pos.y + 1).owner == PLAYER_ME && game.get_case(pos.x, pos.y - 1).owner == PLAYER_ME)
	{
		if (RECYCLER_DEBUG)
			cerr << "in my teritory" << endl;
		return false;
	}
	if (RECYCLER_DEBUG)
		cerr << "valid" << endl;
	return true;
}

vector<Case> adajcent(Case &src, Game &game)
{
	vector<Case> result;
	if (src.pos.x > 0)
	{
		result.push_back(game.get_case(src.pos.x - 1, src.pos.y));
	}
	if (src.pos.x < game.width - 1)
	{
		result.push_back(game.get_case(src.pos.x + 1, src.pos.y));
	}
	if (src.pos.y > 0)
	{
		result.push_back(game.get_case(src.pos.x, src.pos.y - 1));
	}
	if (src.pos.y < game.height - 1)
	{
		result.push_back(game.get_case(src.pos.x, src.pos.y + 1));
	}
	return result;
}

vector<Case> not_mine_available_to_move(Game &game)
{
	vector<Case> result;
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->owner != PLAYER_ME && it->scrap_amount > 0)
		{
			result.push_back(*it);
		}
	}
	return result;
}

vector<Case> other_cases(Game &game)
{
	vector<Case> result;
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->owner == PLAYER_OPPONENT)
		{
			result.push_back(*it);
		}
	}
	return result;
}

void init_recycler(Game &game)
{
	if (game.get_case(game.my_bots[0].pos.x, game.my_bots[0].pos.y + 1).scrap_amount >= 8)
	{
		game.register_action(new ActionBuildRecycler(Position(game.my_bots[0].pos.x, game.my_bots[0].pos.y + 1)));
	}
}

bool is_bot_on_line(Game &game, int height)
{
	for (auto it = game.my_bots.begin(); it != game.my_bots.end(); it++)
	{
		if (it->pos.y == height)
		{
			return true;
		}
	}
}

int count_bot_on_line(Game &game, int height)
{
	int result = 0;
	for (auto it = game.my_bots.begin(); it != game.my_bots.end(); it++)
	{
		if (it->pos.y == height)
		{
			result++;
		}
	}
	return result;
}

Bot &get_most_advanced_on_line(Game &game, int direction, int height)
{
	vector<Bot *> onLine;
	for (auto it = game.my_bots.begin(); it != game.my_bots.end(); it++)
	{
		if (it->pos.y == height)
		{
			onLine.push_back(&(*it));
		}
	}
	if (onLine.size() == 1)
	{
		return *onLine.front();
	}
	else
	{
		if (direction == -1)
		{
			return *onLine.front();
		}
		else
		{
			return *onLine.back();
		}
	}
}

int line_with_bot_in(Game &game, int src, int direction)
{
	int result = 0;
	for (int i = src; i >= 0 && i < game.height; i += direction)
	{
		if (is_bot_on_line(game, i))
		{
			result++;
		}
	}
	return result;
}

bool is_line_util(Game &game, int h, int w, int direction)
{
	Position target = Position(w, h);
	while (target.x >= 0 && target.x < game.width)
	{
		Case &c = game.get_case(target.x, target.y);
		if (c.scrap_amount <= 0 || c.recycler > 0)
		{
			break;
		}
		if (c.owner == PLAYER_OPPONENT)
		{
			return true;
		}
		target.x += direction;
	}
	return (direction == 1 && target.x > game.width / 2) || (direction == -1 && target.x < game.width / 2);
}

bool is_line_with_no_bot_in(Game &game, int src, int direction, int w)
{
	int available = 0;
	for (int i = src; i >= 0 && i < game.height; i += direction)
	{
		if (!is_bot_on_line(game, i) && is_line_util(game, i, w, direction))
		{
			available--;
			if (available <= 0)
				return true;
		}
		else if (i != src)
		{
			available += count_bot_on_line(game, i) - 1;
		}
	}
	return false;
}

void move_top_up(Game &game, int h, Position init_pos, int quantity, int direction)
{
	Position dest = Position(0, 0);
	if (line_with_bot_in(game, h, 1) > line_with_bot_in(game, h, -1))
	{
		// More bots on the bottom, better is to go top
		if (is_line_with_no_bot_in(game, h, -1, init_pos.x))
		{
			// There is a needed line in the top, go to top
			dest = Position(init_pos.x, init_pos.y - 1);
		}
		else if (is_line_with_no_bot_in(game, h, 1, init_pos.x))
		{
			// There is no needed line in the top, go to bottom
			dest = Position(init_pos.x, init_pos.y + 1);
		}
		else
		{
			// There is no needed line in the top, go to bottom
			dest = Position(init_pos.x + direction, init_pos.y);
		}
	}
	else
	{
		// More bots on the top, better is to go bottom
		if (is_line_with_no_bot_in(game, h, 1, init_pos.x))
		{
			// There is a needed line in the bottom, go to bottom
			dest = Position(init_pos.x, init_pos.y + 1);
		}
		else if (is_line_with_no_bot_in(game, h, -1, init_pos.x))
		{
			// There is no needed line in the bottom, go to top
			dest = Position(init_pos.x, init_pos.y - 1);
		}
		else
		{
			// There is no needed line in the bottom, go to bottom
			dest = Position(init_pos.x + direction, init_pos.y);
		}
	}
	if (game.get_case(dest).recycler > 0 || game.get_case(dest).scrap_amount <= 0)
	{
		dest = Position(init_pos.x - direction, init_pos.y);
	}
	game.register_action(new ActionMove(init_pos, dest, quantity));
}

bool is_available_for_defend(Case target)
{
	return target.owner == PLAYER_ME && target.scrap_amount > 0 && target.recycler == 0 && target.units == 0;
}

void expand(Game &game, int direction, vector<Bot> available)
{
	// Build recycler to block ennemy
	for (auto it = game.opp_bots.begin(); it != game.opp_bots.end(); it++)
	{
		if (game.my_matter < 10)
			break;
		Position target = Position(it->pos.x - direction, it->pos.y);
		if (is_available_for_defend(game.get_case(target)))
			game.register_action(new ActionBuildRecycler(target));
	}
	for (auto it = game.opp_bots.begin(); it != game.opp_bots.end(); it++)
	{
		if (game.my_matter < 10)
			break;
		Position target = Position(it->pos.x, it->pos.y + 1);
		if (is_available_for_defend(game.get_case(target)))
			game.register_action(new ActionBuildRecycler(target));
		if (game.my_matter < 10)
			break;
		target = Position(it->pos.x, it->pos.y - 1);
		if (is_available_for_defend(game.get_case(target)))
			game.register_action(new ActionBuildRecycler(target));
	}

	Position spawner;
	int dist = 1000;
	// Parcour lignes
	for (int h = 0; h < game.height; h++)
	{
		if (is_bot_on_line(game, h))
		{
			// Set director direction
			Bot &director = get_most_advanced_on_line(game, direction, h);
			Position target = Position(director.pos.x + direction, director.pos.y);
			if (game.get_case(target).scrap_amount > 0 && game.get_case(target).recycler <= 0 && ((game.get_case(target.x + direction, target.y).scrap_amount > 0 && game.get_case(target.x + direction, target.y).recycler <= 0) || (game.get_case(target.x, target.y + 1).scrap_amount > 0 && game.get_case(target.x, target.y + 1).recycler <= 0) || (game.get_case(target.x, target.y - 1).scrap_amount > 0 && game.get_case(target.x, target.y - 1).recycler <= 0)))
				game.register_action(new ActionMove(director.pos, target, 1));
			else
			{
				move_top_up(game, h, director.pos, 1, direction);
			}
			// Set other bots direction
			for (int w = director.pos.x; w >= 0 && w < game.width; w -= direction)
			{
				int usable = game.get_case(w, h).owner == PLAYER_ME ? game.get_case(w, h).units : 0;
				if (w == director.pos.x)
				{
					usable -= 1;
				}
				if (usable > 0)
				{
					move_top_up(game, h, Position(w, h), usable, direction);
				}
			}
			// Spawn new bot on the middle
			int d = abs(line_with_bot_in(game, h, 1) - line_with_bot_in(game, h, -1));
			if (d < dist)
			{
				dist = d;
				spawner = director.pos;
			}
		}
	}
	if (dist < 1000)
	{
		if (game.my_matter >= 10)
			game.register_action(new ActionSpawn(spawner, 1));
	}
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->owner != PLAYER_ME)
			continue;
		if (game.my_matter < 10)
			break;
		bool near = is_bot_on_line(game, it->pos.y);
		for (auto it2 = available.begin(); it2 != available.end(); it2++)
		{
			if (it->pos.distance(it2->pos) <= 2)
			{
				near = true;
				break;
			}
		}
		if (!near)
		{
			game.register_action(new ActionSpawn(it->pos, 1));
		}
	}
	if (available.size() == 0)
	{
		for (auto it = game.cases.begin(); it != game.cases.end(); it++)
		{
			if (it->owner == PLAYER_ME && it->units > 0)
			{
				game.register_action(new ActionSpawn(it->pos, 1));
				break;
			}
		}
	}
}

void splatoon(Game &game, vector<Bot> available)
{
	vector<Case> notMine;
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->owner == PLAYER_NONE && it->scrap_amount > 0)
		{
			notMine.push_back(*it);
		}
	}
	vector<Bot> myBots = available;
	for (auto it = notMine.begin(); it != notMine.end(); it++)
	{
		if (myBots.size() == 0)
			break;
		Position closest = get_nearest(it->pos, myBots);
		game.register_action(new ActionMove(closest, it->pos, 1));
		for (auto it2 = myBots.begin(); it2 != myBots.end(); it2++)
		{
			if (it2->pos == closest)
			{
				myBots.erase(it2);
				break;
			}
		}
	}
	for (auto it = myBots.begin(); it != myBots.end(); it++)
	{
		Position nearest = get_nearest(it->pos, notMine);
		game.register_action(new ActionMove(it->pos, nearest, 1));
		for (auto it2 = notMine.begin(); it2 != notMine.end(); it2++)
		{
			if (it2->pos == nearest)
			{
				notMine.erase(it2);
				break;
			}
		}
	}
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (game.my_matter < 10)
			break;
		if (it->owner == PLAYER_ME && it->units < 1)
		{
			game.register_action(new ActionSpawn(it->pos, 1));
		}
	}
}

bool isAllIsolate(Game &game)
{
	vector<Teritory> teritories = game.get_teritories();
	for (auto it = teritories.begin(); it != teritories.end(); it++)
	{
		if (!it->isIsolate())
			return false;
	}
	return true;
}

bool is_bot_on_line(Teritory &teritory, int height)
{
	for (auto it = teritory.my_bots.begin(); it != teritory.my_bots.end(); it++)
	{
		if (it->pos.y == height)
		{
			return true;
		}
	}
}

int count_bot_on_line(Teritory &teritory, int height)
{
	int result = 0;
	for (auto it = teritory.my_bots.begin(); it != teritory.my_bots.end(); it++)
	{
		if (it->pos.y == height)
		{
			result++;
		}
	}
	return result;
}

Bot &get_most_advanced_on_line(Teritory &teritory, int xDir, int height)
{
	vector<Bot *> onLine;
	for (auto it = teritory.my_bots.begin(); it != teritory.my_bots.end(); it++)
	{
		if (it->pos.y == height)
		{
			onLine.push_back(&(*it));
		}
	}
	if (onLine.size() == 1)
	{
		return *onLine.front();
	}
	else
	{
		int betterX = onLine.front()->pos.x;
		Bot &betterBot = *onLine.front();
		for (auto it = onLine.begin(); it != onLine.end(); it++)
		{
			if (xDir == 1)
			{
				if ((*it)->pos.x > betterX)
				{
					betterX = (*it)->pos.x;
					betterBot = **it;
				}
			}
			else
			{
				if ((*it)->pos.x < betterX)
				{
					betterX = (*it)->pos.x;
					betterBot = **it;
				}
			}
		}
		return betterBot;
	}
}

Position compute_target(Game &game, Position from, Position spawn, Position middle)
{
	Position offset = middle - spawn;
	Position to = from + offset;
	if (to.x < 0)
	{
		to.x = 0;
	}
	else if (to.x >= game.width)
	{
		to.x = game.width - 1;
	}
	if (to.y < 0)
	{
		to.y = 0;
	}
	else if (to.y >= game.height)
	{
		to.y = game.height - 1;
	}
	return to;
}

int count_bot_from(Game &game, Position from, Teritory &teritory, int yDir)
{
	int count = 0;
	for (int h = from.y + yDir; h >= 0 && h < game.height; h += yDir)
	{
		count += count_bot_on_line(teritory, h);
	}
	return count;
}

int count_line(Game &game, Position from, int yDir)
{
	int count = 0;
	for (int h = from.y + yDir; h >= 0 && h < game.height; h += yDir)
	{
		count++;
	}
	return count;
}

bool is_needed_line(Game &game, Position pos, int xDir)
{
	while (pos.x >= 0 && pos.x < game.width)
	{
		if ((game.get_case(pos.x, pos.y).scrap_amount <= 0 || game.get_case(pos.x, pos.y).recycler >= 0))
		{
			return (xDir == 1 && pos.x >= game.width / 2) || (xDir == -1 && pos.x <= game.width / 2);
		}
		if (game.get_case(pos.x, pos.y).owner == PLAYER_ME)
		{
			return false;
		}
		pos.x += xDir;
	}
	return true;
}

void expand(Game &game, Teritory &teritory, Position spawn, Position middle, int xDir)
{
	int save = xDir;
	Position saveSpawn = spawn;
	vector<Bot> directors;
	vector<Case> front_line;
	for (int h = 0; h < game.height; h++)
	{
		xDir = save;
		spawn = saveSpawn;
		for (int w = (xDir == 1 ? game.width - 1 : 0); w >= 0 && w < game.width; w -= xDir)
		{
			if (game.get_case(w, h).owner == PLAYER_ME && game.get_case(w, h).recycler <= 0)
			{
				front_line.push_back(game.get_case(w, h));
				break;
			}
		}
		if (is_bot_on_line(teritory, h))
		{
			Bot &director = get_most_advanced_on_line(teritory, xDir, h);
			if ((xDir == 1 && director.pos.x > game.width / 2 + 3) || (xDir == -1 && director.pos.x < game.width / 2 - 3))
			{
				spawn = Position(game.width - 1 - spawn.x, game.height - 1 - spawn.y);
				xDir *= -1;
			}
			directors.push_back(director);
			Position target = compute_target(game, director.pos, spawn, middle);
			game.register_action(new ActionMove(director.pos, target, 1));
			for (int w = director.pos.x; w >= 0 && w < game.width; w -= xDir)
			{
				int usable = game.get_case(w, h).owner == PLAYER_ME ? game.get_case(w, h).units : 0;
				Position origin = Position(w, h);
				if (w == director.pos.x)
				{
					usable -= 1;
				}
				if (usable > 0)
				{
					Position botTarget = target;
					if (count_bot_from(game, origin, teritory, -1) < count_bot_from(game, origin, teritory, 1))
					{
						if (count_bot_from(game, origin, teritory, -1) <= count_line(game, origin, -1))
						{
							botTarget = Position(origin.x, 0);
						}
					}
					else
					{
						if (count_bot_from(game, origin, teritory, 1) <= count_line(game, origin, 1))
						{
							botTarget = Position(origin.x, game.height - 1);
						}
					}
					if (botTarget.x < 0 || botTarget.x >= game.width || botTarget.y < 0 || botTarget.y >= game.height || botTarget == origin)
						botTarget = target;
					game.register_action(new ActionMove(origin, botTarget, usable));
				}
			}
		}
	}
	if (directors.size() >= 3 && game.my_matter >= 10)
	{
		int dist = directors.front().pos.distance(middle);
		Position target = directors.front().pos;
		for (auto it = directors.begin(); it != directors.end(); it++)
		{
			if (it->pos.distance(middle) < dist)
			{
				dist = it->pos.distance(middle);
				target = it->pos;
			}
		}
		game.register_action(new ActionSpawn(target, 1));
	}
	for (auto it = front_line.begin(); it != front_line.end(); it++)
	{
		if (game.my_matter < 10)
			break;
		if (it->units == 0)
		{
			if (it->pos.distance(get_nearest(it->pos, teritory.my_bots)) >= 2)
			{
				game.register_action(new ActionSpawn(it->pos, 1));
			}
		}
	}
	for (auto it = teritory.my_bots.begin(); it != teritory.my_bots.end(); it++)
	{
		if (game.my_matter < 10)
			break;
		for (auto it2 = front_line.begin(); it2 != front_line.end(); it2++)
		{
			if (game.my_matter < 10)
				break;
			if (it->pos == it2->pos)
			{
				if (it2->units == 1)
					game.register_action(new ActionSpawn(it->pos, 1));
			}
		}
	}
}

void splatoon(Game &game, Teritory &teritory)
{
	vector<Case> notMine;
	for (auto it = teritory.cases.begin(); it != teritory.cases.end(); it++)
	{
		if (it->owner != PLAYER_ME)
		{
			notMine.push_back(*it);
		}
	}
	if (teritory.my_bots.size() == 0)
	{
		for (auto it = teritory.cases.begin(); it != teritory.cases.end(); it++)
		{
			if (it->owner == PLAYER_ME && it->scrap_amount > 0 && it->recycler <= 0)
			{
				game.register_action(new ActionSpawn(it->pos, 1));
			}
		}
	}
	vector<Bot> available = teritory.my_bots;
	for (auto it = notMine.begin(); it != notMine.end(); it++)
	{
		Position bot = get_nearest(it->pos, available);
		game.register_action(new ActionMove(bot, it->pos, 1));
		for (auto it2 = available.begin(); it2 != available.end(); it2++)
		{
			if (it2->pos == bot)
			{
				available.erase(it2);
				break;
			}
		}
	}
}

/*=======================================================================
||                                                                     ||
||                           Main Function                             ||
||                                                                     ||
=======================================================================*/

int main()
{
	int width;
	int height;
	Mode mode = EXPAND;
	cin >> width >> height;
	cin.ignore();
	if (INPUT_DEBUG)
	{
		cerr << "Readed initialization inputs" << endl;
	}

	Game game(width, height);

	game.read_inputs();

	Position myBase;
	for (auto it = game.cases.begin(); it != game.cases.end(); it++)
	{
		if (it->owner == PLAYER_ME)
		{
			myBase = it->pos;
			break;
		}
	}

	int xDir = myBase.x > game.width / 2 ? -1 : 1;
	int yDir = myBase.y > game.height / 2 ? -1 : 1;
	Position spawn = Position(myBase.x, myBase.y);
	Position middle = Position(game.width / 2 - xDir, game.height / 2 - yDir);

	vector<Teritory> teritories = game.get_teritories();
	expand(game, teritories[0], spawn, middle, xDir);

	game.execute_actions();

	int turn = 1;
	while (++turn)
	{
		game.read_inputs();

		for (auto it = game.opp_bots.begin(); it != game.opp_bots.end(); it++)
		{
			if (game.my_matter < 10)
				break;
			Position target = Position(it->pos.x - xDir, it->pos.y);
			if (is_available_for_defend(game.get_case(target)))
				game.register_action(new ActionBuildRecycler(target));
		}
		for (auto it = game.opp_bots.begin(); it != game.opp_bots.end(); it++)
		{
			if (game.my_matter < 10)
				break;
			Position target = Position(it->pos.x, it->pos.y + 1);
			if (is_available_for_defend(game.get_case(target)))
				game.register_action(new ActionBuildRecycler(target));
			if (game.my_matter < 10)
				break;
			target = Position(it->pos.x, it->pos.y - 1);
			if (is_available_for_defend(game.get_case(target)))
				game.register_action(new ActionBuildRecycler(target));
		}

		vector<Teritory> teritories = game.get_teritories();
		for (auto it = teritories.begin(); it != teritories.end(); it++)
		{
			if (!it->isIsolateWithCase())
			{
				expand(game, *it, spawn, middle, xDir);
			}
			else if (it->owner == PLAYER_ME)
			{
				splatoon(game, *it);
			}
		}
		game.execute_actions();
	}
}
