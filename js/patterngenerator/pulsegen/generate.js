function is_periodic()
{
	return true
}

function get_required_nr_of_samples()
{
	return 10
}

function get_min_sampling_freq()
{
	return 1
}

function generate()
{

console.log("##############")
status_window.print("#############")
	var i=0
	console.log(pg.get_nr_of_samples())
	for(i=0;i<pg.get_nr_of_samples();i++)
		pg.buffer[i] = 0;
	for(i=0;i<nrOfPulses;i++)
	{
		for(var j=i*samplesPerPulse;j<((i*samplesPerPulse)+loPulse);j++)
			pg.buffer[j] = 0x0000;
		for(var j=((i*samplesPerPulse)+loPulse);(j<((i+1)*samplesPerPulse));j++)
			pg.buffer[j] = 0xffff;
	}
	//pg.buffersize = nrOfPulses*samplesPerPulse
	pg.buffersize = pg.get_nr_of_samples()
}

