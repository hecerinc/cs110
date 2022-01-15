#include <iostream>
#include <vector>
#include <queue>
#include <iomanip>
#include <set>
#include <string>

#include "imdb.h"
#include "path.h"
#include "imdb-utils.h"

using namespace std;

static const int kWrongArgumentCount = 1;
static const int kDatabaseNotFound = 2;

/*
You need to implement a breadth-first search algorithm that enlists your imdb class to find the shortest path connecting any two actors or actresses. If the search goes on for so long that you can tell it’ll be of length 7 or more, then you can be reasonably confident (and pretend that you know for sure that) there’s no path connecting them. This part of the assignment is more CS106B-like, and it’s a chance to get some experience with the STL (using vectors, sets, and lists) and to see a legitimate scenario where a complex program benefits from coding in two different paradigms: high-level, object-oriented C++ (with its STL template containers and template algorithms) and low-level, imperative C (with its exposed memory, courtesy of CS107, *, &, [], and ->).
*/
static const int kMaxHops = 7;
struct node {
	string * player;
	unsigned int distance;
	node * parent;
	film parent_movie;
	node (string * p, unsigned int d, node * pp, film &pm): player(p), distance(d), parent(pp), parent_movie(pm) {
	}
};

struct node *bfs(imdb& db, string& start, string& finish) {
	// s = root;
	path p(start);

	// Auxiliary data structures
	set<string> visited_actors;
	set<film> visited_films;

	queue<node*> q; // a queue of actors left to visit

	/*
	for (vertex u in g.vertex - {s}) {
		u.color = white;
		u.d = infinite;
		u.parent = NULL;
	}
	*/

	visited_actors.insert(start);
	film fa;
	node *root  = new node(&start, 0, NULL, fa);

	q.push(root);

	while(!q.empty()) { // for each actor
		node* u = q.front();
		q.pop();
		/* cout << "Processing actor: " << *(u->player) << endl; */

		vector<film> films;
		db.getCredits(*(u->player), films);

		for(film& f: films) {
			/* cout << "Film: " << f.title << endl; */
			vector<string> players;
			db.getCast(f, players);

			for(string& v: players) {

				/* cout << "ACTOR: " << v << endl; */
				if(visited_actors.find(v) == visited_actors.end()) { // have not visited
					visited_actors.insert(v);
					int distance = u->distance + 1;
					// cout << "distance: " << distance  << endl;
					if(distance > kMaxHops) {
						return NULL; // Early termination
					}
					string *name = new string(v);
					node* vnode = new node(name, distance, u, f);
					if(v == finish) {
						return vnode;
					}
					/* cout << "  Adding " << v << " to queue" << endl; */
					q.push(vnode);
				}
				else {
					/* cout << "Already visited " << v << endl; */
				}
			}
		}


		// u.color = black;
	}
	return NULL;
}
void print_bfs(node * result) {
	if (result->parent == NULL) {
		return;
		/* cout << " -> "; */
	}
	print_bfs(result->parent);
	cout << *(result->parent->player) << " was in " << std::quoted(result->parent_movie.title) << " (" << result->parent_movie.year <<  ") with " << *(result->player) << "." << endl;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cerr << "Usage: " << argv[0] << " <actor1> <actor2>" << endl;
		return kWrongArgumentCount;
	}

	imdb db(kIMDBDataDirectory);
	if (!db.good()) {
		cerr << "Data directory not found!  Aborting..." << endl;
		return kDatabaseNotFound;
	}

	string player1 = argv[1];
	string player2 = argv[2];


	node * result = bfs(db, player1, player2);
	if(result == NULL) {
		cout << "No path found" << endl;
	}
	else {
		print_bfs(result);
		cout << endl;
	}

	// listAllMoviesAndCostars(db, player);
	return 0;
}
