#include "defined_variables.h"

{
	"app-id": "org.adi.Scopy",
	"runtime": "org.kde.Platform",
	"runtime-version": EXPAND(RUNTIME_VERSION),
	"sdk": "org.kde.Sdk",
	"command": "scopy",
	"rename-desktop-file": "scopy.desktop",
	"rename-icon": "scopy",
	"finish-args": [
		"--socket=x11",
		"--socket=wayland",
                "--socket=pulseaudio",
		"--share=network",
		"--share=ipc",
		"--filesystem=host:create",
		"--filesystem=~/.config/dconf:create",
		"--filesystem=xdg-config/ADI:create",
		"--device=all",
		"--system-talk-name=org.freedesktop.Avahi"
	],

	"build-options" : {
		"build-args": ["--share=network"],
		"cflags": "-O2 -g",
		"cxxflags": "-O2 -g",
		"env": {
			"V": "1"
		}

	},
	"cleanup": [
		"/include",
		"/lib/python2.7",
		"/lib/pkgconfig",
		"/lib/cmake",
		"/lib/*.la",
		"/lib/*.a",
		"/lib/gio",
		"/lib/giomm-2.4",
		"/lib/libzip",
		"/lib/libgthread*",
		"/lib/xml2Conf.sh"
	],
	"modules": [
		{
			"name":"git-config-update",
			"buildsystem": "simple",
			"build-commands": [
				"git config --global protocol.file.allow always"
			]
		},
		{
			"name": "sshpass",
			"sources": [
				{
					"type": "archive",
					"url": "https://sourceforge.net/projects/sshpass/files/sshpass/1.08/sshpass-1.08.tar.gz",
					"sha1": "efe4573ba2fe972b08cf1cdd95739b7f456e55c1"
				}
			]
		},
		{
			"name": "libusb",
			"config-opts": [ "--disable-udev", "--prefix=/app" ],
			"sources": [
				{
					"type": "archive",
					"url": "https://downloads.sourceforge.net/project/libusb/libusb-1.0/libusb-1.0.24/libusb-1.0.24.tar.bz2",
					"sha256": "7efd2685f7b327326dcfb85cee426d9b871fd70e22caa15bb68d595ce2a2b12a"
				}
			]
		},
		"shared-modules/dbus-glib/dbus-glib.json",
		{
			"name": "dbus-glib-submodule"
		},
		"shared-modules/intltool/intltool-0.51.json",
		{
			"name": "intltool-submodule"
		},
#ifndef __ARM__
		{
		    "name": "dbus-python",
		    "sources": [
                {
                    "type": "archive",
                    "url": "https://dbus.freedesktop.org/releases/dbus-python/dbus-python-1.3.2.tar.gz",
                    "sha256": "ad67819308618b5069537be237f8e68ca1c7fcc95ee4a121fe6845b1418248f8"
                }
			]
		},
#endif
		{
			"name": "avahi",
			"config-opts": [
				"--disable-monodoc",
				"--disable-gtk",
				"--disable-gtk3",
				"--disable-qt3",
				"--disable-qt4",
				"--disable-pygobject",
				"--disable-gdbm",
				"--disable-libdaemon",
				"--disable-mono",
				"--disable-pygtk",
				"--disable-python",
#ifdef __ARM__
                "--disable-dbus",
#endif
				"--enable-compat-libdns_sd",
				"--with-distro=none"
      			],
			"sources": [
       				{
				"type": "archive",
				"url": "https://github.com/lathiat/avahi/releases/download/v0.7/avahi-0.7.tar.gz",
				"sha256": "57a99b5dfe7fdae794e3d1ee7a62973a368e91e414bd0dfa5d84434de5b14804"
        			}
			]
		},
		{
			"name": "libxml2",
			"cleanup": [ "/bin", "/share" ],
			"config-opts": [ "--prefix=/app"],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/GNOME/libxml2",
					"tag": "v2.9.14"
				}
			]
		},
		{
			"name": "libfftw3",
			"config-opts": [
				"--enable-shared",
				"--disable-static",
				"--enable-threads",
                "--enable-single"
#ifdef __X86__
                ,"--enable-float",
                "--enable-sse2",
                "--enable-avx",
				"--enable-openmp"
#endif
			],
			"sources": [
			        {
					"type": "archive",
					"url": "http://swdownloads.analog.com/cse/scopydeps/fftw-3.3.8.tar.gz",
					"sha256": "6113262f6e92c5bd474f2875fa1b01054c4ad5040f6b0da7c03c98821d9ae303"
				}
			]
		},
		{
			"name": "python3-mako",
			"buildsystem": "simple",
			"build-commands": [
				"pip3 install --no-index --find-links=\"file://${PWD}\" --prefix=/app mako"
			],
			"sources": [
				{
					"type": "file",
					"url": "https://files.pythonhosted.org/packages/b9/2e/64db92e53b86efccfaea71321f597fa2e1b2bd3853d8ce658568f7a13094/MarkupSafe-1.1.1.tar.gz",
					"sha256": "29872e92839765e546828bb7754a68c418d927cd064fd4708fab9fe9c8bb116b"
				},
				{
					"type": "file",
					"url": "https://files.pythonhosted.org/packages/28/03/329b21f00243fc2d3815399413845dbbfb0745cff38a29d3597e97f8be58/Mako-1.1.1.tar.gz",
					"sha256": "2984a6733e1d472796ceef37ad48c26f4a984bb18119bb2dbc37a44d8f6e75a4"
				}
			],
			"cleanup": [ "*" ]
		},
		{

			"name": "serialport",
			"builddir": false,
			"buildsystem": "autotools",
			"config-opts": [ "--prefix=/app" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/sigrokproject/libserialport",
					"branch" : "master"
				}
			]

		},

		{
			"name": "libiio",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_PREFIX_PATH:PATH=/app",
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DCMAKE_INSTALL_LIBDIR:STRING=lib",
				"-DINSTALL_UDEV_RULE:BOOL=OFF",
				"-DWITH_TESTS:BOOL=OFF",
				"-DWITH_DOC:BOOL=OFF",
				"-DHAVE_DNS_SD:BOOL=ON",
				"-DWITH_IIOD:BOOL=OFF",
				"-DWITH_LOCAL_BACKEND:BOOL=OFF",
				"-DENABLE_IPV6:BOOL=OFF",
				"-DWITH_SERIAL_BACKEND:BOOL=ON"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libiio",
					"tag": "v0.26"
				}
			]
		},
		{
			"name": "libad9361",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app", "-DCMAKE_INSTALL_LIBDIR:STRING=lib" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libad9361-iio",
					"branch" : "main"
				}
			]
		},
		{
			"name": "libad9166",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app", "-DCMAKE_INSTALL_LIBDIR:STRING=lib" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libad9166-iio",
					"branch" : "libad9166-iio-v0"
				}
			]
		},
		{
			"name": "qwt",
			"cleanup": [ "/features", "/plugins", "/share" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/cseci/qwt",
					"branch": "qwt-multiaxes-updated"
				},
				{
					"type": "script",
					"commands": [
						"sed -i \"s/^\\s*QWT_INSTALL_PREFIX.*$/QWT_INSTALL_PREFIX=\\/app/g\" qwtconfig.pri",
						"sed -i \"s/qwtLibraryTarget(qwt)/qwtLibraryTarget(qwt_scopy)/\" src/src.pro",
						"sed -i \"s/QWT_SONAME=libqwt.so/QWT_SONAME=libqwt_scopy.so/\" src/src.pro",
						"sed -i 's|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt)|qwtAddLibrary($${QWT_OUT_ROOT}/lib, qwt_scopy)|' designer/designer.pro examples/examples.pri playground/playground.pri tests/tests.pri",
						"qmake"
					],
					"dest-filename": "configure"
				}
			]
		},
		{
			"name": "libzip",
			"cleanup": [ "/bin", "/share" ],
			"sources": [
				{
					"type": "archive",
					"url": "https://nih.at/libzip/libzip-1.1.3.tar.xz",
					"sha256": "729a295a59a9fd6e5b9fe9fd291d36ae391a9d2be0b0824510a214cfaa05ceee"
				}
			]
		},
		{
			"name": "libtinyiiod",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DCMAKE_INSTALL_LIBDIR:STRING=lib",
				"-DBUILD_SHARED_LIBS=OFF",
				"-DBUILD_EXAMPLES=OFF"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libtinyiiod",
					"branch" : "master"
				}
			]
		},
		{
			"name": "KDDockWidgets",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DCMAKE_INSTALL_LIBDIR:STRING=lib",
				"-DKDDockWidgets_QT6=ON",
				"-DKDDockWidgets_FRONTENDS=qtwidgets",
				"-DKDDockWidgets_EXAMPLES=OFF",
				"-DKDDockWidgets_TESTS=OFF"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/KDAB/KDDockWidgets.git",
					"branch": "2.2"
				}
			]
		},
		{
			"name": "extra-cmake-modules",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DBUILD_TESTING=OFF",
				"-DBUILD_HTML_DOCS=OFF",
				"-DBUILD_MAN_DOCS=OFF",
				"-DBUILD_QTHELP_DOCS=OFF"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/KDE/extra-cmake-modules.git",
					"branch": "v6.8.0"
				}
			]
		},
		{
			"name": "karchive",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DBUILD_TESTING=OFF"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/KDE/karchive.git",
					"branch": "v6.8.0"
				}
			]
		},
		{
			"name": "genalyzer",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DCMAKE_INSTALL_LIBDIR:STRING=lib",
				"-DBUILD_TESTING=OFF",
				"-DBUILD_SHARED_LIBS=ON"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/genalyzer.git",
					"branch": "main"
				}
			]
		},
		{
			"name": "iio-emu",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/iio-emu",
					"branch": "master"
				}
			]
		},
	{
		"name": "scopy",
		"builddir": true,
		"buildsystem": "cmake",
		"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app", "-DCMAKE_INSTALL_LIBDIR:STRING=lib", "-DCMAKE_PREFIX_PATH=/app/lib/pkgconfig;/app/lib/cmake", "-DCMAKE_BUILD_TYPE=Release", "-DENABLE_TESTING=OFF", "-DENABLE_ALL_PACKAGES=ON", "-DENABLE_PACKAGE_M2K=OFF", "-DENABLE_PLUGIN_ADC=OFF", "-DWITH_SIGROK=OFF", "-DWITH_PYTHON=OFF"],
		"sources": [
			{
				"type": "git",
				"url": "https://github.com/analogdevicesinc/scopy",
				"branch": "main"
			}
		]
	}
 ]
 }