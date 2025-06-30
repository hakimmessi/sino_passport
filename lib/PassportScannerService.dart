import 'package:flutter/services.dart';
class PassportScannerService {
  static const MethodChannel _channel = MethodChannel('sinosecu');

  static Future <int> initializeScanner({
    required String userId,
    required int nType,
    required String sdkDirectory,
}) async{
    try{
      print('[Flutter] Calling initializeScanner with:');
      print('[Flutter]   UserID: $userId');
      print('[Flutter]   nType: $nType');
      print('[Flutter]   SDK Directory: $sdkDirectory');

      final int result = await _channel.invokeMethod('initializeScanner', {
        'userId': userId,
        'nType': nType,
        'sdkDirectory': sdkDirectory,
      });
      print('[Flutter] initializeScanner result: $result');
      return result;
    } on PlatformException catch (e) {
      print('[Flutter] Failed to initialize scanner: ${e.message}');
      print('[Flutter] PlatformException Details: ${e.details}');
      return -100;
    } catch (e) {
      print('[Flutter] Unknown error during initializeScanner: $e');
      return -200;
    }
    }
  }
