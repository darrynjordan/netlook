import subprocess

repetitions = '10';
datasets = ['0', '1', '2', '3', '4'];
sizes =  ['2048', '4096', '8192', '10240'];
fftw_flags = ['64'];
fftw_threads = ['1'];
threads = ['1', '2', '4', '8', '16', '32', '64', '128'];

for dataset in datasets: 
	for size in sizes:	
		for fftw_flag in fftw_flags:				
			for fftw_thread in fftw_threads:
				for thread in threads:	
					subprocess.call(['../build/netlook', '-d ' + dataset, '-t ' + thread, '-l ' + size, '-z ' + size, '-p ' + fftw_flag, '-f ' + fftw_thread, '-r ' + repetitions]);
						
