import 'dart:io';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';

import 'PassportScannerService.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({Key? key}) : super(key: key);

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  String _statusText = "Press 'Initialize Scanner' to start";
  bool _isScannerInitialized = false;
  bool _isScanning = false;

  Future <void> _initializeScanner() async {
    setState(() {
      _statusText = "Initializing scanner...";
    });

    try {
      final String userId = "426911010110763248";
      final int nType = 0;
      final String sdkDirectory = "/home/kinektek/sino_passport/build/linux/arm64/release/bundle/lib";

      final int result = await PassportScannerService.initializeScanner(
        userId: userId,
        nType: nType,
        sdkDirectory: sdkDirectory,
      );

      if (result == 0) {
        setState(() {
          _statusText = "Scanner initialized successfully!";
          _isScannerInitialized = true;
        });
      } else {
        String errorMsg = _getInitErrorMessage(result);
        setState(() {
          _statusText = "Failed to initialize scanner. Error code: $result";
          _isScannerInitialized = false;
        });
      }
    } catch (e) {
      setState(() {
        _statusText = "Error initializing scanner: $e";
        _isScannerInitialized = false;
      });
    }
  }


  String _getInitErrorMessage(int errorCode) {
    switch (errorCode) {
      case 1:
        return "Authorization ID incorrect - check license";
      case 2:
        return "Device initialization failed - check connection";
      case 3:
        return "Recognition engine init failed - check SDK files";
      case 4:
        return "Authorization files not found";
      case 5:
        return "Failed to load templates";
      case 6:
        return "Chip reader initialization failed";
      default:
        return "Unknown error";
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.white,
      appBar: AppBar(
        title: const Text('SinoScanner App'),
        centerTitle: true,
      ),
      body: Center(
        child: SingleChildScrollView(
          padding: const EdgeInsets.all(20.0),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Container(
                width: 100,
                height: 100,
                padding: const EdgeInsets.all(15),
                child: SvgPicture.asset(
                  'assets/icon_scan.svg',
                  width: 50,
                  height: 50,
                ),
              ),
              const SizedBox(height: 30),
              Text(
                _statusText,
                textAlign: TextAlign.center,
                style: const TextStyle(fontSize: 16, height: 1.5),
              ),
              const SizedBox(height: 30),

              // Scan Button
              if (_isScannerInitialized) ...[
                Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    ElevatedButton(
                      onPressed: _initializeScanner,
                      style: ElevatedButton.styleFrom(
                        backgroundColor: Colors.white,
                        foregroundColor: Colors.black87,
                        padding: const EdgeInsets.symmetric(
                            horizontal: 40, vertical: 20),
                        shape: RoundedRectangleBorder(
                          borderRadius: BorderRadius.circular(30),
                          side: const BorderSide(
                              color: Colors.black87, width: 2),
                        ),
                        elevation: 0,
                      ),
                      child: const Text('Scan Document'),
                    ),
                  ],
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}