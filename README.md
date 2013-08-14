Cinder-libpd
------------

#### libpd static lib build instructions on OS X

Use the following patch to build a static library and copy libpd/libs/libpd.a
to the Cinder-libpd/lib/macosx/ folder.

<pre>
diff --git a/Makefile b/Makefile
index ad07480..1a9b17d 100644
--- a/Makefile
+++ b/Makefile
@@ -69,7 +69,9 @@ PD_FILES = \
 	pure-data/src/x_midi.c pure-data/src/x_misc.c pure-data/src/x_net.c \
 	pure-data/src/x_qlist.c pure-data/src/x_time.c \
 	libpd_wrapper/s_libpdmidi.c libpd_wrapper/x_libpdreceive.c \
-	libpd_wrapper/z_libpd.c 
+	libpd_wrapper/z_libpd.c \
+	libpd_wrapper/util/ringbuffer.c libpd_wrapper/util/z_hook_util.c \
+	libpd_wrapper/util/z_print_util.c libpd_wrapper/util/z_queued.c
 
 PDJAVA_JAR_CLASSES = \
 	java/org/puredata/core/PdBase.java \
@@ -103,7 +105,7 @@ CFLAGS = -DPD -DHAVE_UNISTD_H -DUSEAPI_DUMMY -I./pure-data/src \
 libpd: $(LIBPD)
 
 $(LIBPD): ${PD_FILES:.c=.o}
-	$(CC) -o $(LIBPD) $^ $(LDFLAGS) -lm -lpthread 
+	libtool -static -o libs/libpd.a $^
 
 javalib: $(JNIH_FILE) $(PDJAVA_JAR)
 
</pre>

#### libpd static lib build on Windows

Use the following patched [libpd](https://github.com/bgbotond/libpd) with
Visual Studio to recompile libpd.lib.

