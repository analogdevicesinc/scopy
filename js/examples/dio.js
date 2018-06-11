/* 4 bit counter using DigitalIO */

/* Setup DigitalIO */
function set_dio(){
	
	/* Set first 4 digital channels as output */ 
	for (var i = 0; i < 4; i++){
		dio.dir[i]=true
		dio.out[i]=true
	}
	
	/* Set the rest of 12 channels as input */
	for ( var i = 4; i < 16; i++){
		dio.dir[i]=false
		dio.out[i]=false
	}
	
	/* Run DigitalIO */
	dio.running = true
	msleep(1000)
	
}

/* 4-bit Binary count function */
function binary_counter(){
	
	for(var i = 0; i <= 15; i++){
		
		var k = i
		for(var j = 0; j <4; j++){
			
			dio.out[j] = k % 2
			k = Math.floor(k/2)
		
		}
		
		msleep(1000)
		
	}
	dio.running = false
}

/* main function */
function main(){

	set_dio()
	
	binary_counter()
	
}

main()