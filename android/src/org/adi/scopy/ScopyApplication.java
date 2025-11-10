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

import android.system.Os;
import android.system.ErrnoException;
import android.preference.PreferenceManager;
import android.content.res.AssetManager;
import android.util.Log;


public class ScopyApplication extends QtApplication {
  private static void listAllFiles(Path currentPath, List<Path> allFiles)
    throws IOException {
    try (DirectoryStream<Path> stream = Files.newDirectoryStream(currentPath)) {
      for (Path entry : stream) {
        if (Files.isDirectory(entry)) {
          listAllFiles(entry, allFiles);
        } else {
          allFiles.add(entry);
        }
      }
    }
  }

  private static boolean copyAssetFolder(AssetManager assetManager, String fromAssetPath, String toPath) {

    try {
      String[] files = assetManager.list(fromAssetPath);
      new File(toPath).mkdirs();
      System.out.println("toPath " + toPath);
      System.out.println("fromAssetPath " + fromAssetPath);
      System.out.println("Files length: " + files.length);
      boolean res = true;
      for (String file : files) {
        System.out.println("Files: " + file);
        if (file.contains(".")) {
          res &= copyAsset(assetManager, fromAssetPath + "/" + file, toPath + "/" + file);
        } else {
          res &= copyAssetFolder(assetManager, fromAssetPath + "/" + file, toPath + "/" + file);
        }
      }
      return res;
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
      new File(toPath).createNewFile();
      out = new FileOutputStream(toPath);
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

  private static void copyFileUsingJava7Files(File source, File dest) throws IOException {
    Files.copy(source.toPath(), dest.toPath());
  }

  @Override
  public void onCreate() {
    System.out.println("QtApplication started");
    String apk = getApplicationInfo().sourceDir;
    String cache = getApplicationContext().getCacheDir().toString();
    System.out.println("sourcedir: " + getApplicationInfo().sourceDir);
    System.out.println("public sourcedir: " + getApplicationInfo().publicSourceDir);
    String libdir = getApplicationInfo().nativeLibraryDir;
    System.out.println("native library dir:" + libdir);
    System.out.println("applcation cache dir:" + cache);
    System.out.println("Hello Scopy !");

    try {
      Os.setenv("PYTHONHOME", ".", true);
      Os.setenv("PYTHONPATH", apk + "/assets/python3.11", true);
      Os.setenv("SIGROKDECODE_DIR", apk + "/assets/libsigrokdecode/decoders", true);
      Os.setenv("APPDATA", cache, true);
      Os.setenv("LD_LIBRARY_PATH", libdir, true);
      Os.setenv("IIOEMU_BIN", libdir + "/iio-emu.so", true);
    } catch (ErrnoException x) {
      System.out.println("Cannot set envvars");
    }

    super.onCreate();

    boolean reloadLibs = true;

    System.out.println("Copying assets to " + cache);

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

  private boolean isInstalled() {
    //return PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).getBoolean("installed", false);
    return false;
  }

  private void setInstalled() {
    PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit().putBoolean("installed", true).commit();
  }

  private void clearInstalled() {
    PreferenceManager.getDefaultSharedPreferences(getApplicationContext()).edit().putBoolean("installed", false).commit();
  }

}
