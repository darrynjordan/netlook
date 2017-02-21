import subprocess

repetitions = '10';
datasets = ['0', '1', '2', '3', '4'];
threads = ['1', '2', '4', '8', '9', '16', '18', '32', '36', '64', '72', '128', '144', '256', '288', '512', '576', '1024', '1152'];
lines = ['10000', '20000', '30000', '40000', '50000', '60000', '70000', '80000', '90000', '100000', '110000', '120000', '130000'];
zeros = ['2048', '4096', '8192', '10240'];
fftw_flags = ['0', '64'];
fftw_threads = ['1', '2'];


for dataset in datasets: 
	for thread in threads:		
		for line in lines:			
			for zero in zeros:				
				for fftw_flag in fftw_flags:					
					for fftw_thread in fftw_threads:
						subprocess.call(['../build/netlook', '-d ' + dataset, '-t ' + thread, '-l ' + line, '-z ' + zero, '-p ' + fftw_flag, '-f ' + fftw_thread, '-r ' + repetitions]);
						
