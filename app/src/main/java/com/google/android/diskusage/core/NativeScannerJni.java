package com.google.android.diskusage.core;

import android.os.ParcelFileDescriptor;
import java.io.InputStream;

public class NativeScannerJni {
    static {
        System.loadLibrary("scan");
    }

    private static native int scanTree(String path);

    public static InputStream createStream(String path) {
        int fd = scanTree(path);
        if (fd < 0) throw new RuntimeException("Failed to create scan pipe");
        return new ParcelFileDescriptor.AutoCloseInputStream(
                ParcelFileDescriptor.adoptFd(fd));
    }
}
