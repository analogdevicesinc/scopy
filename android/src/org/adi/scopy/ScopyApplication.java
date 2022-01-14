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

import org.qtproject.qt5.android.bindings.QtApplication;

import java.io.File;
import java.io.IOException;
import android.system.Os;
import android.system.ErrnoException;


public class ScopyApplication extends QtApplication
{
	@Override
	public void onCreate()
	{
		System.out.println("QtApplication started");
		String apk = getApplicationInfo().sourceDir;
		String cache = getApplicationContext().getCacheDir().toString();
		System.out.println("sourcedir: "+ getApplicationInfo().sourceDir);
		System.out.println("public sourcedir: "+ getApplicationInfo().publicSourceDir);
		String libdir = getApplicationInfo().nativeLibraryDir;
		System.out.println("native library dir:" + libdir);
		System.out.println("applcation cache dir:" + cache);
		System.out.println("Hello Scopy !");

		try {
		    Os.setenv("PYTHONHOME",".",true);
		    Os.setenv("PYTHONPATH",apk + "/assets/python3.8",true);
		    Os.setenv("SIGROKDECODE_DIR", apk + "/assets/libsigrokdecode/decoders",true);
		    Os.setenv("APPDATA", cache, true);
		    Os.setenv("LD_LIBRARY_PATH", libdir, true);
		    Os.setenv("IIOEMU_BIN", libdir+"/iio-emu.so", true);

		}

		catch(ErrnoException x) {
		     System.out.println("Cannot set envvars");
		}

		super.onCreate();
	}

}
