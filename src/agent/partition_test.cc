// This is a test for partition

#include "stdio.h"
#include "src/agent/partition.h"
#include <iostream>

using namespace rpscc;
using namespace std;

int main() {
  Partition p;
  int key_range = 100;
  int server_num = 10;
  vector<int> part_vec(10, 0);
  for (int i = 0; i < 10; i++)
    part_vec[i] = i * 11;
  p.Initialize(key_range, server_num, part_vec);
  cout << "Partition: ";
  for (int i = 0; i < 10; i++) {
    cout << part_vec[i] << " ";
  }
  cout << endl;
  printf("%d\n", p.GetServerByKey(0));
  printf("%d\n", p.GetServerByKey(1));
  printf("%d\n", p.GetServerByKey(10));
  printf("%d\n", p.GetServerByKey(99));
  printf("%d\n", p.GetServerByKey(18));
  printf("%d\n", p.GetServerByKey(98));
  printf("%d\n", p.GetServerByKey(99));

  vector<int> keys(16, 0);
  cout << "Keys: ";
  for (int i = 0; i < 16; i++) {
    keys[i] = i * 6;
    cout << keys[i] << " ";
  }
  cout << endl;

  int start = 0, end, server_id;
  while (start < 16) {
    end = p.NextEnding(keys, start, server_id);
    cout << "start, end = " << start << ", " << end << endl;
    cout << "server_id = " << server_id << endl;
    start = end;
  }

  return 0;
}
