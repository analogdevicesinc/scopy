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
import org.qtproject.qt.android.bindings.QtApplication;

import android.content.Context;
import android.content.SharedPreferences;
import android.system.Os;
import android.system.ErrnoException;
import android.util.Log;

public class ScopyApplication extends QtApplication {

  private static final String TAG = "ScopyApplication";

  @Override
  public void onCreate() {
    Log.d(TAG, "QtApplication started");
    String cache = getApplicationContext().getCacheDir().toString();
    Log.d(TAG, "SourceDir: " + getApplicationInfo().sourceDir);
    Log.d(TAG, "Public SourceDir: " + getApplicationInfo().publicSourceDir);
    String libdir = getApplicationInfo().nativeLibraryDir;
    Log.d(TAG, "Native Library dir:" + libdir);
    Log.d(TAG, "Application Cache dir:" + cache);
    Log.d(TAG, "Hello Scopy !");

    // APPDATA is where scopyconfig.cpp resolves style/translations/packages on Android.
    // The plugin .so's themselves are extracted from the APK assets into the cache dir below
    // and dlopen'd from there; LD_LIBRARY_PATH points at the flat nativeLibraryDir so that
    // their dependencies (the Scopy framework libs and the third-party .so's, which Qt does
    // place in lib/<abi>/) resolve.
    try {
      Os.setenv("APPDATA", cache, true);
      Os.setenv("LD_LIBRARY_PATH", libdir, true);
    } catch (ErrnoException x) {
      Log.e(TAG, "Cannot set environment variables", x);
    }

    super.onCreate();

    // Copy the bundled data files (style/translations/package metadata) to APPDATA on first run.
    SharedPreferences prefs = getSharedPreferences("app_prefs", Context.MODE_PRIVATE);
    if (!Helpers.isInstalled(prefs)) {
      Log.d(TAG, "First installation detected. Copying all assets to " + cache);
      boolean copySuccess = Helpers.copyAllAssets(getAssets(), cache);
      if (copySuccess) {
        Helpers.setInstalled(prefs);
        Log.d(TAG, "All assets successfully copied to: " + cache);
      } else {
        Log.e(TAG, "Failed to copy all assets. App may not function properly.");
      }
    } else {
      Log.d(TAG, "All assets are already copied to: " + cache);
    }
  }

}
