#include "imdb.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
using namespace std;

const char* const imdb::kActorFileName = "actordata";
const char* const imdb::kMovieFileName = "moviedata";

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

bool imdb::good() const { return !((actorInfo.fd == -1) || (movieInfo.fd == -1)); }
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
	int actorLength = *(int*)actorFile;
	char* beginning = (char*)actorFile + sizeof(int);
	char* end = (char*)(beginning + (actorLength * sizeof(int)));
	/* for(int *i = (int *)beginning; i != (int *)end; i++) { */
	/* 	printf("%d\t%s\n", *i, (char *)actorFile + *i); */
	/* } */

	auto actorPos = lower_bound((int*)beginning, (int*)end, player, [this](int offset, const string& value) {
		// get actor name at offset `offset`
		char* actorPointer = (char*)actorFile + offset;
		return strcmp(actorPointer, value.c_str()) < 0;
	});

	char* actorRecord = (char*)actorFile + *actorPos;
	if (actorPos != (int*)end && strcmp(actorRecord, player.c_str()) != 0) {
		// 1. binary search actor in actorFile;
		// if not found set films size to 0
		// return false;
		// cout << "Actor " << player << " not found" << endl;
		films.clear();
		return false;
	}

	char* movieArray;
	short* numberOfMovies;
	const int len = strlen(actorRecord);
	int shifts = 0;

	if (len % 2 == 0) {
		numberOfMovies = (short*)(actorRecord + len + 2);
		shifts = len + 2 + sizeof(short);
	}
	else {
		numberOfMovies = (short*)(actorRecord + len + 1);
		shifts = len + 1 + sizeof(short);
	}
	movieArray = actorRecord + shifts;
	if (shifts % 4 != 0) {
		movieArray += 2;
	}

	// cout << "Number of movies: " << *numberOfMovies << endl;
	// Generate the film structs
	for (int *i = (int*)movieArray, j = 0; j < *numberOfMovies; j++) {
		const char* title = (char*)movieFile + *i;
		const int len = strlen(title);
		const char* yearPtr = title + len + 1;
		int year = 1900 + static_cast<int>(*yearPtr);

		struct film f = {title, year};
		// cout << f.year << "\t" << f.title << endl;
		i++;
		films.push_back(f);
	}

	return true;
}

struct film getFilmFromOffset(const void* movieFile, int offset) {
	const char* title = (char*)movieFile + offset;
	const int len = strlen(title);
	const char* yearPtr = title + len + 1;
	int year = 1900 + static_cast<int>(*yearPtr);

	struct film f = {title, year};
	return f;
}
/*
 * Searches the receiving imdb for the specified film and returns the cast
 * by populating the specified vector<string> with the list of actors and actresses
 * who star in it.  If the movie doesn't exist in the database, the players vector
 * is cleared and its size left at 0.
 */
bool imdb::getCast(const film& movie, vector<string>& players) const {
	int moviesLength = *(int*)movieFile;
	char* beginning = (char*)movieFile + sizeof(int);
	char* end = (char*)(beginning + (moviesLength * sizeof(int)));
	auto moviePos =
		lower_bound((int*)beginning, (int*)end, movie, [& movieFile = movieFile](int offset, const film& value) {
			// get movie name at offset `offset`

			struct film f = getFilmFromOffset(movieFile, offset);
			return f < value;
		});

	const film f = getFilmFromOffset(movieFile, *moviePos);
	if (moviePos != (int*)end && !(movie == f)) {
		// 1. binary search movie in movieFile;
		// if not found set films size to 0
		// return false;
		/* cout << "Movie " << movie.title << " not found" << endl; */
		players.clear();
		return false;
	}

	// Populate the actors array
	const char* movieRecord = (char*)movieFile + *moviePos;
	const int titlePlusYearLen = movie.title.length() + /* null terminator */ 1 + /* one byte for year */ 1;
	int shifts = titlePlusYearLen;

	char* actorNumberPtr = (char*)movieRecord + titlePlusYearLen;

	if (titlePlusYearLen % 2 != 0) {
		// If it's odd, an extra NUL character sits between the year and the data that follows
		actorNumberPtr++;
		shifts++;
	}

	shifts += sizeof(short);
	short numberOfActors = *((short*)actorNumberPtr);

	// cout << numberOfActors << endl;
	// cout << "Number of actors: " << numberOfActors << endl;
	char* actorArray = (char*)actorNumberPtr + sizeof(short);

	if (shifts % 4 != 0) {
		actorArray += 2;
	}

	for (int *i = (int*)actorArray, j = 0; j < numberOfActors; j++) {
		const char* actorPtr = (char*)actorFile + *i;
		players.push_back(string(actorPtr));
		i++;
	}
	return true;
}

const void* imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
	struct stat stats;
	stat(fileName.c_str(), &stats);
	info.fileSize = stats.st_size;
	info.fd = open(fileName.c_str(), O_RDONLY);
	return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
	if (info.fileMap != NULL) munmap((char*)info.fileMap, info.fileSize);
	if (info.fd != -1) close(info.fd);
}
