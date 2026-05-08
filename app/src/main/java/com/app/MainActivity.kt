package com.app

import android.os.Bundle
import android.os.Environment
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            MaterialTheme {
                Surface(modifier = Modifier.fillMaxSize()) {
                    App()
                }
            }
        }
    }
}

@Composable
fun App() {
    var containerPath by remember { mutableStateOf("/storage/emulated/0/Download/test.img") }
    var password by remember { mutableStateOf("") }
    var files by remember { mutableStateOf<List<String>>(emptyList()) }
    var mounted by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    Column(modifier = Modifier.padding(16.dp)) {
        OutlinedTextField(value = containerPath, onValueChange = { containerPath = it }, label = { Text("LUKS container path") })
        Spacer(modifier = Modifier.height(8.dp))
        OutlinedTextField(value = password, onValueChange = { password = it }, label = { Text("Password") })
        Spacer(modifier = Modifier.height(8.dp))
        Button(onClick = {
            scope.launch {
                withContext(Dispatchers.IO) {
                    try {
                        val result = NativeBridge.openContainer(containerPath, password)
                        files = result.toList()
                        mounted = true
                    } catch (e: Exception) {
                        withContext(Dispatchers.Main) {
                            Toast.makeText(this@MainActivity, "Failed: ${e.message}", Toast.LENGTH_SHORT).show()
                        }
                    }
                }
            }
        }) { Text("Open Container") }

        if (mounted) {
            Text("Files (tap to extract)", style = MaterialTheme.typography.titleMedium)
            LazyColumn {
                items(files) { file ->
                    Row(
                        modifier = Modifier
                            .fillMaxWidth()
                            .clickable {
                                scope.launch {
                                    val dest = File(
                                        Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS),
                                        file
                                    )
                                    withContext(Dispatchers.IO) {
                                        val ok = NativeBridge.extractFile("/$file", dest.absolutePath)
                                        withContext(Dispatchers.Main) {
                                            Toast.makeText(
                                                this@MainActivity,
                                                if (ok) "Extracted" else "Extract failed",
                                                Toast.LENGTH_SHORT
                                            ).show()
                                        }
                                    }
                                }
                            }
                            .padding(8.dp)
                    ) {
                        Text(file, modifier = Modifier.weight(1f))
                        Button(onClick = {
                            scope.launch {
                                withContext(Dispatchers.IO) {
                                    NativeBridge.deleteFile("/$file")
                                    files = NativeBridge.openContainer(containerPath, password).toList()
                                }
                            }
                        }) { Text("Del") }
                    }
                }
            }
        }
    }
}