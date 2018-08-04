#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include "torrent_parser.h"
#include "bencode_value_safe_cast.h"

struct torrent_info
{
  std::vector<std::string> announce_vec_;
  std::string comment_;
  std::string info_hash_;
  long long creation_date_;
  long long piece_length_;
};

void get_announce(torrent_info *ti, bencode_dict *root_dict)
{
  bencode_member *bm_announce = root_dict->get("announce-list");
  if (bm_announce != nullptr)
  {
    bencode_list *announce_list = dynamic_cast<bencode_list *>(bm_announce);
    assert(announce_list != nullptr);

    for (auto &list : *announce_list)
    {
      bencode_list *bl = dynamic_cast<bencode_list *>(list.get());
      assert(bl != nullptr);

      for (auto &elem : *bl)
      {
        bencode_string *bs = dynamic_cast<bencode_string *>(elem.get());
        assert(bs != nullptr);
        std::cout << bs->get_value() << std::endl;
        ti->announce_vec_.push_back(bs->get_value());
      }
    }
  }
  else
  {
    // just have one main announce
    bm_announce = root_dict->get("announce");
    bencode_string *announce = dynamic_cast<bencode_string *>(bm_announce);
    assert(announce != nullptr);
    std::cout << announce->get_value() << std::endl;
    ti->announce_vec_.push_back(announce->get_value());
  }
}

long long get_piece_length(torrent_info *ti, bencode_dict *root_dict)
{
  bencode_member *bm = root_dict->get("info");
  assert(bm != nullptr);
  bencode_dict *info_dict = dynamic_cast<bencode_dict *>(bm);
  assert(info_dict != nullptr);

  bencode_number *bn = dynamic_cast<bencode_number *>(info_dict->get("piece length"));
  assert(bn != nullptr);

  return bn->get_value();
}

long long get_creation_date(torrent_info *ti, bencode_dict *root_dict)
{
  bencode_member *bm = root_dict->get("creation date");
  assert(bm != nullptr);
  bencode_number *bn = bencode_cast<bencode_number>(bm);
  assert(bn != nullptr);

  return bn->get_value();
}
