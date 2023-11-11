#include <algorithm>
#include "utils.h"

using namespace std;

string utils::toUpper(string s)
{
  string r(s.length(), 0);
  transform(s.begin(), s.end(), r.begin(), ::toupper);
  return r;
}
