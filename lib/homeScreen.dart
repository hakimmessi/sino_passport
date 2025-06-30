import 'dart:io';

import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';
import 'package:sino_passport/ScannerResult.dart';

import 'PassportData.dart';
import 'DeviceInfo.dart';
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
  bool _isInitializing = false;
  DeviceInfo? _deviceInfo;
  PassportData? _scannedData;

  Future<void> _initializeScanner() async {
    setState(() {
      _statusText = "Initializing scanner...";
      _isInitializing = true;
    });

    try {
      final String userId = "426911010110763248";
      final int nType = 0;
      final String sdkDirectory = "/home/kinektek/sino_passport/build/linux/arm64/release/bundle/lib";

      final ScannerResult<DeviceInfo> result = await PassportScannerService.initializeScanner(
        userId: userId,
        nType: nType,
        sdkDirectory: sdkDirectory,
      );

      if (result.success) {
        setState(() {
          _statusText = "Scanner initialized successfully!";
          _isScannerInitialized = true;
          _isInitializing = false;
          _deviceInfo = result.data;
        });
      } else {
        setState(() {
          _statusText = "Failed to initialize scanner: ${result.message}";
          _isScannerInitialized = false;
          _isInitializing = false;
        });
      }
    } catch (e) {
      setState(() {
        _statusText = "Error initializing scanner: $e";
        _isScannerInitialized = false;
        _isInitializing = false;
      });
    }
  }

  Future<void> _scanDocument() async {
    setState(() {
      _statusText = "Place document on scanner...";
      _isScanning = true;
      _scannedData = null;
    });

    try {
      final ScannerResult<PassportData> result = await PassportScannerService.scanDocument(
        timeout: const Duration(seconds: 20),
      );

      if (result.success && result.data != null) {
        setState(() {
          _statusText = "Document scanned successfully!";
          _isScanning = false;
          _scannedData = result.data;
        });
      } else {
        setState(() {
          _statusText = "Scan failed: ${result.message}";
          _isScanning = false;
        });
      }
    } catch (e) {
      setState(() {
        _statusText = "Error scanning document: $e";
        _isScanning = false;
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
  void dispose() {
    // Clean up scanner resources when widget is disposed
    if (_isScannerInitialized) {
      PassportScannerService.releaseScanner();
    }
    super.dispose();
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

              // Show device info when initialized
              if (_deviceInfo != null) ...[
                const SizedBox(height: 15),
                Text(
                  'Device: ${_deviceInfo!.deviceModel}\nSerial: ${_deviceInfo!.serialNumber}',
                  textAlign: TextAlign.center,
                  style: const TextStyle(fontSize: 12, color: Colors.grey),
                ),
              ],

              const SizedBox(height: 30),

              // Initialize Scanner Button
              ElevatedButton(
                onPressed: _isInitializing ? null : _initializeScanner,
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
                child: _isInitializing
                    ? const SizedBox(
                  width: 20,
                  height: 20,
                  child: CircularProgressIndicator(
                    strokeWidth: 2,
                    color: Colors.black87,
                  ),
                )
                    : const Text('Initialize Scanner'),
              ),

              // Scan Button (only show when initialized)
              if (_isScannerInitialized) ...[
                const SizedBox(height: 20),
                ElevatedButton(
                  onPressed: _isScanning ? null : _scanDocument,
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
                  child: _isScanning
                      ? const SizedBox(
                    width: 20,
                    height: 20,
                    child: CircularProgressIndicator(
                      strokeWidth: 2,
                      color: Colors.black87,
                    ),
                  )
                      : const Text('Scan Document'),
                ),
              ],

              // Show scanned data when available
              if (_scannedData != null && !_scannedData!.isEmpty) ...[
                const SizedBox(height: 30),
                Container(
                  width: double.infinity,
                  padding: const EdgeInsets.all(20),
                  decoration: BoxDecoration(
                    border: Border.all(color: Colors.black87, width: 2),
                    borderRadius: BorderRadius.circular(15),
                  ),
                  child: Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Text(
                        'Scanned Document:',
                        style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold),
                      ),
                      const SizedBox(height: 10),
                      if (_scannedData!.passportNumber.isNotEmpty)
                        Text('Passport: ${_scannedData!.passportNumber}'),
                      if (_scannedData!.englishName.isNotEmpty)
                        Text('Name: ${_scannedData!.englishName}'),
                      if (_scannedData!.surname.isNotEmpty && _scannedData!.firstName.isNotEmpty)
                        Text('Full Name: ${_scannedData!.surname}, ${_scannedData!.firstName}'),
                      if (_scannedData!.gender.isNotEmpty)
                        Text('Gender: ${_scannedData!.gender}'),
                      if (_scannedData!.birthDate.isNotEmpty)
                        Text('Birth Date: ${_scannedData!.birthDate}'),
                      if (_scannedData!.expiry.isNotEmpty)
                        Text('Expiry: ${_scannedData!.expiry}'),
                      if (_scannedData!.nationality.isNotEmpty)
                        Text('Nationality: ${_scannedData!.nationality}'),
                      if (_scannedData!.issuingCountry.isNotEmpty)
                        Text('Issuing Country: ${_scannedData!.issuingCountry}'),
                    ],
                  ),
                ),
              ],
            ],
          ),
        ),
      ),
    );
  }
}