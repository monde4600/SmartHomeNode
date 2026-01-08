#pragma once
#include <Firebase_ESP_Client.h>

// Firebase objects
extern FirebaseData fbdo;
extern FirebaseData stream;
extern FirebaseAuth auth;
extern FirebaseConfig config;

void firebase_begin();
