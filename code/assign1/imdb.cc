#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include "imdb.h"
using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

// Constructor
imdb::imdb(const string& directory) {
	const string actorFileName = directory + "/" + kActorFileName;
	const string movieFileName = directory + "/" + kMovieFileName;
	actorFile = acquireFileMap(actorFileName, actorInfo);
	movieFile = acquireFileMap(movieFileName, movieInfo);
}

// Destructor
imdb::~imdb() {
	releaseFileMap(actorInfo);
	releaseFileMap(movieInfo);
}

bool imdb::good() const {
	return !( (actorInfo.fd == -1) ||
			(movieInfo.fd == -1) );
}
/*
 *
	 * Method: getCredits
	 * ------------------
	 * Searches for an actor/actress's list of movie credits.  The list
	 * of credits is returned via the second argument, which you'll note
	 * is a non-const vector<film> reference.  If the specified actor/actress
	 * isn't in the database, then the films vector will be left empty.
	 *
	 * @param player the name of the actor or actresses being queried.
	 * @param films a reference to the vector of films that should be updated
	 *              with the list of the specified actor/actress's credits.
	 * @return true if and only if the specified actor/actress appeared in the
	 *              database, and false otherwise.
	 */
bool imdb::getCredits(const string& player, vector<film>& films) const {
	int actorLength = *(int *)actorFile;
	char* beginning = (char *)actorFile + sizeof(int);
	char* end = (char *)(beginning + (actorLength * sizeof(int)));
	/* for(int *i = (int *)beginning; i != (int *)end; i++) { */
	/* 	printf("%d\t%s\n", *i, (char *)actorFile + *i); */
	/* } */

	auto actorPos = lower_bound((int *)beginning, (int *)end, player, [&actorFile = actorFile](int offset, const string& value){
		// get actor name at offset `offset`
		char* actorPointer = (char *)actorFile + offset;
		return strcmp(actorPointer, value.c_str()) < 0;
	});

	char * actorRecord = (char *)actorFile + *actorPos;
	if(actorPos != (int *)end && strcmp(actorRecord, player.c_str()) != 0) {
		// 1. binary search actor in actorFile;
			// if not found set films size to 0
			// return false;
		cout << "Actor " << player << " not found" << endl;
		films.clear();
		return false;
	}
	else {
		cout << "Actor found at position " << (actorPos - (int *)actorFile) << endl;
		// printf("ACTOR: %s\n", (char *)actorPos);
	}

	// 2. offsets = Given actor offset, get list of offsets into movies
	// 3. getFilmsFromOffsets(offsets)
	return true;
}
/*
	 * Searches the receiving imdb for the specified film and returns the cast
	 * by populating the specified vector<string> with the list of actors and actresses
	 * who star in it.  If the movie doesn't exist in the database, the players vector
	 * is cleared and its size left at 0.
 */
bool imdb::getCast(const film& movie, vector<string>& players) const {
	// 1. binary search movie in movieFile;
		// if not found set players size to 0
		// return false;
	// 2. offsets = Given movie offset, get list of offsets into actors
	// 3. getActorsFromOffsets(offsets)
	return true;
}

/*
 * getFilmsFromOffsets():
 *
 * Given a list of offsets, return vector of film structs;
 */

/*
 * getActorsFromOffsets();
 *
 * Given a list of offsets, return vector of actors.
 */

const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
	struct stat stats;
	stat(fileName.c_str(), &stats);
	info.fileSize = stats.st_size;
	info.fd = open(fileName.c_str(), O_RDONLY);
	return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
	if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
	if (info.fd != -1) close(info.fd);
}
