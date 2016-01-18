#ifndef MUTILATE_H
#define MUTILATE_H

#include <string>
#include <vector>

#include "cmdline.h"
#include "ConnectionOptions.h"

#ifdef HAVE_LIBZMQ
#include <zmq.hpp>
#endif

#define USE_CACHED_TIME 0
#define MINIMUM_KEY_LENGTH 2
#define MAXIMUM_CONNECTIONS 512

#define MAX_SAMPLES 100000

#define LOADER_CHUNK 1024

using std::string;
using std::vector;

extern char random_char[];
extern gengetopt_args_info args;

string name_to_ipaddr(string host);

struct thread_data {
  const vector<string> *servers;
  options_t *options;
  bool master;  // Thread #0, not to be confused with agent master.
#ifdef HAVE_LIBZMQ
  zmq::socket_t *socket;
#endif
  int id;
};

class ConnectionStats;

void do_mutilate(const vector<string>& servers, options_t& options,
                 ConnectionStats& stats, bool master
#ifdef HAVE_LIBZMQ
, zmq::socket_t* socket
#endif
  , int thread_id = 0);

#endif // MUTILATE_H
