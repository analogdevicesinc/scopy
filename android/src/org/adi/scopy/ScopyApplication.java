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

import static java.lang.System.in;

import org.qtproject.qt5.android.bindings.QtApplication;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.stream.Stream;

import android.content.Context;
import android.content.SharedPreferences;
import android.system.Os;
import android.system.ErrnoException;
import android.preference.PreferenceManager;
import android.content.res.AssetManager;
import android.util.Log;


public class ScopyApplication extends QtApplication {
  private static boolean isAssetDirectory(AssetManager assetManager, String path) {
    try {
      String[] files = assetManager.list(path);
      return files != null && files.length > 0;
    } catch (IOException e) {
      return false;
    }
  }

  private static boolean copyAssetFolder(AssetManager assetManager, String fromAssetPath, String toPath) {
    try {
      String[] files = assetManager.list(fromAssetPath);
      if (files == null || files.length == 0) {
        System.out.println("No files found in: " + fromAssetPath);
        return false;
      }
      File toDir = new File(toPath);
      if (!toDir.exists()) {
        boolean created = toDir.mkdirs();
        if (!created && !toDir.exists()) {
          System.out.println("Can't create folder: " + toPath);
          return false;
        }
      }

      boolean ret = true;
      for (String file : files) {
        String fromChildPath = fromAssetPath + "/" + file;
        String toChildPath = toPath + "/" + file;
        System.out.println("Processing: " + fromChildPath);
        if (isAssetDirectory(assetManager, fromChildPath)) {
          ret &= copyAssetFolder(assetManager, fromChildPath, toChildPath);
        } else {
          ret &= copyAsset(assetManager, fromChildPath, toChildPath);
        }
      }
      return ret;
    } catch (Exception e) {
      e.printStackTrace();
      return false;
    }
  }

  private static boolean copyAsset(AssetManager assetManager, String fromAssetPath, String toPath) {
    InputStream in = null;
    OutputStream out = null;
    try {
      System.out.println("Copying from " + fromAssetPath + " to " + toPath);
      in = assetManager.open(fromAssetPath);
      out = Files.newOutputStream(Paths.get(toPath));
      copyFile(in, out);
      in.close();
      in = null;
      out.flush();
      out.close();
      out = null;
      if (toPath.endsWith(".so")) {
        System.out.println("Making " + toPath + "executable !");
        File f = new File(toPath);
        f.setReadable(true, false);
        f.setExecutable(true, false);
        f.setWritable(true, false);
      }
      return true;
    } catch (Exception e) {
      e.printStackTrace();
      return false;
    }
  }

  private static void copyFile(InputStream in, OutputStream out) throws IOException {
    byte[] buffer = new byte[1024];
    int read;
    while ((read = in.read(buffer)) != -1) {
      out.write(buffer, 0, read);
    }
  }

  @Override
  public void onCreate() {
    System.out.println("QtApplication started");
    String apk = getApplicationInfo().sourceDir;
    String cache = getApplicationContext().getCacheDir().toString();
    System.out.println("SourceDir: " + getApplicationInfo().sourceDir);
    System.out.println("Public SourceDir: " + getApplicationInfo().publicSourceDir);
    String libdir = getApplicationInfo().nativeLibraryDir;
    System.out.println("Native Library dir:" + libdir);
    System.out.println("Application Cache dir:" + cache);
    System.out.println("Hello Scopy !");

    try {
      Os.setenv("PYTHONHOME", cache + "/python3.12", true);
      Os.setenv("PYTHONPATH", cache + "/python3.12", true);
      Os.setenv("SIGROKDECODE_DIR", cache + "/decoders", true);
      Os.setenv("APPDATA", cache, true);
      Os.setenv("LD_LIBRARY_PATH", libdir, true);
      Os.setenv("IIOEMU_BIN", libdir + "/iio-emu.so", true);
    } catch (ErrnoException x) {
      System.out.println("Cannot set envvars");
    }

    super.onCreate();

    SharedPreferences prefs = getSharedPreferences("app_prefs", Context.MODE_PRIVATE);
    boolean reloadLibs = true;
    if (reloadLibs)
      clearInstalled(prefs);

    if (!isInstalled(prefs)) {
      setInstalled(prefs);
      System.out.println("Copying all assets to " + cache);
      String[] folders = null;
      try {
        folders = getAssets().list("");
      } catch (Exception e) {
        e.printStackTrace();
      }

      for (String folder : folders) {
        System.out.println("Folder found: " + folder);
        copyAssetFolder(getAssets(), folder, cache + "/" + folder);
      }
    } else {
      System.out.println("All assets are copied to: " + cache);
    }

//    System.out.println("Copy Files here:");
//    for (File file : Objects.requireNonNull(new File(cache).listFiles())) {
//      System.out.println("File found: " + file.getName());
//      copyAssetFolder(getAssets(), file.getName(), cache + file.getName());
//    }

    // make a method to copy all files from Assets to Cache
//    copyAssetFolder(getAssets(), "scopy-plugins", cache + "/scopy-plugins");
//    copyAssetFolder(getAssets(), "style", cache + "/style");
//    copyAssetFolder(getAssets(), "translations", cache + "/translations");
//    copyAssetFolder(getAssets(), "decoders", cache + "/decoders");
//    copyAssetFolder(getAssets(), "python3.11", cache + "/decoders");
//      copyAsset(getAssets(), "scopy_emu_options_config.json", cache + "/scopy_emu_options_config.json");+


  }

  private boolean isInstalled(SharedPreferences prefs) {
    return prefs.getBoolean("installed", false);
  }

  private void setInstalled(SharedPreferences prefs) {
    prefs.edit().putBoolean("installed", true).apply();
  }

  private void clearInstalled(SharedPreferences prefs) {
    prefs.edit().putBoolean("installed", false).apply();
  }

}
