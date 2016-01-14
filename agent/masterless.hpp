#ifndef MUTILATE_MASTERLESS_HPP
#define MUTILATE_MASTERLESS_HPP

#include <cstring>
#include <vector>

#include "../config.h"
#include "../cmdline.h"
#include "../mutilate.h"
#include "../log.h"
#include "../ConnectionOptions.h"
#include "../ConnectionStats.h"


using std::vector;
using std::string;

class MasterlessAgent {
public:
  MasterlessAgent();

  void run(gengetopt_args_info _args, options_t _options);

private:
  void _run(const vector<string>& _servers, gengetopt_args_info _args,
            options_t& _options, ConnectionStats& _stats);

  pthread_barrier_t barrier;
};

#endif //MUTILATE_MASTERLESS_HPP
