#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <string.h>
using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
imdb::imdb(const string& directory) {
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const {
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

imdb::~imdb() {
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

bool imdb::getCredits(const string& player, vector<film>& films) const { 
  const int number = *(int*) actorFile;
  int *firstOffset = (int*) actorFile;
  int *lastOffset = firstOffset + number;
  firstOffset = lastOffset;
  char *name = (char *) actorFile + *firstOffset;
  char *pointer = name;
  cout << "i am the number " << number << endl;
  cout << "first offset " << name << endl;
  cout << "the length is : " << strlen(name) << endl;

  int offset = 0;
  pointer+=strlen(name);
  if(strlen(name) %2 ==0){
    offset = 2;
  }else{
    offset = 1;
  }
  pointer+=offset;
  short *numberMovies = (short * ) pointer;
  cout << "number of movies in " << *numberMovies << endl;



}

bool imdb::getCast(const film& movie, vector<string>& players) const { 
  return false; 
}

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
