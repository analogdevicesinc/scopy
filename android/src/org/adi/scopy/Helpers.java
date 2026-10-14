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

import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;

// Copies the Scopy data files (style, translations, package metadata) bundled in the APK
// assets into the app writable/cache dir on first launch. The native plugin .so's are NOT
// copied here: androiddeployqt flattens them into lib/<abi>/ via QT_ANDROID_EXTRA_LIBS and
// they are loaded from nativeLibraryDir by base name.
public class Helpers {

    private static final int BUFFER_SIZE = 8192; // 8KB buffer for better performance
    private static final int THREAD_POOL_SIZE = 4; // Number of parallel copy threads
    private static final String TAG = "ScopyApplicationHelper";

    // Cache for directory checks to avoid repeated asset listings
    private static final Map<String, Boolean> directoryCache = new HashMap<>();

    public static boolean isAssetDirectory(AssetManager assetManager, String path) {
        // Check cache first
        Boolean cached = directoryCache.get(path);
        if (cached != null) {
            return cached;
        }

        try {
            String[] files = assetManager.list(path);
            boolean isDirectory = files != null && files.length > 0;
            directoryCache.put(path, isDirectory);
            return isDirectory;
        } catch (IOException e) {
            directoryCache.put(path, false);
            return false;
        }
    }

    public static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[BUFFER_SIZE];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

    private static boolean copyAsset(AssetManager assetManager, String fromAssetPath, String toPath) {
        // Use try-with-resources for automatic resource management
        try (InputStream in = assetManager.open(fromAssetPath); OutputStream out = Files.newOutputStream(Paths.get(toPath))) {
            copyFile(in, out);
            if (toPath.endsWith(".so")) {
                File f = new File(toPath);
                f.setReadable(true, false);
                f.setExecutable(true, false);
                f.setWritable(true, false);
            }
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Error copying asset: " + fromAssetPath, e);
            return false;
        }
    }

    // Batch copy method for copying all assets efficiently
    public static boolean copyAllAssets(AssetManager assetManager, String cacheDir) {
        try {
            directoryCache.clear();

            String[] topLevelFiles = assetManager.list("");
            if (topLevelFiles == null || topLevelFiles.length == 0) {
                Log.e(TAG, "No assets found");
                return false;
            }

            Log.d(TAG, "Starting copy of " + topLevelFiles.length + " top-level folders");
            long startTime = System.currentTimeMillis();

            AtomicBoolean allSuccess = new AtomicBoolean(true);

            for (String folder : topLevelFiles) {
                Log.d(TAG, "Copying folder: " + folder);
                boolean success = copyAssetFolder(assetManager, folder, cacheDir + "/" + folder);
                if (!success) {
                    allSuccess.set(false);
                    Log.e(TAG, "Failed to copy folder: " + folder);
                }
            }

            long endTime = System.currentTimeMillis();
            Log.d(TAG, "Asset copy completed in " + (endTime - startTime) / 1000.0 + " seconds");

            directoryCache.clear();
            return allSuccess.get();
        } catch (Exception e) {
            Log.e(TAG, "Error in batch asset copy", e);
            return false;
        }
    }

    public static void countTotalFiles(AssetManager assetManager, String path, AtomicInteger counter) {
        try {
            String[] files = assetManager.list(path);
            if (files != null) {
                for (String file : files) {
                    String childPath = path + "/" + file;
                    if (isAssetDirectory(assetManager, childPath)) {
                        countTotalFiles(assetManager, childPath, counter);
                    } else {
                        counter.incrementAndGet();
                    }
                }
            }
        } catch (IOException e) {
            Log.e(TAG, "Error counting files in: " + path, e);
        }
    }

    public static boolean copyAssetFolder(AssetManager assetManager, String fromAssetPath, String toPath) {
        try {
            String[] files = assetManager.list(fromAssetPath);
            if (files == null || files.length == 0) {
                Log.d(TAG, "No files found in: " + fromAssetPath);
                return false;
            }

            File toDir = new File(toPath);
            if (!toDir.exists()) {
                boolean created = toDir.mkdirs();
                if (!created && !toDir.exists()) {
                    Log.e(TAG, "Can't create folder: " + toPath);
                    return false;
                }
            }

            // Use parallel processing for better performance
            ExecutorService executor = Executors.newFixedThreadPool(THREAD_POOL_SIZE);
            AtomicBoolean allSuccess = new AtomicBoolean(true);
            AtomicInteger totalFiles = new AtomicInteger(0);
            AtomicInteger processedFiles = new AtomicInteger(0);

            // Count total files for progress tracking
            countTotalFiles(assetManager, fromAssetPath, totalFiles);

            for (String file : files) {
                String fromChildPath = fromAssetPath + "/" + file;
                String toChildPath = toPath + "/" + file;

                if (isAssetDirectory(assetManager, fromChildPath)) {
                    // Recursively copy directories
                    executor.submit(() -> {
                        boolean success = copyAssetFolder(assetManager, fromChildPath, toChildPath);
                        if (!success) allSuccess.set(false);
                    });
                } else {
                    // Copy files in parallel
                    executor.submit(() -> {
                        boolean success = copyAsset(assetManager, fromChildPath, toChildPath);
                        if (!success) allSuccess.set(false);
                        int processed = processedFiles.incrementAndGet();
                        if (processed % 100 == 0) { // Log progress every 100 files
                            Log.d(TAG, "Progress: " + processed + "/" + totalFiles.get() + " files copied");
                        }
                    });
                }
            }

            executor.shutdown();
            try {
                if (!executor.awaitTermination(30, TimeUnit.MINUTES)) {
                    Log.e(TAG, "Timeout waiting for file copy completion");
                    executor.shutdownNow();
                    return false;
                }
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupted while copying files", e);
                executor.shutdownNow();
                Thread.currentThread().interrupt();
                return false;
            }

            return allSuccess.get();
        } catch (Exception e) {
            Log.e(TAG, "Error copying asset folder", e);
            return false;
        }
    }

    public static boolean isInstalled(SharedPreferences prefs) {
        return prefs.getBoolean("installed", false);
    }

    public static void setInstalled(SharedPreferences prefs) {
        prefs.edit().putBoolean("installed", true).apply();
    }

    public static void clearInstalled(SharedPreferences prefs) {
        prefs.edit().putBoolean("installed", false).apply();
    }

}
