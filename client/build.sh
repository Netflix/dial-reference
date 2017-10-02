export TARGET=/usr/local/i686-netflix-linux-gnu-4.3/bin/i686-netflix-linux-gnu-
export LDFLAGS="-L/usr/local/i686-netflix-linux-gnu-4.3/netflix/lib -Wl,-rpath,/usr/local/i686-netflix-linux-gnu-4.3/netflix/lib -Wl,--allow-shlib-undefined"
export INCLUDES=-I/usr/local/i686-netflix-linux-gnu-4.3/netflix/include
make
