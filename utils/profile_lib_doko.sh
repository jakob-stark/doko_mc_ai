
sudo perf probe -x lib_doko.so 'mc_sample'
sudo perf probe -x lib_doko.so 'mc_sample%return'

sudo perf record -e probe_lib_doko:mc_sample -e probe_lib_doko:mc_sample__return -aR sleep 10

sudo perf probe -d 'mc_sample'
sudo perf probe -d 'mc_sample__return'

sudo chown jakob:jakob perf.data

