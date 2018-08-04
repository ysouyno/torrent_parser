#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <vector>
#include <map>
#include <sstream>

class bencode_string;
class bencode_number;
class bencode_list;
class bencode_dict;
class bencode_reader;

class bencode_crawler
{
public:
  virtual void crawl(bencode_string *p) = 0;
  virtual void crawl(bencode_number *p) = 0;
  virtual void crawl(bencode_list *p) = 0;
  virtual void crawl(bencode_dict *p) = 0;
};

class bencode_member
{
public:
  virtual void print_member() = 0;
  virtual void visit(bencode_crawler *p) = 0;
};

class bencode_string : public bencode_member
{
public:
  bencode_string(const std::string &str) : value_(str)
  {}

  void print_member()
  {
    std::cout << value_;
  }

  void visit(bencode_crawler *p) override
  {
    p->crawl(this);
  }

  const std::string &get_value() const
  {
    return value_;
  }

private:
  std::string value_;
};

class bencode_number : public bencode_member
{
public:
  bencode_number(size_t value) : value_(value)
  {}

  void print_member()
  {
    std::cout << value_;
  }

  void visit(bencode_crawler *p) override
  {
    p->crawl(this);
  }

  size_t get_value() const
  {
    return value_;
  }

private:
  size_t value_;
};

class bencode_list : public bencode_member
{
public:
  void print_member()
  {
    for (std::vector<std::shared_ptr<bencode_member> >::iterator it = value_.begin(); it != value_.end(); ++it)
    {
      (*it)->print_member();
    }
  }

  void visit(bencode_crawler *p) override
  {
    p->crawl(this);
  }

  void insert_to_list(std::shared_ptr<bencode_member> value)
  {
    value_.push_back(value);
  }

  const std::vector<std::shared_ptr<bencode_member> > &get_value()
  {
    return value_;
  }

  std::vector<std::shared_ptr<bencode_member> >::iterator begin()
  {
    return value_.begin();
  }

  std::vector<std::shared_ptr<bencode_member> >::iterator end()
  {
    return value_.end();
  }

private:
  std::vector<std::shared_ptr<bencode_member> > value_;
};

class bencode_dict : public bencode_member
{
public:
  void print_member()
  {
    for (std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator it = value_.begin();
      it != value_.end(); ++it)
    {
      std::cout << it->first;
      std::cout << ": ";
      it->second.get()->print_member();
      std::cout << std::endl;
    }
  }

  void visit(bencode_crawler *p) override
  {
    p->crawl(this);
  }

  void insert_to_dict(std::string key, std::shared_ptr<bencode_member> value)
  {
    value_.insert(std::make_pair(key, value));
  }

  const std::multimap<std::string, std::shared_ptr<bencode_member> > &get_value()
  {
    return value_;
  }

  bencode_member *get(const std::string &key)
  {
    std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator it = value_.find(key);
    if (it == std::end(value_))
    {
      return nullptr;
    }
    else
    {
      return it->second.get();
    }
  }

  std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator begin()
  {
    return value_.begin();
  }

  std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator end()
  {
    return value_.end();
  }

private:
  std::multimap<std::string, std::shared_ptr<bencode_member> > value_;
};

class torrent_parser
{
  friend bencode_reader;

public:
  torrent_parser() {}
  ~torrent_parser() {}

  torrent_parser(std::ifstream &in)
  {
    sp_bm_ = parse(in);
  }

  std::shared_ptr<bencode_member> parse(std::ifstream &in)
  {
    char ch = in.get();

    switch (ch)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
      in.putback(ch);
      size_t len = 0;
      in >> len;
      in.get();

      std::vector<char> v(len);
      in.read(v.data(), v.size());
      std::string str;
      for (std::vector<char>::iterator it = v.begin(); it != v.end(); ++it)
        str += *it;

      return std::make_shared<bencode_string>(str);
    }
    case 'i':
    {
      size_t i;
      in >> i;
      in.get();

      return std::make_shared<bencode_number>(i);
    }
    case 'l':
    {
      bencode_list *bl = new bencode_list();
      while (in.peek() != 'e')
      {
        std::shared_ptr<bencode_member> value = parse(in);
        bl->insert_to_list(value);
      }
      in.get();
      std::shared_ptr<bencode_member> sp_bm(bl);
      return sp_bm;
    }
    case 'd':
    {
      bencode_dict *bd = new bencode_dict();
      while (in.peek() != 'e')
      {
        // key is a string always
        std::shared_ptr<bencode_member> sp_bm_key = parse(in);
        std::string key = dynamic_cast<bencode_string *>(sp_bm_key.get())->get_value();
        std::shared_ptr<bencode_member> value = parse(in);
        bd->insert_to_dict(key, value);
      }
      in.get();
      std::shared_ptr<bencode_member> sp_bm(bd);
      return sp_bm;
    }
    default:
      return sp_bm_;
    }
  }

  void print_all()
  {
    sp_bm_.get()->print_member();
  }

  std::shared_ptr<bencode_member> get_value()
  {
    return sp_bm_;
  }

private:
  std::shared_ptr<bencode_member> sp_bm_;
};

