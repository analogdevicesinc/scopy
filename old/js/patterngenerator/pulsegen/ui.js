function setSamples()
{
	console.log("setSamples")
	ui_elements.samplesPerPulse.setText( (parseInt(ui_elements.hiPulse.text) + parseInt(ui_elements.loPulse.text)).toString() )	
}

function verifyConditions()
{
	console.log("verify conditions")
	console.log(parseInt(ui_elements.hiPulse.text))
	console.log(parseInt(ui_elements.loPulse.text))
	console.log(parseInt(ui_elements.hiPulse.text) + parseInt(ui_elements.loPulse.text))
	console.log(parseInt(ui_elements.samplesPerPulse.text))
	if( (parseInt(ui_elements.hiPulse.text) + parseInt(ui_elements.loPulse.text)) == parseInt(ui_elements.samplesPerPulse.text)) 
{
	ui_elements.label_6.setText("VALID")
}
else 
{
	ui_elements.label_6.setText("INVALID")
}
}


function post_load_ui()
{
//console.log(ui_elements)
console.log("abcd");
	for(property in script){
	console.log(property);
}
	ui_elements.nrOfPulses.text = 2
	ui_elements.samplesPerPulse.setText("5")
	ui_elements.hiPulse.setText("2")
	ui_elements.loPulse.setText("3")

	ui_elements.hiPulse.textChanged.connect(this,setSamples)
	ui_elements.loPulse.textChanged.connect(this,setSamples)
	ui_elements.samplesPerPulse.textChanged.connect(this,verifyConditions)

	ui_elements.nrOfPulses.textChanged.connect(this,script.parse_ui);
	ui_elements.hiPulse.textChanged.connect(this,script.parse_ui);
	ui_elements.loPulse.textChanged.connect(this,script.parse_ui);
	ui_elements.samplesPerPulse.textChanged.connect(this,script.parse_ui);
	console.log("connected parse_ui");
	script.parse_ui();
//	ui_elements.hiPulse.textChanged.connect(this,parse_ui);

}
function parse_ui_callback()
{
	console.log("parse UI")
	//for(property in ui_elements.nrOfPulses){
	//console.log(property);}

	nrOfPulses = parseInt(ui_elements.nrOfPulses.text)
	console.log(nrOfPulses)
	samplesPerPulse = parseInt(ui_elements.samplesPerPulse.text)
	console.log(samplesPerPulse)
	hiPulse = parseInt(ui_elements.hiPulse.text)
	console.log(hiPulse)
	loPulse = parseInt(ui_elements.loPulse.text)
	console.log(loPulse)

}

