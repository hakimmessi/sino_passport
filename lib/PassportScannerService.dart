import 'package:flutter/services.dart';
import 'package:sino_passport/ScannerResult.dart';

import 'PassportData.dart';
import 'DeviceInfo.dart';
class PassportScannerService {
  static const MethodChannel _channel = MethodChannel('sinosecu');

  // Initialize the scanner
  static Future<ScannerResult<DeviceInfo>> initializeScanner({
    required String userId,
    required int nType,
    required String sdkDirectory,
  }) async {
    try {
      print('[Flutter] Calling initializeScanner with:');
      print('[Flutter]   UserID: $userId');
      print('[Flutter]   nType: $nType');
      print('[Flutter]   SDK Directory: $sdkDirectory');

      final Map<String, dynamic> result = await _channel.invokeMethod(
          'initializeScanner', {
        'userId': userId,
        'nType': nType,
        'sdkDirectory': sdkDirectory,
      });

      print('[Flutter] initializeScanner result: $result');

      final bool success = result['success'] ?? false;
      final String message = result['message'] ?? '';
      final int resultCode = result['result'] ?? -1;

      if (success) {
        final deviceInfo = DeviceInfo.fromMap(result);
        return ScannerResult.success(deviceInfo, message: message);
      } else {
        return ScannerResult.error(message, errorCode: resultCode);
      }
    } on PlatformException catch (e) {
      print('[Flutter] Failed to initialize scanner: ${e.message}');
      print('[Flutter] PlatformException Details: ${e.details}');
      return ScannerResult.error(
          'Platform error: ${e.message}', errorCode: ScannerError.general);
    } catch (e) {
      print('[Flutter] Unknown error during initializeScanner: $e');
      return ScannerResult.error(
          'Unknown error: $e', errorCode: ScannerError.unknown);
    }
  }

  // Check device status
  static Future<ScannerResult<int>> checkDeviceStatus() async {
    try {
      print('[Flutter] Checking device status...');

      final Map<String, dynamic> result = await _channel.invokeMethod(
          'checkDeviceStatus');

      final int status = result['status'] ?? DeviceStatus.disconnected;
      final String message = result['message'] ?? '';

      print('[Flutter] Device status: $status - $message');

      return ScannerResult.success(status, message: message);
    } on PlatformException catch (e) {
      print('[Flutter] Failed to check device status: ${e.message}');
      return ScannerResult.error('Failed to check device status: ${e.message}');
    } catch (e) {
      print('[Flutter] Unknown error during checkDeviceStatus: $e');
      return ScannerResult.error('Unknown error: $e');
    }
  }

  // Detect document presence
  static Future<ScannerResult<int>> detectDocument() async {
    try {
      final Map<String, dynamic> result = await _channel.invokeMethod(
          'detectDocument');

      final int detectionResult = result['detectionResult'] ??
          DetectionResult.notDetected;
      final String message = result['message'] ?? '';
      final bool documentPresent = result['documentPresent'] ?? false;

      print(
          '[Flutter] Document detection: $detectionResult - $message (Present: $documentPresent)');

      return ScannerResult.success(detectionResult, message: message);
    } on PlatformException catch (e) {
      print('[Flutter] Failed to detect document: ${e.message}');
      return ScannerResult.error('Failed to detect document: ${e.message}');
    } catch (e) {
      print('[Flutter] Unknown error during detectDocument: $e');
      return ScannerResult.error('Unknown error: $e');
    }
  }

  // Process document and extract data
  static Future<ScannerResult<PassportData>> processDocument() async {
    try {
      print('[Flutter] Processing document...');

      final Map<String, dynamic> result = await _channel.invokeMethod(
          'processDocument');

      final bool success = result['success'] ?? false;
      final String message = result['message'] ?? '';
      final int processResult = result['processResult'] ?? -1;

      if (success && result.containsKey('passportData')) {
        final Map<String, dynamic> passportDataMap = Map<String, dynamic>.from(
            result['passportData']);
        final passportData = PassportData.fromMap(passportDataMap);

        print('[Flutter] Document processed successfully: $passportData');
        return ScannerResult.success(passportData, message: message);
      } else {
        print('[Flutter] Document processing failed: $message');
        return ScannerResult.error(message, errorCode: processResult);
      }
    } on PlatformException catch (e) {
      print('[Flutter] Failed to process document: ${e.message}');
      return ScannerResult.error('Failed to process document: ${e.message}');
    } catch (e) {
      print('[Flutter] Unknown error during processDocument: $e');
      return ScannerResult.error('Unknown error: $e');
    }
  }

