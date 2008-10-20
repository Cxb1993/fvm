#! /usr/bin/env python
# encoding: utf-8
#
# Martin Hunt <mmh@purdue.edu>

"""
Functions for dealing with PBS
"""

import os, config, time

def start(bp, cname):
    if not config.config('Testing', 'PBS'):
        return 0
    
    in_batch = 0
    try:
        if os.environ["PBS_ENVIRONMENT"] == 'PBS_BATCH':
            in_batch = 1
    except:
        pass

    if in_batch:
        return 0

    qsub(bp, cname)
    return 1


# write pbs batch file and submit it
def qsub(bp, cname):

    # first remove any old results
    os.system('/bin/rm -f run_tests.pbs.*')
    
    f = open(os.path.join(bp.topdir, 'run_tests.pbs'), 'w')
    f.write('#!/bin/bash\n')

    qname = config.config('Testing', 'queue')
    f.write('#PBS -q %s\n' % qname)

    cpus = config.config('Testing', 'cpus')
    nodes = config.config('Testing', 'nodes')
    f.write('#PBS -l select=%s:ncpus=%s:mpiprocs=%s\n' % (nodes,cpus,cpus))
    f.write('#PBS -l walltime=1:00:00\n')

    f.write('cd  $PBS_O_WORKDIR\n')
    f.write('source env.sh\n')
    
    for cmd in config.config('before', 0):
        f.write(cmd + '\n')

    f.write('./make.py --test %s\n' % cname)
    f.close()

    job = os.popen("qsub run_tests.pbs").readline()
    job = job.split('.')[0]
    
    print "Job %s submitted to queue %s." % (job, qname)

    state = ''
    while True:
        time.sleep(15)
        st = os.popen("qstat %s &> /dev/null" % job)
        try:
            newstate = st.read().split('\n')[2].split()[4]
            if newstate != state:
                if newstate == 'Q':
                    print "Waiting for job to start."
                elif newstate == 'R':
                    print 'Job is running.'
                elif newstate == 'E':
                    print 'Job state is exiting.'
                elif newstate ==  'X':
                    break
                else:
                    print 'Error: job state is %s. Exiting.' % newstate
                    break
                state = newstate
        except:
            pass
        if st.close():
            break


    for filename in ['run_tests.pbs.e%s' % job, 'run_tests.pbs.o%s' % job]:
        st = os.stat(filename)
        if st.st_size:
            os.system('cat %s' % filename)
        

    
