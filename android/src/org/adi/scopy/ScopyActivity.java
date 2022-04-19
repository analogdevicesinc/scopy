/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

package org.adi.scopy;

import java.io.File;
import java.io.IOException;
import org.qtproject.qt5.android.bindings.QtActivity;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.Context;
import android.content.ComponentName;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.PendingIntent;

import android.widget.RemoteViews;
import android.widget.Toast;

import android.os.PowerManager;
import android.os.PowerManager.WakeLock;


//import androidx.core.app.NotificationCompat;


public class ScopyActivity extends QtActivity
{
	public static native void saveSessionJavaHelper();
	public static native void saveAndStopRunningToolsJNI();
	public static native void saveAndStopRunningInputToolsJNI();
	public static native void restoreRunningToolsJNI();
	public static native int nrOfToolsSavedJNI();
	public static native int nrOfToolsRunningJNI();
	public static native boolean hasCtxJNI();
	private static final int SCOPY_WAKELOCK_NOTIFICATION_ID = 1;
	private static final String SCOPY_NOTIFICATION_CHANNEL_ID = "scopy";
	boolean initialized;
	private WakeLock wakeLock;


	private void createNotificationChannel() {
	    // Create the NotificationChannel, but only on API 26+ because
	    // the NotificationChannel class is new and not in the support library

	        CharSequence name = "Scopy notifications";
		String description = "Various messages from Scopy";
		int importance = NotificationManager.IMPORTANCE_DEFAULT;
		NotificationChannel scopyNotificationChannel = new NotificationChannel(SCOPY_NOTIFICATION_CHANNEL_ID, name, importance);
		scopyNotificationChannel.setDescription(description);
		// Register the channel with the system; you can't change the importance
		// or other notification behaviors after this
		NotificationManager notificationManager = getSystemService(NotificationManager.class);
		notificationManager.createNotificationChannel(scopyNotificationChannel);

	}

        public void createNotification(String message)
	{
		NotificationManager notificationManager = (NotificationManager) getSystemService(NotificationManager.class);
		createNotificationChannel();

		Intent notificationIntent = new Intent(this, ScopyActivity.class);
		notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);

		PendingIntent openAppOnTapIntent = PendingIntent.getActivity(this, 0, notificationIntent, 0);

		Notification notification = new Notification.Builder(this,SCOPY_NOTIFICATION_CHANNEL_ID)
		        .setSmallIcon(R.drawable.icon)
			.setContentText(message)
			.setPriority(Notification.PRIORITY_DEFAULT)
			.setContentIntent(openAppOnTapIntent)
			.setOngoing(true)
			.build();

		notificationManager.notify(SCOPY_WAKELOCK_NOTIFICATION_ID, notification);
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		System.out.println("-- ScopyActivity: onCreate");
		initialized = false;
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
		wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
		        "Scopy::ScopyWakelockTag");

	}

	@Override
	protected void onStart()
	{
		System.out.println("-- ScopyActivity: onStart");
		super.onStart();
		if(initialized) {
			restoreRunningToolsJNI();
		}
	}

	@Override
	protected void onStop()
	{
		System.out.println("-- ScopyActivity: onStop");
		System.out.println(nrOfToolsSavedJNI());
		if (initialized) {
			if (hasCtxJNI()) {
				saveAndStopRunningInputToolsJNI();
				if (nrOfToolsRunningJNI() != 0) {
					System.out.println("-- Creating Notification");
					wakeLock.acquire();
					createNotification("Scopy is still running in the background. Device outputs are still enabled.");
				}
			}
		}
		super.onStop();
	}

        @Override
	protected void onResume()
	{
		cancelNotification(SCOPY_WAKELOCK_NOTIFICATION_ID);
		if (wakeLock.isHeld()) {
			wakeLock.release();
		}

		super.onResume();
	}

	protected void onPause(){
		System.out.println("-- ScopyActivity: onPause - saving application state to ini file ");
		if (initialized) {
			saveSessionJavaHelper(); // actually save the data
		}

		super.onPause();
	}

	protected void onDestroy(){
		System.out.println("-- ScopyActivity: onDestroy ");
		cancelNotification(SCOPY_WAKELOCK_NOTIFICATION_ID);
		if (wakeLock.isHeld()) {
			wakeLock.release();
		}
		if(initialized) {
			saveAndStopRunningToolsJNI();
		}

		super.onDestroy();
	}

	public void restart() {
		saveSessionJavaHelper();
		System.out.println("-- ScopyActivity: Restarting ");
		Context context = getApplicationContext();
		PackageManager packageManager = context.getPackageManager();
		Intent intent = packageManager.getLaunchIntentForPackage(context.getPackageName());
		ComponentName componentName = intent.getComponent();
		Intent mainIntent = Intent.makeRestartActivityTask(componentName);
		context.startActivity(mainIntent);
		Runtime.getRuntime().exit(0);
	}

	public String getScaleFactor() {
		initialized = true;
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		double scaleFactor = ((double)displayMetrics.widthPixels/displayMetrics.heightPixels)
								/displayMetrics.scaledDensity;
		String formattedScaleFactor = String.format("%.02f", scaleFactor);
		System.out.println("-- ScopyActivity: scale factor is: " + formattedScaleFactor);


		return formattedScaleFactor.replace(",",".");
	}

        private void cancelNotification(int id){
		NotificationManager nMgr = (NotificationManager) this.getSystemService(NotificationManager.class);
		nMgr.cancel(id);
	}
}
