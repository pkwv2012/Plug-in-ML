// This is a test for partition

#include "stdio.h"
#include "src/agent/partition.h"

using namespace rpscc;
using namespace std;

int main() {
  Partition p;
  int key_range = 100;
  int server_num = 10;
  vector<int> part_vec(11, 0);
  for (int i = 1; i <= 10; i++)
    part_vec[i] = i * 10;
  p.Initialize(key_range, server_num, part_vec);
  printf("%d\n", p.GetServerByKey(0));
  printf("%d\n", p.GetServerByKey(1));
  printf("%d\n", p.GetServerByKey(10));
  printf("%d\n", p.GetServerByKey(100));
  printf("%d\n", p.GetServerByKey(23));
  printf("%d\n", p.GetServerByKey(-1));
  printf("%d\n", p.GetServerByKey(110));
  return 0;
}
