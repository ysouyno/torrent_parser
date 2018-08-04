#pragma once

#include <iostream>
#include <string>
#include <cassert>
#include "torrent_parser.h"

// need this empty class or compiler error
// error C2259: 'bencode_value_safe_cast<TO>' : cannot instantiate abstract class
class empty_bencode_crawler : public bencode_crawler
{
public:
  virtual void crawl(bencode_string *p) {}
  virtual void crawl(bencode_number *p) {}
  virtual void crawl(bencode_list *p) {}
  virtual void crawl(bencode_dict *p) {}
};

template <typename T>
class bencode_value_safe_cast : public empty_bencode_crawler
{
public:
  virtual void crawl(T *t)
  {
    // the type of parameter 't' is a pointer pointed to 'this', so it will be bencode_dict object
    // and the type of return value 't_' is bvsc's TO type, it is bencode_dict type
    // so here just assignment, it is safety, better than dynamic_cast
    t_ = t;
  }

  T *get_cast_result()
  {
    return t_;
  }

private:
  T * t_;
};

template <typename TO, typename FROM>
TO *bencode_cast(FROM *from)
{
  std::cout << "bencode_cast called" << std::endl;
  assert(from != nullptr);
  bencode_value_safe_cast<TO> bvsc;

  // because the parameter 'from' is the abstract class pointer type so visit function will call it's derived class's function
  // here the 'from' pointer pointed to bencode_dict, it will invoke bencode_dict's visit funciton, and
  // because parameter 'bvsc' is a derived class inherited from bencode_crawler pointer also, so when call dict's
  // visit funciton, it will call bencode_value_safe_cast's crawl function, and in that function
  // the parameter type is this pointer that is bencode_dict class type, see above to continue understand
  from->visit(&bvsc);
  return bvsc.get_cast_result();
}
