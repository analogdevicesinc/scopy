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
		{
			"name": "boost",
			"cleanup": [ "/bin", "/share" ],
			"sources": [
				{
					"type": "archive",
					"url": "https://boostorg.jfrog.io/artifactory/main/release/1.73.0/source/boost_1_73_0.tar.gz",
					"sha256": "9995e192e68528793755692917f9eb6422f3052a53c5e13ba278a228af6c7acf"
				},
				{
					"type": "script",
					"commands": [
						"#!/bin/sh",
						"exec ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,regex,system,test,thread,atomic,chrono --prefix=/app"
					],
					"dest-filename": "configure"
				},
				{
					"type": "script",
					"commands": [
						"all:",
						"\t./b2",
						"install:",
						"\t./b2 install"
					],
					"dest-filename": "Makefile"
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
                    "url": "https://dbus.freedesktop.org/releases/dbus-python/dbus-python-1.2.16.tar.gz",
                    "sha256": "11238f1d86c995d8aed2e22f04a1e3779f0d70e587caffeab4857f3c662ed5a4"
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
			"name": "matio",
			"cleanup": [ "/bin", "/share" ],
			"config-opts": [ "" ],
			"build-commands": [ "true"],
			"sources": [
				{
					"type": "archive",
					"url": "https://github.com/tbeu/matio/releases/download/v1.5.12/matio-1.5.12.tar.gz",
					"sha1": "cb05a4a9a3af992890237b3f7a87f01cd8819528"
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
			"name": "libgmp",
			"config-opts": [ "--prefix=/app", "--enable-cxx",
		    EXPAND(CFLAGS)
#ifdef __ARM__
            ,EXPAND(ASFLAGS)
#endif
            ],
			"sources": [
				{
					"type": "archive",
					"url": "https://ftp.gnu.org/gnu/gmp/gmp-6.2.0.tar.bz2",
					"sha256": "f51c99cb114deb21a60075ffb494c1a210eb9d7cb729ed042ddb7de9534451ea"
				}
			]
		},
		{
			"name": "liborc",
			"config-opts": [ "--prefix=/app"
#ifdef __ARM__
            ,EXPAND(CFLAGS) ,EXPAND(ASFLAGS) 
#endif
            ],
			"sources": [
				{
					"type": "archive",
					"url": "https://gstreamer.freedesktop.org/data/src/orc/orc-0.4.28.tar.xz",
					"sha256": "bfcd7c6563b05672386c4eedfc4c0d4a0a12b4b4775b74ec6deb88fc2bcd83ce"
				}
			]
		},
		{
			"name": "libvolk",
			"cleanup": [ "/bin", "/share" ],
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
            "-DCMAKE_INSTALL_PREFIX:PATH=/app",
            EXPAND(CMAKE_C_FLAGS), EXPAND(CMAKE_ASM_FLAGS)
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/gnuradio/volk",
					"branch": "v2.5.1"
				}
			]
		},
		{
			"name": "spdlog",
			"cleanup": [ "/bin", "/share" ],
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
            "-DCMAKE_INSTALL_PREFIX:PATH=/app", "-DSPDLOG_BUILD_SHARED=ON",
            EXPAND(CMAKE_C_FLAGS), EXPAND(CMAKE_ASM_FLAGS)
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/gabime/spdlog",
					"branch": "v1.10.0"
				}
			]
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
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libad9361-iio",
					"branch" : "main"
				}
			]
		},
		{
			"name": "gnuradio",
			"cleanup": [ "/bin", "/share" ],
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DENABLE_DEFAULT=OFF",
				"-DENABLE_GNURADIO_RUNTIME=ON",
				"-DENABLE_GR_ANALOG=ON",
				"-DENABLE_GR_BLOCKS=ON",
				"-DENABLE_GR_FFT=ON",
				"-DENABLE_GR_FILTER=ON",
				"-DENABLE_GR_IIO=ON",
                EXPAND(CMAKE_C_FLAGS)
#ifdef __ARM__
                , EXPAND(CMAKE_ASM_FLAGS)
#endif
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/gnuradio",
					"branch" : "scopy2-maint-3.10"
				}
			]
		},
		{
			"name": "glog",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DWITH_GFLAGS=OFF"
			],
			"sources": [
				{
					"type": "git",
					"commit": "17e7679fd9beb95277ccd0708056ba85363f892b" ,
					"url": "https://github.com/google/glog"
				}
			]
		},
		{
			"name": "libm2k",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DENABLE_PYTHON=OFF",
				"-DENABLE_CSHARP=OFF",
				"-DENABLE_TOOLS=OFF",
				"-DBUILD_EXAMPLES=OFF",
				"-DINSTALL_UDEV_RULES=OFF"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libm2k",
					"branch": "main"
				}
			]
		},
		{
			"name": "gr-m2k",
			"cleanup": [ "/share" ],
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/gr-m2k",
					"branch" : "main"
				}
			]
		},
		{
			"name": "gr-scopy",
			"cleanup": [ "/share" ],
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/gr-scopy",
					"branch" : "3.10"
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
			"name": "sigrokdecode",
			"builddir": false,
			"buildsystem": "autotools",
			"config-opts": [ "--prefix=/app" ],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/sigrokproject/libsigrokdecode",
					"commit": "e556e1168af7027df08622ecfe11309811249e81"
				}
			]
		},
		{
			"name": "libtinyiiod",
			"builddir": true,
			"buildsystem": "cmake",
			"config-opts": [
				"-DCMAKE_INSTALL_PREFIX:PATH=/app",
				"-DBUILD_SHARED_LIBS=OFF",
				"-DBUILD_EXAMPLES=OFF"
			],
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/analogdevicesinc/libtinyiiod",
					"commit": "15e79e7c0064b0476ab4608d02d5efb988b93fc9"
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
			"name": "qtadvanceddocking",
			"builddir": true,
			"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app" ],
			"buildsystem": "cmake",
			"sources": [
				{
					"type": "git",
					"url": "https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System",
					"tag": "3.8.1"
				}
			]
		},
	{
		"name": "scopy",
		"builddir": true,
		"buildsystem": "cmake",
		"config-opts": [ "-DCMAKE_INSTALL_PREFIX:PATH=/app", "-DCMAKE_PREFIX_PATH=/app/lib/pkgconfig;/app/lib/cmake", "-DCMAKE_BUILD_TYPE=Release", "-DENABLE_TESTING=OFF"],
		"sources": [
			{
				"type": "git",
				"url": "https://github.com/analogdevicesinc/scopy",
				"branch": "dev"
			}
		]
	}
 ]
 }