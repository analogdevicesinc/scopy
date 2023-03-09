package org.qtproject.example.jnimessenger;
import android.content.Context;
import android.hardware.usb.UsbManager;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.content.IntentFilter;
import android.app.PendingIntent;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.os.ParcelFileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileDescriptor;
import android.util.Log;
import java.util.HashMap;
import java.util.Iterator;


public class JniMessenger
{


private static final String ACTION_USB_PERMISSION =
    "com.android.example.USB_PERMISSION";
private static final String TAG = "MyActivity";
    
private final BroadcastReceiver usbReceiver = new BroadcastReceiver() {

    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        if (ACTION_USB_PERMISSION.equals(action)) {
            synchronized (this) {
                UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);

                if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                    if(device != null){
                      //call method to set up device communication
                   }
                }
                else {
                    Log.d(TAG, "permission denied for device " + device);
                }
            }
        }
         if (UsbManager.ACTION_USB_DEVICE_DETACHED.equals(action)) {
            UsbDevice device = (UsbDevice)intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
            if (device != null) {
                // call your method that cleans up and closes communication with the device
            }
        }
    }
};

    private static native void callFromJava(String message);

    public JniMessenger() {}

    public static void printFromJava(String message)
    {
        System.out.println("This is printed from JAVA, message is: " + message);
        callFromJava("Hello from JAVA!");
    }
    
    
     UsbDeviceConnection connection;
     UsbDevice device;
     
    public int getUsbFd(Context ctx) {
    
	UsbManager manager = (UsbManager) ctx.getSystemService(Context.USB_SERVICE);
	
	PendingIntent permissionIntent = PendingIntent.getBroadcast(ctx, 0, new Intent(ACTION_USB_PERMISSION), 0);
	IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
	ctx.registerReceiver(usbReceiver, filter);
	
	HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
	Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
	System.out.println("Devices found " + Integer.toString(deviceList.size()));
	while(deviceIterator.hasNext()){
	    device = deviceIterator.next();
	    System.out.println("Device " + device.getDeviceName());
	    manager.requestPermission(device, permissionIntent);
	    
	  //  if(ContextCompat.checkSelfPermission(ACTION_USB_PERMISSION) == PackageManager.PERMISSION_GRANTED) {
	    try {
	connection = manager.openDevice(device);
	
	
	String fd = Integer.toString(connection.getFileDescriptor());
	String usbfsPath = device.getDeviceName();
	String serial = device.getSerialNumber();
	String man = device.getManufacturerName();
	String pn = device.getProductName();
	String vid = Integer.toString(device.getVendorId(),16);        
	String pid = Integer.toString(device.getProductId(),16);
        
           System.out.println("fd: " + fd + "\nusbfs: "+usbfsPath+"\nserial: "+serial+"\nvid: "+vid+" pid: "+ pid + "\nManufacturer: "+ man  + "\nProduct Name: "+ pn);
           	    return connection.getFileDescriptor();
	    }catch(Exception e) {System.out.println(e);}

	//}
	}
	return -1;
    }
    
    public String getUsbFs() {
	    return device.getDeviceName();
    }
    
	
};

