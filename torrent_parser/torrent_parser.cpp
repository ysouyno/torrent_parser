// torrent_parser.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <fstream>
#include "torrent_parser.h"
#include "percent_encode.h"
#include <chrono>
#include <Windows.h>
#include "torrent_info.h"
#include "bencode_value_safe_cast.h"

// just temp test
int area(int length, int width)
{
  return length * width;
}

int frame_area(int x, int y)
{
  return area(x - 2, y - 2);
}

void printf_reverse(const char *str, char *p, size_t len)
{
  if (p - str != len)
  {
    printf_reverse(str, p + 1, len);
  }

  printf("%c", *p);
}

int main()
{
  std::string path_file("D:\\dnld\\xxx.torrent");
  std::ifstream file(path_file, std::ios::binary);

  torrent_parser tp(file);
  std::shared_ptr<bencode_member> sp_bm = tp.get_value();
  // tp.print_all();

  bencode_to_map aa(sp_bm);
  aa.print();
  aa.print_multimap();

  std::string hash = "\x12\x34\x56\x78\x9a";
  std::string result = percentEncode(hash);
  std::cout << result << std::endl;

  char a = '1';
  printf("%d\n", isdigit(a));

  int x = -1;
  int y = 2;
  int z = 4;
  int area1 = area(x, y);
  int area2 = frame_area(1, z);
  int area3 = frame_area(y, z);
  double ratio = double(area1) / area3;

  printf("%d %d %d %f\n", area1, area2, area3, ratio);

  std::chrono::seconds(5);
  std::cout << "chrono" << std::endl;

  bencode_reader br(sp_bm);
  br.get_announce();

  bencode_encoder be(sp_bm);
  be.crawl_all();
  be.print_all();

  // temp test
  DWORD t1 = (DWORD)(1 << 24);
  std::cout << std::hex << t1 << std::endl;

  char smh[] = "abcdefgh";
  char *start, *end;
  start = smh;
  end = smh + sizeof(smh) - 2;

  while (start < end)
  {
    char temp = '0';

    temp = *start;
    *start = *end;
    *end = temp;

    start++;
    end--;
  }

  std::cout << smh << std::endl;

  char recursive[] = "abcdefghijklmn";
  char *p_r = recursive;
  printf_reverse(recursive, p_r, sizeof(recursive) - 2);

  std::cout << std::endl;

  auto ti = std::make_shared<torrent_info>();
  get_announce(ti.get(), dynamic_cast<bencode_dict *>(sp_bm.get()));
  long long piece_length = get_piece_length(ti.get(), dynamic_cast<bencode_dict *>(sp_bm.get()));
  printf("piece length: %lld\n", piece_length);
  long long creation_date = get_creation_date(ti.get(), bencode_cast<bencode_dict>(sp_bm.get()));
  printf("creation date: %lld\n", creation_date);

  return 0;
}