  // Get device information
  static Future<ScannerResult<DeviceInfo>> getDeviceInfo() async {
    try {
      final Map<String, dynamic> result = await _channel.invokeMethod(
          'getDeviceInfo');

      final deviceInfo = DeviceInfo.fromMap(result);
      print('[Flutter] Device info: ${deviceInfo.deviceModel} (${deviceInfo
          .serialNumber})');

      return ScannerResult.success(deviceInfo);
    } on PlatformException catch (e) {
      print('[Flutter] Failed to get device info: ${e.message}');
      return ScannerResult.error('Failed to get device info: ${e.message}');
    } catch (e) {
      print('[Flutter] Unknown error during getDeviceInfo: $e');
      return ScannerResult.error('Unknown error: $e');
    }
  }

  // Play buzzer
  static Future<ScannerResult<bool>> playBuzzer({int duration = 100}) async {
    try {
      final Map<String, dynamic> result = await _channel.invokeMethod(
          'playBuzzer', {
        'duration': duration,
      });

      final bool success = result['success'] ?? false;
      print('[Flutter] Buzzer played: $success');

      return ScannerResult.success(success);
    } on PlatformException catch (e) {
      print('[Flutter] Failed to play buzzer: ${e.message}');
      return ScannerResult.error('Failed to play buzzer: ${e.message}');
    } catch (e) {
      print('[Flutter] Unknown error during playBuzzer: $e');
      return ScannerResult.error('Unknown error: $e');
    }
  }

  // Release scanner resources
  static Future<ScannerResult<bool>> releaseScanner() async {
    try {
      print('[Flutter] Releasing scanner...');

      final Map<String, dynamic> result = await _channel.invokeMethod(
          'releaseScanner');

      final bool success = result['success'] ?? false;
      final String message = result['message'] ?? '';

      print('[Flutter] Scanner release: $success - $message');

      return ScannerResult.success(success, message: message);
    } on PlatformException catch (e) {
      print('[Flutter] Failed to release scanner: ${e.message}');
      return ScannerResult.error('Failed to release scanner: ${e.message}');
    } catch (e) {
      print('[Flutter] Unknown error during releaseScanner: $e');
      return ScannerResult.error('Unknown error: $e');
    }
  }

  // Convenience method for continuous document detection
  static Stream<int> detectDocumentStream(
      {Duration interval = const Duration(seconds: 1)}) async* {
    while (true) {
      await Future.delayed(interval);

      final result = await detectDocument();
      if (result.success && result.data != null) {
        yield result.data!;
      } else {
        yield DetectionResult.notDetected;
      }
    }
  }

  // High-level method to scan a document (detect + process)
  static Future<ScannerResult<PassportData>> scanDocument({
    Duration timeout = const Duration(seconds: 30),
    Duration checkInterval = const Duration(milliseconds: 500),
  }) async {
    print('[Flutter] Starting document scan...');

    final stopwatch = Stopwatch()
      ..start();

    while (stopwatch.elapsed < timeout) {
      // Check for document
      final detectionResult = await detectDocument();
      if (!detectionResult.success) {
        return ScannerResult.error(
            'Detection failed: ${detectionResult.message}');
      }

      if (detectionResult.data == DetectionResult.documentPlaced) {
        print('[Flutter] Document detected, processing...');

        // Small delay to ensure document is stable
        await Future.delayed(const Duration(milliseconds: 500));

        // Process the document
        final processResult = await processDocument();

        if (processResult.success) {
          // Play success sound
          await playBuzzer(duration: 200);
          return processResult;
        } else {
          // Play error sound
          await playBuzzer(duration: 50);
          return ScannerResult.error(
              'Processing failed: ${processResult.message}');
        }
      }

      await Future.delayed(checkInterval);
    }

    return ScannerResult.error(
        'Scan timeout: No document detected within ${timeout
            .inSeconds} seconds');
  }

  // Helper method to get error message from error code
  static String getErrorMessage(int errorCode) {
    switch (errorCode) {
      case ScannerError.success:
        return 'Success';
      case ScannerError.authIncorrect:
        return 'Authorization ID is incorrect';
      case ScannerError.deviceInit:
        return 'Device initialization failed';
      case ScannerError.engineInit:
        return 'Recognition engine initialization failed';
      case ScannerError.authFilesNotFound:
        return 'Authorization files not found';
      case ScannerError.templateLoad:
        return 'Failed to load recognition templates';
      case ScannerError.chipReaderInit:
        return 'Chip reader initialization failed';
      case ScannerError.config:
        return 'Configuration error';
      case ScannerError.general:
        return 'General error';
      case ScannerError.timeout:
        return 'Operation timeout';
      case ScannerError.unknown:
        return 'Unknown error';
      default:
        return 'Unknown error code: $errorCode';
    }
  }
}

