# perf script event handlers, generated by perf script -g python
# Licensed under the terms of the GNU GPL License version 2

# The common_* event handler fields are the most useful fields common to
# all events.  They don't necessarily correspond to the 'common_*' fields
# in the format files.  Those fields not available as handler params can
# be retrieved using Python functions of the form common_*(context).
# See the perf-script-python Documentation for the list of available functions.

from __future__ import print_function

import os
import sys
import matplotlib.pyplot as plt
import numpy as np

sys.path.append(os.environ['PERF_EXEC_PATH'] + \
    '/scripts/python/Perf-Trace-Util/lib/Perf/Trace')

from perf_trace_context import *
from Core import *

start = 0
times = []

def trace_begin():
    pass

def trace_end():
    global times
    times = np.array(times)
    plt.hist(times, bins=np.arange(400,1200,10))
    print(np.mean(times))
    print(np.std(times))
    plt.show()


def probe_lib_doko__mc_sample(event_name, context, common_cpu,
	common_secs, common_nsecs, common_pid, common_comm,
	common_callchain, __probe_ip, perf_sample_dict):
		global start
		start = common_secs*1000000000 + common_nsecs

def probe_lib_doko__mc_sample__return(event_name, context, common_cpu,
	common_secs, common_nsecs, common_pid, common_comm,
	common_callchain, __probe_ip, perf_sample_dict):
		now = common_secs*1000000000 + common_nsecs
		duration = now - start
		times.append(duration)

def trace_unhandled(event_name, context, event_fields_dict, perf_sample_dict):
		print(get_dict_as_string(event_fields_dict))
		print('Sample: {'+get_dict_as_string(perf_sample_dict['sample'], ', ')+'}')

def print_header(event_name, cpu, secs, nsecs, pid, comm):
	print("%-20s %5u %05u.%09u %8u %-20s " % \
	(event_name, cpu, secs, nsecs, pid, comm), end="")

def get_dict_as_string(a_dict, delimiter=' '):
	return delimiter.join(['%s=%s'%(k,str(v))for k,v in sorted(a_dict.items())])