class bencode_to_map : public bencode_crawler
{
public:
  bencode_to_map(std::shared_ptr<bencode_member> sp_bm) : sp_bm_(sp_bm)
  {
  }

  virtual void crawl(bencode_string *p)
  {
    bencode_str_ = p->get_value();
  }

  virtual void crawl(bencode_number *p)
  {
    bencode_str_ = std::to_string(p->get_value());
  }

  virtual void crawl(bencode_list *p)
  {
    std::vector<std::shared_ptr<bencode_member> > value = p->get_value();
    for (std::vector<std::shared_ptr<bencode_member> >::iterator it = value.begin(); it != value.end(); ++it)
    {
      std::string key = get_list_name();
      (*it)->visit(this);
      // comment: is there a dictionary in a list?
      mm_.insert(std::make_pair(key, bencode_str_));
    }
  }

  virtual void crawl(bencode_dict *p)
  {
    std::multimap<std::string, std::shared_ptr<bencode_member> > value = p->get_value();
    for (std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator it = value.begin();
      it != value.end(); ++it)
    {
      std::string key = it->first;
      // it->second.get()->visit(this);
      std::string value = "";

      bencode_dict *bd = dynamic_cast<bencode_dict *>(it->second.get());
      bencode_list *bl = dynamic_cast<bencode_list *>(it->second.get());
      if (bd == NULL && bl == NULL)
      {
        // neither dict nor list
        // string or integer
        it->second.get()->visit(this);
        value = bencode_str_;
        mm_.insert(std::make_pair(key, value));
      }
      else if (bd != NULL && bl == NULL)
      {
        // dict
        it->second.get()->visit(this);
        mm_.insert(std::make_pair(key, ""));
      }
      else if (bd == NULL && bl != NULL)
      {
        // list
        list_name_ = bencode_str_;
        it->second.get()->visit(this);
      }
    }
  }

  const std::string &get_list_name() const
  {
    return list_name_;
  }

  void print()
  {
    crawl_all();
  }

  void crawl_all()
  {
    sp_bm_->visit(this);
  }

  void print_multimap()
  {
    for (std::multimap<std::string, std::string>::iterator it = mm_.begin(); it != mm_.end(); ++it)
    {
      if (0 != _stricmp(it->first.c_str(), "pieces"))
      {
        std::cout << it->first << " : " << it->second << std::endl;
      }
      else
      {
        std::cout << it->first << " : (" << it->second.size() << ")" << std::endl;
      }
    }
  }

private:
  std::string bencode_str_;
  std::shared_ptr<bencode_member> sp_bm_;
  std::multimap<std::string, std::string> mm_;
  std::string list_name_;
};

class bencode_printer : public bencode_crawler
{
public:
  virtual void crawl(bencode_string *p)
  {
    bencode_str_ += p->get_value();
  }

  virtual void crawl(bencode_number *p)
  {
    bencode_str_ += std::to_string(p->get_value());
  }

  virtual void crawl(bencode_list *p)
  {
  }

  virtual void crawl(bencode_dict *p)
  {
    std::multimap<std::string, std::shared_ptr<bencode_member> > value = p->get_value();
    for (std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator it = value.begin(); it != value.end(); ++it)
    {
      it->second.get()->visit(this);
    }
  }

private:
  std::string bencode_str_;
};

class bencode_encoder : public bencode_crawler
{
public:
  bencode_encoder(std::shared_ptr<bencode_member> sp_bm) : sp_bm_(sp_bm)
  {
  }

  virtual void crawl(bencode_string *p)
  {
    stream_ += std::to_string(p->get_value().size());
    stream_ += ':';
    stream_ += p->get_value();
  }

  virtual void crawl(bencode_number *p)
  {
    stream_ += 'i';
    stream_ += std::to_string(p->get_value());
    stream_ += 'e';
  }

  virtual void crawl(bencode_list *p)
  {
    stream_ += 'l';
    for (const auto &e : *p)
    {
      e.get()->visit(this);
    }
    stream_ += 'e';
  }

  virtual void crawl(bencode_dict *p)
  {
    stream_ += 'd';
    for (const auto &e : *p)
    {
      auto key = e.first;
      stream_ += std::to_string(key.size());
      stream_ += ':';
      stream_ += key.c_str();
      e.second.get()->visit(this);
    }
    stream_ += 'e';
  }

  void crawl_all()
  {
    sp_bm_->visit(this);
  }

  void print_all()
  {
    std::cout << "\n" << stream_.size() << std::endl;
  }

private:
  std::string stream_;
  std::shared_ptr<bencode_member> sp_bm_;
};

class bencode_reader
{
public:
  bencode_reader(std::shared_ptr<bencode_member> sp_tp) : sp_tp_(sp_tp)
  {
  }

  std::string get_announce()
  {
    bencode_dict *bd = dynamic_cast<bencode_dict *>(sp_tp_.get());
    std::multimap<std::string, std::shared_ptr<bencode_member> > dict = bd->get_value();
    std::multimap<std::string, std::shared_ptr<bencode_member> >::iterator it = dict.find("announce");
    bencode_string *bs = dynamic_cast<bencode_string *>(it->second.get());
    std::cout << bs->get_value() << std::endl;
    return bs->get_value();
  }

private:
  std::shared_ptr<bencode_member> sp_tp_;
};
