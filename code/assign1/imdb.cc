#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
#include <string.h>
#include <algorithm>
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
  int *firstOffset = (int*) actorFile + 1;
  //399788
  int *lastOffset = firstOffset + number;
  int *indexOfPlayer = lower_bound(firstOffset,lastOffset,player,[&](const int a,const string& b) -> bool{
    return getPlayer(a).compare(b) < 0;
  });
  char *pointer = (char *) actorFile + *indexOfPlayer;
  int dolzina = strlen(pointer);
  cout << "name is " << pointer << endl;
  int offset = strlen(pointer) + 1;
  if(strlen(pointer) % 2==0){
    offset+=1;
  }
  pointer+=offset;
  short numberMovies = *(short *) pointer;
  // cout << "number of of movies " << numberMovies <<  endl;
  if((offset+2) % 4 !=0){
    pointer+=2;
  }
  pointer+=2;

  int *moviesOffset = (int *) pointer;

  for(int i=0;i<numberMovies;i++){
      film temp = getFilm(*moviesOffset);
      moviesOffset+=1;
      films.push_back(temp);
  }
  return true;
}
const film imdb::getFilm(int offset) const{
  char *pointer = (char *) movieFile + offset;
  string title = string(pointer);
  pointer += strlen(pointer) + 1;
  char year = *pointer;
  return film{title,year};
}
 const string imdb::getPlayer(int offset) const{
   return string((char * ) actorFile + offset);
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
