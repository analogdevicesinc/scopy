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

//import androidx.core.app.NotificationCompat;


public class ScopyActivity extends QtActivity
{
	public static native void saveSessionJavaHelper();
	public static native void saveAndStopRunningToolsJNI();
	public static native void saveAndStopRunningInputToolsJNI();
	public static native void restoreRunningToolsJNI();
	public static native int nrOfToolsSaved();
	public static native int nrOfToolsRunning();
	boolean initialized;


	private void createNotificationChannel() {
	    // Create the NotificationChannel, but only on API 26+ because
	    // the NotificationChannel class is new and not in the support library

		CharSequence name = "R.string.channel_name";
		String description = "R.string.channel_description";
		int importance = NotificationManager.IMPORTANCE_DEFAULT;
		NotificationChannel channel = new NotificationChannel("1234", name, importance);
		channel.setDescription("description");
		// Register the channel with the system; you can't change the importance
		// or other notification behaviors after this
		NotificationManager notificationManager = getSystemService(NotificationManager.class);
		notificationManager.createNotificationChannel(channel);

	}

	public void createNotification() {
		NotificationManager notificationManager = (NotificationManager) getSystemService(NotificationManager.class);
		createNotificationChannel();
		Notification notif = new Notification.Builder(this,"1234")
			.setSmallIcon(R.drawable.icon)
			.setContentTitle("Scopy")
			.setContentText("Scopy still running in the background ")
			.setPriority(Notification.PRIORITY_DEFAULT)
			.setOngoing(true)
			.build();
		notificationManager.notify(1234567, notif);
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		System.out.println("-- ScopyActivity: onCreate");
		initialized = false;
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
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
		if(initialized) {
			saveAndStopRunningInputToolsJNI();
			if(nrOfToolsRunning() != 0) {
				System.out.println("-- Creating Notification");
				createNotification();
				/*NotificationCompat.Builder builder = new NotificationCompat.Builder(this, DEFAULT_CHANNEL_ID)
					.setContentTitle("Scopy")
					.setContentText("Input instruments paused. Output instruments still running ... ")
					.setPriority(NotificationCompat.PRIORITY_DEFAULT);*/
					;
			}
		}
		super.onStop();
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
}
