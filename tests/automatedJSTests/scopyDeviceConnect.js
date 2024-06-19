/* main function */
function main(){
    //CONNECT TO EMU
    var deviceID = scopy.addDevice("", "ip:127.0.0.0")
    //CONNECT TO DEVICE
    scopy.connectDevice(deviceID)
}

main()
