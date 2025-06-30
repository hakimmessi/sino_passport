class DeviceInfo {
  final String serialNumber;
  final String deviceModel;
  final bool isReady;

  DeviceInfo({
    this.serialNumber = '',
    this.deviceModel = '',
    this.isReady = false,
  });

  factory DeviceInfo.fromMap(Map<String, dynamic> map) {
    return DeviceInfo(
      serialNumber: map['serialNumber'] ?? '',
      deviceModel: map['deviceModel'] ?? '',
      isReady: map['isReady'] ?? false,
    );
  }
}