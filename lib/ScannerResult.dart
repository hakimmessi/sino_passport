class ScannerResult<T> {
  final bool success;
  final T? data;
  final String message;
  final int? errorCode;

  ScannerResult({
    required this.success,
    this.data,
    this.message = '',
    this.errorCode,
  });

  factory ScannerResult.success(T data, {String message = ''}) {
    return ScannerResult(
      success: true,
      data: data,
      message: message,
    );
  }

  factory ScannerResult.error(String message, {int? errorCode}) {
    return ScannerResult(
      success: false,
      message: message,
      errorCode: errorCode,
    );
  }
}

// Detection result constants
class DetectionResult {
  static const int notDetected = 0;
  static const int documentPlaced = 1;
  static const int documentRemoved = 2;
  static const int phoneBarcodeDetected = 3;
}

// Device status constants
class DeviceStatus {
  static const int connected = 1;
  static const int disconnected = 2;
  static const int needsReinit = 3;
}

// Error codes
class ScannerError {
  static const int success = 0;
  static const int authIncorrect = 1;
  static const int deviceInit = 2;
  static const int engineInit = 3;
  static const int authFilesNotFound = 4;
  static const int templateLoad = 5;
  static const int chipReaderInit = 6;
  static const int config = -5;
  static const int general = -1;
  static const int timeout = -100;
  static const int unknown = -200;
}