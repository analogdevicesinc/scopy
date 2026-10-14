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

// Qt6: the bindings package is org.qtproject.qt.android.bindings (Qt5 used qt5).
import org.qtproject.qt.android.bindings.QtActivity;
import android.content.pm.PackageManager;
import android.content.Intent;
import android.content.Context;
import android.content.ComponentName;
import android.os.Build;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.PendingIntent;

import android.os.PowerManager;
import android.os.PowerManager.WakeLock;

public class ScopyActivity extends QtActivity
{
	private static final int SCOPY_WAKELOCK_NOTIFICATION_ID = 1;
	private static final String SCOPY_NOTIFICATION_CHANNEL_ID = "scopy";
	private WakeLock wakeLock;

	private void createNotificationChannel() {
		// NotificationChannel is required on API 26+.
		CharSequence name = "Scopy notifications";
		String description = "Various messages from Scopy";
		int importance = NotificationManager.IMPORTANCE_DEFAULT;
		NotificationChannel scopyNotificationChannel = new NotificationChannel(SCOPY_NOTIFICATION_CHANNEL_ID, name, importance);
		scopyNotificationChannel.setDescription(description);
		NotificationManager notificationManager = getSystemService(NotificationManager.class);
		notificationManager.createNotificationChannel(scopyNotificationChannel);
	}

	public void createNotification(String message)
	{
		NotificationManager notificationManager = (NotificationManager) getSystemService(NotificationManager.class);
		createNotificationChannel();

		Intent notificationIntent = new Intent(this, ScopyActivity.class);
		notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);

		// FLAG_IMMUTABLE is mandatory for PendingIntents on API 31+.
		int flags = PendingIntent.FLAG_IMMUTABLE;
		PendingIntent openAppOnTapIntent = PendingIntent.getActivity(this, 0, notificationIntent, flags);

		Notification notification = new Notification.Builder(this, SCOPY_NOTIFICATION_CHANNEL_ID)
			.setSmallIcon(R.drawable.icon)
			.setContentText(message)
			.setContentIntent(openAppOnTapIntent)
			.setOngoing(true)
			.build();

		notificationManager.notify(SCOPY_WAKELOCK_NOTIFICATION_ID, notification);
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		System.out.println("-- ScopyActivity: onCreate");
		super.onCreate(savedInstanceState);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
		wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "Scopy::ScopyWakelockTag");
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

	@Override
	protected void onDestroy()
	{
		System.out.println("-- ScopyActivity: onDestroy ");
		cancelNotification(SCOPY_WAKELOCK_NOTIFICATION_ID);
		if (wakeLock.isHeld()) {
			wakeLock.release();
		}
		super.onDestroy();
	}

	// Called from C++ (ApplicationRestarter) through JNI to relaunch the app.
	public void restart() {
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
		DisplayMetrics displayMetrics = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);

		double scaleFactor = ((double) displayMetrics.widthPixels / displayMetrics.heightPixels)
				/ displayMetrics.scaledDensity;
		String formattedScaleFactor = String.format("%.02f", scaleFactor);
		System.out.println("-- ScopyActivity: scale factor is: " + formattedScaleFactor);

		return formattedScaleFactor.replace(",", ".");
	}

	private void cancelNotification(int id) {
		NotificationManager nMgr = (NotificationManager) this.getSystemService(NotificationManager.class);
		nMgr.cancel(id);
	}
}
