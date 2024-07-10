function connect(){
    //TEST WITH EMU
    var  deviceID = scopy.addDevice("", "ip:127.0.0.0")
    //CONNECT TO DEVICE
    scopy.connectDevice(deviceID)
    msleep(1000)
}

function disconnect(){
    //DISCONNECT FROM DEVICE
    scopy.disconnectDevice()
    exit(0)
}
