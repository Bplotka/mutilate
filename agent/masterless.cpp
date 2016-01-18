#include "masterless.hpp"

MasterlessAgent::MasterlessAgent() {
  V("Starting Masterless Agent");
}

/**
 * Refactored & improved agent() function.
 */
void MasterlessAgent::run(gengetopt_args_info _args, options_t _options) {
  while (true) {
    //*((int *) num.data()) = _args.threads_arg * _args.lambda_mul_arg;

    // Save addresses to memcached.
    // NOTE(bplotka): Initially that info was sent from master.
    vector<string> memcached_servers;
    for (int i = 0; i < _options.server_given; i++) {
      memcached_servers.push_back(name_to_ipaddr(string(_args.server_arg[i])));
    }

    for (auto i : memcached_servers) {
      V("Got Memcached Server = %s", i.c_str());
    }

    _options.threads = _args.threads_arg;

    // Lamda denom was initially sent by master.
    // In masterless mode we need to configure it manually.
    _options.lambda_denom = _args.connections_arg * _options.threads;

    //?? V("AGENT SLEEPS"); sleep(1);
    _options.lambda =
      (double) _options.qps / _options.lambda_denom * _args.lambda_mul_arg;

    V("lambda_denom = %d, lambda = %f, qps = %d",
      _options.lambda_denom, _options.lambda, _options.qps);

    //??    if (options.threads > 1)
    pthread_barrier_init(&barrier, NULL, _options.threads);

    ConnectionStats stats;
    _run(memcached_servers, _args, _options, stats);

    AgentStats as;
    as.rx_bytes = stats.rx_bytes;
    as.tx_bytes = stats.tx_bytes;
    as.gets = stats.gets;
    as.sets = stats.sets;
    as.get_misses = stats.get_misses;
    as.start = stats.start;
    as.stop = stats.stop;
    as.skips = stats.skips;

    V("Finished loop");

//    V("Local QPS = %.1f (%d / %.1fs)",
//      total / (stats.stop - stats.start),
//      total, stats.stop - stats.start);
  }
}


void* _mainThread(void *arg) {
  struct thread_data* threadData = (struct thread_data *) arg;

  ConnectionStats *connectionStats = new ConnectionStats();

  V("Thread: " + threadData->id + " starting mutilating");
  // Use global func from mutilate.cc (!)
  do_mutilate(*threadData->servers, *threadData->options,
              *connectionStats, threadData->master
#ifdef HAVE_LIBZMQ
  , NULL
#endif
   );

  return connectionStats;
}


void MasterlessAgent::_run(
  const vector<string>& _servers, gengetopt_args_info _args,
  options_t& _options, ConnectionStats& _stats) {
  if (_options.threads > 1) {
    pthread_t threads[_options.threads];
    struct thread_data threadData[_options.threads];
    vector<string> threadServer[_options.threads];

#ifdef __linux__
    int current_cpu = -1;
#endif

    // Fill thread structs.
    for (int t = 0; t < _options.threads; t++) {
      threadData[t].options = &_options;
      threadData[t].id = t;

      if (t == 0) threadData[t].master = true;
      else threadData[t].master = false;

      if (_options.roundrobin) {
        for (unsigned int i = (t % _servers.size());
             i < _servers.size();
             i += _options.threads) {
          threadServer[t].push_back(_servers[i % _servers.size()]);
        }

        threadData[t].servers = &threadServer[t];
      } else {
        threadData[t].servers = &_servers;
      }

      pthread_attr_t attr;
      pthread_attr_init(&attr);

#ifdef __linux__
      if (args.affinity_given) {
        int max_cpus = 8 * sizeof(cpu_set_t);
        cpu_set_t m;
        CPU_ZERO(&m);
        sched_getaffinity(0, sizeof(cpu_set_t), &m);

        for (int i = 0; i < max_cpus; i++) {
          int c = (current_cpu + i + 1) % max_cpus;
          if (CPU_ISSET(c, &m)) {
            CPU_ZERO(&m);
            CPU_SET(c, &m);
            int ret;
            if ((ret = pthread_attr_setaffinity_np(&attr,
                                                   sizeof(cpu_set_t), &m)))
              DIE("pthread_attr_setaffinity_np(%d) failed: %s",
                  c, strerror(ret));
            current_cpu = c;
            break;
          }
        }
      }
#endif

      if (pthread_create(&threads[t], &attr, _mainThread, &threadData[t]))
        DIE("pthread_create() failed");
    }

    for (int t = 0; t < _options.threads; t++) {
      ConnectionStats *localStats;
      if (pthread_join(threads[t], (void**) &localStats)) DIE("pthread_join() "
                                                          "failed");
      _stats.accumulate(*localStats);
      delete localStats;
    }
  } else if (_options.threads == 1) {
    do_mutilate(_servers, _options, _stats, true
#ifdef HAVE_LIBZMQ
      , NULL
#endif
    );
  }
}
