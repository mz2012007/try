package com.app

object NativeBridge {
    init { System.loadLibrary("native-lib") }

    external fun openContainer(path: String, password: String): Array<String>
    external fun extractFile(srcPath: String, destPath: String): Boolean
    external fun deleteFile(path: String): Boolean
}